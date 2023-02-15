/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#include "sid_error.h"
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/storage/flash_map.h>

#include <sid_thread.h>
#include <sid_api_delegated.h>

#include <sid_api.h>
#include <sid_ble_link_config_ifc.h>
#include <sid_pal_mfg_store_ifc.h>
#include <sid_pal_storage_kv_ifc.h>
#include <sid_pal_crypto_ifc.h>
#if defined(CONFIG_SIDEWALK_LINK_MASK_FSK) || defined(CONFIG_SIDEWALK_LINK_MASK_LORA)
#include <sx126x_config.h>
#include <sid_pal_gpio_ifc.h>
#include <sid_900_cfg.h>
#include <sid_pal_serial_bus_ifc.h>
#include <sid_pal_serial_bus_spi_config.h>
#endif

#if !FLASH_AREA_LABEL_EXISTS(mfg_storage)
	#error "Flash partition is not defined for the Sidewalk manufacturing storage!!"
#endif

LOG_MODULE_REGISTER(sid_thread, LOG_LEVEL_DBG);

K_THREAD_STACK_DEFINE(sidewalk_dut_work_q_stack, SIDEWALK_DUT_WORK_Q_STACK_SIZE);

static void sidewalk_event_worker(struct k_work *work);
static void on_sidewalk_event(bool in_isr, void *context);
static void on_sidewalk_msg_received(const struct sid_msg_desc *msg_desc, const struct sid_msg *msg, void *context);
static void on_sidewalk_msg_sent(const struct sid_msg_desc *msg_desc, void *context);
static void on_sidewalk_send_error(sid_error_t error, const struct sid_msg_desc *msg_desc, void *context);
static void on_sidewalk_factory_reset(void *context);
static void on_sidewalk_status_changed(const struct sid_status *status, void *context);

// ////////////////////////////////////////////////////////////////////////////

static struct k_work_q sidewalk_dut_work_q;

static struct sid_handle *sid_handle = NULL;

static struct app_context g_app_context = {
	.sidewalk_handle = &sid_handle,
};

static struct sid_event_callbacks event_callbacks = {
	.context = &g_app_context,
	.on_event = on_sidewalk_event,                          /* Called from ISR context */
	.on_msg_received = on_sidewalk_msg_received,            /* Called from sid_process() */
	.on_msg_sent = on_sidewalk_msg_sent,                    /* Called from sid_process() */
	.on_send_error = on_sidewalk_send_error,                /* Called from sid_process() */
	.on_status_changed = on_sidewalk_status_changed,        /* Called from sid_process() */
	.on_factory_reset = on_sidewalk_factory_reset,          /* Called from sid_process */
};

#if defined(CONFIG_SIDEWALK_LINK_MASK_FSK) || defined(CONFIG_SIDEWALK_LINK_MASK_LORA)
static const struct sid_sub_ghz_links_config sub_ghz_link_config = {
	.enable_link_metrics = true,
	.registration_config = {
		.enable = true,
		.periodicity_s = UINT32_MAX,
	},
};
#endif

static const sid_ble_link_config_t ble_link_config = {
	.create_ble_adapter = sid_pal_ble_adapter_create,
	.config = NULL,
};

static struct sid_config config = {
	.callbacks = &event_callbacks,
	.link_config = &ble_link_config,
	.time_sync_periodicity_seconds = 7200,
#if defined(CONFIG_SIDEWALK_LINK_MASK_FSK) || defined(CONFIG_SIDEWALK_LINK_MASK_LORA)
	.sub_ghz_link_config = &sub_ghz_link_config,
#else
	.sub_ghz_link_config = NULL,
#endif
};

// ////////////////////////////////////////////////////////////////////////////

static void sidewalk_event_worker(struct k_work *work)
{
	LOG_DBG("sid_process handled");
	struct app_context *app_ctx = CONTAINER_OF(work, struct app_context, sidewalk_event_work);

	sid_error_t e = sid_process(*app_ctx->sidewalk_handle);

	if (e != SID_ERROR_NONE) {
		LOG_ERR("sid process failed with error %d", e);
	}
}

static void on_sidewalk_event(bool in_isr, void *context)
{
	LOG_DBG("Sidewalk -> App");
	struct app_context *ctx = (struct app_context *)context;

	if (ctx->sidewalk_event_work.handler == NULL) {
		k_work_init(&ctx->sidewalk_event_work, sidewalk_event_worker);
	}
	k_work_submit_to_queue(&sidewalk_dut_work_q, &ctx->sidewalk_event_work);
}

static void on_sidewalk_msg_received(const struct sid_msg_desc *msg_desc, const struct sid_msg *msg, void *context)
{
	LOG_DBG("Sidewalk -> App");
	LOG_HEXDUMP_INF(msg->data, msg->size, "");
}

static void on_sidewalk_msg_sent(const struct sid_msg_desc *msg_desc, void *context)
{
	LOG_DBG("Sidewalk -> App");
}

static void on_sidewalk_send_error(sid_error_t error, const struct sid_msg_desc *msg_desc, void *context)
{
	LOG_DBG("Sidewalk -> App: error %d", error);
}

static void on_sidewalk_factory_reset(void *context)
{
	LOG_DBG("Sidewalk -> App");
}

static void on_sidewalk_status_changed(const struct sid_status *status, void *context)
{
	struct app_context *app_context = (struct app_context *)context;

	switch (status->state) {
	case SID_STATE_READY:
		LOG_INF("Sidewalk status changed to SID_STATE_READY");
		break;
	case SID_STATE_NOT_READY:
		LOG_INF("Sidewalk status changed to SID_STATE_NOT_READY");
		break;
	case SID_STATE_ERROR:
		if (*app_context->sidewalk_handle) {
			LOG_ERR("sidewalk error: %d", (int)sid_get_error(*app_context->sidewalk_handle));
		} else {
			LOG_ERR("sidewalk error: handle NULL");
		}
		break;
	case SID_STATE_SECURE_CHANNEL_READY:
		LOG_INF("Sidewalk status changed to SID_STATE_SECURE_CHANNEL_READY");
		break;
	}

	LOG_INF("EVENT SID STATUS: State: %d, Reg: %d, Time: %d, Link_Mask: %x",
		status->state,
		status->detail.registration_status,
		status->detail.time_sync_status,
		status->detail.link_status_mask);
	LOG_INF("EVENT SID STATUS LINK MODE: LORA: %x, FSK: %x, BLE: %x",
		status->detail.supported_link_modes[2],
		status->detail.supported_link_modes[1],
		status->detail.supported_link_modes[0]);
}

// ////////////////////////////////////////////////////////////////////////////
#if defined(CONFIG_SIDEWALK_LINK_MASK_FSK) || defined(CONFIG_SIDEWALK_LINK_MASK_LORA)
#define REGION_US915

/* This product has no external PA and SX1262 can support max of 22dBm*/
#define RADIO_SX1262_MAX_TX_POWER                                  22
#define RADIO_SX1262_MIN_TX_POWER                                  -9

#define RADIO_MAX_TX_POWER_NA                                      20
#define RADIO_MAX_TX_POWER_EU                                      14

#if defined (REGION_ALL)
#define RADIO_REGION                                               RADIO_REGION_NONE
#elif defined (REGION_US915)
#define RADIO_REGION                                               RADIO_REGION_NA
#elif defined (REGION_EU868)
#define RADIO_REGION                                               RADIO_REGION_EU
#endif

#define RADIO_SX1262_SPI_BUFFER_SIZE                               255

#define RADIO_SX1262_PA_DUTY_CYCLE                                 0x04
#define RADIO_SX1262_HP_MAX                                        0x07
#define RADIO_SX1262_DEVICE_SEL                                    0x00
#define RADIO_SX1262_PA_LUT                                        0x01

#define RADIO_RX_LNA_GAIN                                          0
#define RADIO_MAX_CAD_SYMBOL                                       SID_PAL_RADIO_LORA_CAD_04_SYMBOL
#define RADIO_ANT_GAIN(X)                                          ((X) * 100)

static const struct sid_pal_serial_bus_factory radio_spi_factory = {
	.create = sid_pal_serial_bus_nordic_spi_create,
};

static uint8_t radio_sx1262_buffer[RADIO_SX1262_SPI_BUFFER_SIZE] = { 0 };

static int32_t radio_sx1262_pa_cfg(int8_t tx_power, radio_sx126x_pa_cfg_t *pa_cfg)
{
	int8_t pwr = tx_power;

	if (tx_power > RADIO_SX1262_MAX_TX_POWER) {
		pwr = RADIO_SX1262_MAX_TX_POWER;
	}

	if (tx_power < RADIO_SX1262_MIN_TX_POWER) {
		pwr = RADIO_SX1262_MIN_TX_POWER;
	}

	pa_cfg->pa_duty_cycle = RADIO_SX1262_PA_DUTY_CYCLE;
	pa_cfg->hp_max = RADIO_SX1262_HP_MAX;
	pa_cfg->device_sel = RADIO_SX1262_DEVICE_SEL;
	pa_cfg->pa_lut = RADIO_SX1262_PA_LUT;
	pa_cfg->tx_power = pwr;  // one to one mapping between tx params and tx power
	pa_cfg->ramp_time = RADIO_SX126X_RAMP_40_US;

	return 0;
}

const radio_sx126x_regional_param_t radio_sx126x_regional_param[] =
{
    #if defined (REGION_ALL) || defined (REGION_US915)
	{
		.param_region = RADIO_REGION_NA,
		.max_tx_power = { RADIO_MAX_TX_POWER_NA, RADIO_MAX_TX_POWER_NA, RADIO_MAX_TX_POWER_NA,
				  RADIO_MAX_TX_POWER_NA, RADIO_MAX_TX_POWER_NA, RADIO_MAX_TX_POWER_NA },
		.cca_level_adjust = { 0, 0, 0, 0, 0, 0 },
		.ant_dbi = RADIO_ANT_GAIN(2.15)
	},
    #endif
    #if defined (REGION_ALL) || defined (REGION_EU868)
	{
		.param_region = RADIO_REGION_EU,
		.max_tx_power = { RADIO_MAX_TX_POWER_EU, RADIO_MAX_TX_POWER_EU, RADIO_MAX_TX_POWER_EU,
				  RADIO_MAX_TX_POWER_EU, RADIO_MAX_TX_POWER_EU, RADIO_MAX_TX_POWER_EU },
		.cca_level_adjust = { 0, 0, 0, 0, 0, 0 },
		.ant_dbi = RADIO_ANT_GAIN(2.15)
	},
    #endif
};

const radio_sx126x_device_config_t radio_sx1262_cfg = {
	.id = SEMTECH_ID_SX1262,                     // chip id register not supported
	.regulator_mode = RADIO_SX126X_REGULATOR_DCDC,
	.rx_boost = false,
	.lna_gain = RADIO_RX_LNA_GAIN,
	.bus_factory = &radio_spi_factory,
	.gpio_power = 3,                // sx1262_NRESET
	.gpio_int1 = 38,                // sx1262_DIO1
	.gpio_radio_busy = 36,          // sx1262_BUSY
	.gpio_rf_sw_ena = 42,           // sx1262 ANT_SW
	.gpio_tx_bypass = 128,
	.bus_selector = {
		.client_selector = 40, // sx1262_NSS
		.bit_order = SID_PAL_SERIAL_BUS_BIT_ORDER_MSB_FIRST,
	},
	.pa_cfg_callback = radio_sx1262_pa_cfg,

	.tcxo = {
		.ctrl = SX126X_TCXO_CTRL_NONE,
	},

	.regional_config = {
		.radio_region = RADIO_REGION,
		.reg_param_table_size = sizeof(radio_sx126x_regional_param) / sizeof(radio_sx126x_regional_param[0]),
		.reg_param_table = radio_sx126x_regional_param,
	},

	.state_timings = {
		.sleep_to_full_power_us = 440,
		.full_power_to_sleep_us = 0,
		.rx_to_tx_us = 0,
		.tx_to_rx_us = 0,
	},

	.internal_buffer = {
		.p = radio_sx1262_buffer,
		.size = sizeof(radio_sx1262_buffer),
	},
};
#endif /* defined(CONFIG_SIDEWALK_LINK_MASK_FSK) || defined(CONFIG_SIDEWALK_LINK_MASK_LORA) */

static sid_error_t sid_pal_init(void)
{
	sid_error_t ret_code;

	ret_code = sid_pal_storage_kv_init();
	if (SID_ERROR_NONE != ret_code) {
		LOG_ERR("Sidewalk KV store init failed, err: %d", ret_code);
		return ret_code;
	}

	ret_code = sid_pal_crypto_init();
	if (SID_ERROR_NONE != ret_code) {
		LOG_ERR("Sidewalk Init Crypto HAL, err: %d", ret_code);
		return ret_code;
	}

	static const sid_pal_mfg_store_region_t mfg_store_region = {
		.addr_start = (uintptr_t)(FLASH_AREA_OFFSET(mfg_storage)),
		.addr_end = (uintptr_t)(FLASH_AREA_OFFSET(mfg_storage) + FLASH_AREA_SIZE(mfg_storage)),
	};

	sid_pal_mfg_store_init(mfg_store_region);

#if defined(CONFIG_SIDEWALK_LINK_MASK_FSK) || defined(CONFIG_SIDEWALK_LINK_MASK_LORA)
	set_radio_sx126x_device_config(&radio_sx1262_cfg);
#endif /* defined(CONFIG_SIDEWALK_LINK_MASK_FSK) || defined(CONFIG_SIDEWALK_LINK_MASK_LORA) */

	return SID_ERROR_NONE;
}

sid_error_t sid_thread_init(void)
{
	k_work_queue_init(&sidewalk_dut_work_q);
	static struct k_work_queue_config cfg = {.name = "sidewalk_thread", .no_yield = false};
	k_work_queue_start(&sidewalk_dut_work_q, sidewalk_dut_work_q_stack,
			   K_THREAD_STACK_SIZEOF(sidewalk_dut_work_q_stack), CONFIG_SIDEWALK_THREAD_PRIORITY,
			   &cfg);
	sid_api_delegated(&sidewalk_dut_work_q);
	return sid_pal_init();
}

struct sid_config *sid_thread_get_config()
{
	return &config;
}

struct app_context *sid_thread_get_app_context()
{
	return &g_app_context;
}
