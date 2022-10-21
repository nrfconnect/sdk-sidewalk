/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#include <sidewalk_thread.h>

#include <sid_api.h>
#include <sid_error.h>
#include <sid_pal_crypto_ifc.h>
#include <sid_pal_ble_adapter_ifc.h>
#include <sid_ble_link_config_ifc.h>
#include <sid_pal_storage_kv_ifc.h>
#include <sid_pal_mfg_store_ifc.h>
#include <sid_hal_reset_ifc.h>
#include <sid_pal_assert_ifc.h>
#if defined(CONFIG_SIDEWALK_LINK_MASK_FSK) || defined(CONFIG_SIDEWALK_LINK_MASK_LORA)
#include <sid_900_cfg.h>
#include <sx126x_config.h>
#include <spi_bus.h>
#endif /* defined(CONFIG_SIDEWALK_LINK_MASK_FSK) || defined(CONFIG_SIDEWALK_LINK_MASK_LORA) */

#include <zephyr/kernel.h>
#include <storage/flash_map.h>
#include <dk_buttons_and_leds.h>
#include <sid_pal_gpio_ifc.h>

#include <logging/log.h>
LOG_MODULE_REGISTER(sid_thread, CONFIG_SIDEWALK_LOG_LEVEL);

#if !FLASH_AREA_LABEL_EXISTS(mfg_storage)
	#error "Flash partition is not defined for the Sidewalk manufacturing storage!!"
#endif

#define SID_LED_INDICATE_CONNECTED   (DK_LED1)
#define SID_LED_INDICATE_INIT_ERROR  (DK_ALL_LEDS_MSK)

#ifdef CONFIG_SIDEWALK_LINK_MASK_BLE
#define LINK_MASK       (SID_LINK_TYPE_1)
#define LINK_MASK_START (LINK_MASK)
#elif  CONFIG_SIDEWALK_LINK_MASK_FSK
#define LINK_MASK       (SID_LINK_TYPE_2)
#define LINK_MASK_START (SID_LINK_TYPE_ANY)
#elif CONFIG_SIDEWALK_LINK_MASK_LORA
#define LINK_MASK       (SID_LINK_TYPE_3)
#define LINK_MASK_START (SID_LINK_TYPE_ANY)
#else
#error "Not defined Sidewalk link mask!!"
#endif

enum app_state {
	STATE_INIT,
	STATE_SIDEWALK_READY,
	STATE_SIDEWALK_NOT_READY,
	STATE_SIDEWALK_SECURE_CONNECTION,
};

struct link_status {
	enum sid_registration_status link_mask;
	enum sid_link_mode supported_link_mode[SID_LINK_TYPE_MAX_IDX];
};

typedef struct app_context {
	struct sid_handle *sidewalk_handle;
	enum app_state state;
	struct link_status link_status;
	uint8_t counter;
#if !defined(CONFIG_SIDEWALK_LINK_MASK_FSK) && !defined(CONFIG_SIDEWALK_LINK_MASK_LORA)
	bool connection_request;
#endif /* !defined(CONFIG_SIDEWALK_LINK_MASK_FSK) && !defined(CONFIG_SIDEWALK_LINK_MASK_LORA) */
} app_context_t;

static struct sid_msg sidewalk_msg;
static struct sid_msg_desc sidewalk_msg_desc = {
	.type = SID_MSG_TYPE_NOTIFY,
	.link_type = SID_LINK_TYPE_ANY,
	.link_mode = SID_LINK_MODE_CLOUD,
};

K_MSGQ_DEFINE(sid_msgq, sizeof(enum event_type), CONFIG_SIDEWALK_THREAD_QUEUE_SIZE, 4);
K_THREAD_STACK_DEFINE(sid_stack_area, CONFIG_SIDEWALK_THREAD_STACK_SIZE);
static struct k_thread sid_thread;
static k_tid_t sid_tid;

static const uint8_t *status_name[] = {
	"ready", "not ready", "Error", "secure channel ready"
};

static const uint8_t *link_mode_name[] = {
	"none", [SID_LINK_MODE_CLOUD] = "cloud", [SID_LINK_MODE_MOBILE] = "mobile"
};

static const uint8_t *link_mode_idx_name[] = {
	"ble", "fsk", "lora"
};

#if defined(CONFIG_SIDEWALK_LINK_MASK_FSK) || defined(CONFIG_SIDEWALK_LINK_MASK_LORA)
static struct sid_device_profile set_dp_cfg = {
	.unicast_params.device_profile_id = SID_LINK3_PROFILE_A,
	.unicast_params.rx_window_count = SID_RX_WINDOW_CNT_2,
	.unicast_params.unicast_window_interval.async_rx_interval_ms = SID_LINK3_RX_WINDOW_SEPARATION_3,
};

static struct sid_device_profile dev_cfg;
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

static const halo_serial_bus_factory_t radio_spi_factory =
{
	.create = bus_serial_ncs_spi_create,
	.config = NULL,
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
	.bus_selector = (halo_serial_bus_client_t){ .client_selector = 40 },
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
		.sleep_to_full_power_us = 406,
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

static void on_sidewalk_event(bool in_isr, void *context)
{
	LOG_DBG("on event, from %s, context %p", in_isr ? "ISR" : "App", context);
	sidewalk_thread_message_q_write(EVENT_TYPE_SIDEWALK);
}

static void on_sidewalk_msg_received(const struct sid_msg_desc *msg_desc, const struct sid_msg *msg, void *context)
{
	LOG_DBG("received message(type: %d, link_mode: %d, id: %u size %u)", (int)msg_desc->type,
		(int)msg_desc->link_mode, msg_desc->id, msg->size);
	LOG_HEXDUMP_INF((uint8_t *)msg->data, msg->size, "Message data: ");
}

static void on_sidewalk_msg_sent(const struct sid_msg_desc *msg_desc, void *context)
{
	LOG_DBG("sent message(type: %d, id: %u)", (int)msg_desc->type, msg_desc->id);
}

static void on_sidewalk_send_error(sid_error_t error, const struct sid_msg_desc *msg_desc, void *context)
{
	LOG_ERR("failed to send message(type: %d, id: %u), err:%d", (int)msg_desc->type, msg_desc->id, (int)error);
}

static void on_sidewalk_status_changed(const struct sid_status *status, void *context)
{
	LOG_INF("status changed: %s", status_name[status->state]);

	app_context_t *app_context = (app_context_t *)context;

	switch (status->state) {
	case SID_STATE_READY:
		dk_set_led_on(SID_LED_INDICATE_CONNECTED);
		app_context->state = STATE_SIDEWALK_READY;
#if !defined(CONFIG_SIDEWALK_LINK_MASK_FSK) && !defined(CONFIG_SIDEWALK_LINK_MASK_LORA)
		app_context->connection_request = false;
#endif /* !defined(CONFIG_SIDEWALK_LINK_MASK_FSK) && !defined(CONFIG_SIDEWALK_LINK_MASK_LORA) */
		break;
	case SID_STATE_NOT_READY:
		dk_set_led_off(SID_LED_INDICATE_CONNECTED);
		app_context->state = STATE_SIDEWALK_NOT_READY;
		break;
	case SID_STATE_ERROR:
		LOG_ERR("sidewalk error: %d", (int)sid_get_error(app_context->sidewalk_handle));
		break;
	case SID_STATE_SECURE_CHANNEL_READY:
		app_context->state = STATE_SIDEWALK_SECURE_CONNECTION;
		break;
	}

	LOG_INF("Device %sregistered, Time Sync %s, Link status %s",
		(SID_STATUS_REGISTERED == status->detail.registration_status) ? "Is " : "Un",
		(SID_STATUS_TIME_SYNCED == status->detail.time_sync_status) ? "Success" : "Fail",
		status->detail.link_status_mask ? "Up" : "Down");

	app_context->link_status.link_mask = (enum sid_registration_status)status->detail.link_status_mask;
	for (int i = 0; i < SID_LINK_TYPE_MAX_IDX; i++) {
		enum sid_link_mode mode = (enum sid_link_mode)status->detail.supported_link_modes[i];
		app_context->link_status.supported_link_mode[i] = mode;

		if (mode) {
			LOG_INF("Link mode %s, on %s", link_mode_name[mode], link_mode_idx_name[i]);
		}
	}
}

static void on_sidewalk_factory_reset(void *context)
{
	LOG_DBG("factory reset notification received from sid api");
	if (sid_hal_reset(SID_HAL_RESET_NORMAL)) {
		LOG_WRN("Reboot type not supported");
	}
}

#if !defined(CONFIG_SIDEWALK_LINK_MASK_FSK) && !defined(CONFIG_SIDEWALK_LINK_MASK_LORA)
static void connection_request(app_context_t *app_context)
{
	if (STATE_SIDEWALK_READY == app_context->state) {
		LOG_WRN("Sidewalk ready, operation not valid");
	} else {
		bool next = !app_context->connection_request;
		LOG_INF("%s connection request", next ? "Set" : "Clear");
		sid_error_t ret = sid_ble_bcn_connection_request(app_context->sidewalk_handle, next);
		if (SID_ERROR_NONE == ret) {
			app_context->connection_request = next;
		} else {
			LOG_ERR("Connection request failed %d", ret);
		}
	}
}

#else /* !defined(CONFIG_SIDEWALK_LINK_MASK_FSK) && !defined(CONFIG_SIDEWALK_LINK_MASK_LORA) */
static void set_device_profile(app_context_t *app_context, struct sid_device_profile *device_profile)
{
	sid_error_t ret = sid_option(app_context->sidewalk_handle, SID_OPTION_900MHZ_GET_DEVICE_PROFILE,
				     &dev_cfg, sizeof(dev_cfg));

	if (ret) {
		LOG_ERR("Option failed (err %d)", ret);
		return;
	}

	if (device_profile->unicast_params.device_profile_id != dev_cfg.unicast_params.device_profile_id
	    || device_profile->unicast_params.rx_window_count != dev_cfg.unicast_params.rx_window_count
	    || (device_profile->unicast_params.device_profile_id < SID_LINK3_PROFILE_A
		&& device_profile->unicast_params.unicast_window_interval.sync_rx_interval_ms
		!= dev_cfg.unicast_params.unicast_window_interval.sync_rx_interval_ms)
	    || (device_profile->unicast_params.device_profile_id >= SID_LINK3_PROFILE_A
		&& device_profile->unicast_params.unicast_window_interval.async_rx_interval_ms
		!= dev_cfg.unicast_params.unicast_window_interval.async_rx_interval_ms)) {
		ret = sid_option(app_context->sidewalk_handle, SID_OPTION_900MHZ_SET_DEVICE_PROFILE,
				 device_profile, sizeof(dev_cfg));
		if (ret) {
			LOG_ERR("Option failed (err %d)", ret);
		}
	} else {
		LOG_INF("Device profile is already set to the desired value");
	}
}

#endif /* !defined(CONFIG_SIDEWALK_LINK_MASK_FSK) && !defined(CONFIG_SIDEWALK_LINK_MASK_LORA) */

static void factory_reset(app_context_t *app_context)
{
	sid_error_t ret = sid_set_factory_reset(app_context->sidewalk_handle);

	if (SID_ERROR_NONE != ret) {
		LOG_ERR("Notification of factory reset to sid api failed!");
	} else {
		LOG_DBG("Wait for Sid api to notify to proceed with factory reset!");
	}
}

static void send_message(app_context_t *app_context)
{
	if (STATE_SIDEWALK_READY == app_context->state ||
	    STATE_SIDEWALK_SECURE_CONNECTION == app_context->state) {
		LOG_INF("sending counter update: %d", app_context->counter);

		sidewalk_msg.data = (uint8_t *)&app_context->counter;
		sidewalk_msg.size = sizeof(uint8_t);

		sidewalk_msg_desc.type = SID_MSG_TYPE_NOTIFY;
		sidewalk_msg_desc.link_type = SID_LINK_TYPE_ANY;
		sidewalk_msg_desc.link_mode = SID_LINK_MODE_CLOUD;

		if ((app_context->link_status.link_mask & SID_LINK_TYPE_1) &&
		    (app_context->link_status.supported_link_mode[SID_LINK_TYPE_1_IDX] & SID_LINK_MODE_MOBILE)) {
			sidewalk_msg_desc.link_mode = SID_LINK_MODE_MOBILE;
		}

		sid_error_t ret = sid_put_msg(app_context->sidewalk_handle, &sidewalk_msg, &sidewalk_msg_desc);
		if (SID_ERROR_NONE != ret) {
			LOG_ERR("failed queueing data, err:%d", (int) ret);
		} else {
			LOG_DBG("queued data message id:%u", sidewalk_msg_desc.id);
			app_context->counter++;
		}
	} else {
		LOG_ERR("sidewalk is not ready yet!");
	}
}

static void set_battery_level(app_context_t *app_context)
{
	static uint8_t fake_bat_lev = 70;

	++fake_bat_lev;
	if (fake_bat_lev > 100) {
		fake_bat_lev = 0;
	}
	sid_error_t ret = sid_option(app_context->sidewalk_handle, SID_OPTION_BLE_BATTERY_LEVEL,
				     &fake_bat_lev, sizeof(fake_bat_lev));
	if (SID_ERROR_NONE != ret) {
		LOG_ERR("failed setting sidewalk option!");
	} else {
		LOG_DBG("set battery level to %d", fake_bat_lev);
	}
}

#if defined(CONFIG_SIDEWALK_LINK_MASK_FSK) || defined(CONFIG_SIDEWALK_LINK_MASK_LORA)
static void initialize_radio_busy_gpio(void)
{
	LOG_DBG("Init Semtech busy pin");
	sid_pal_gpio_set_direction(radio_sx1262_cfg.gpio_radio_busy, SID_PAL_GPIO_DIRECTION_INPUT);
	sid_pal_gpio_input_mode(radio_sx1262_cfg.gpio_radio_busy, SID_PAL_GPIO_INPUT_CONNECT);
	sid_pal_gpio_pull_mode(radio_sx1262_cfg.gpio_radio_busy, SID_PAL_GPIO_PULL_NONE);
}

#endif

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
	initialize_radio_busy_gpio();
	set_radio_sx126x_device_config(&radio_sx1262_cfg);
#endif /* defined(CONFIG_SIDEWALK_LINK_MASK_FSK) || defined(CONFIG_SIDEWALK_LINK_MASK_LORA) */

	return SID_ERROR_NONE;
}

static sid_error_t sid_lib_run(app_context_t *app_context)
{
	static const sid_ble_link_config_t ble_link_config = {
		.create_ble_adapter = sid_pal_ble_adapter_create,
		.config = NULL,
	};

	struct sid_event_callbacks event_callbacks = {
		.context = app_context,
		.on_event = on_sidewalk_event,                          /* Called from ISR context */
		.on_msg_received = on_sidewalk_msg_received,            /* Called from sid_process() */
		.on_msg_sent = on_sidewalk_msg_sent,                    /* Called from sid_process() */
		.on_send_error = on_sidewalk_send_error,                /* Called from sid_process() */
		.on_status_changed = on_sidewalk_status_changed,        /* Called from sid_process() */
		.on_factory_reset = on_sidewalk_factory_reset,          /* Called from sid_process() */
	};

	struct sid_config config = {
		.link_mask = LINK_MASK,
		.callbacks = &event_callbacks,
		.link_config = &ble_link_config,
	};

	sid_error_t ret_code = sid_init(&config, &app_context->sidewalk_handle);

	if (SID_ERROR_NONE != ret_code) {
		LOG_ERR("failed to initialize sidewalk, err: %d", (int)ret_code);
		dk_set_leds(SID_LED_INDICATE_INIT_ERROR);
		sid_deinit(app_context->sidewalk_handle);
		return ret_code;
	}

	ret_code = sid_start(app_context->sidewalk_handle, LINK_MASK_START);
	if (SID_ERROR_NONE != ret_code) {
		LOG_ERR("failed to start sidewalk, err: %d", (int)ret_code);
		dk_set_leds(SID_LED_INDICATE_INIT_ERROR);
		return ret_code;
	}

	app_context->state = STATE_SIDEWALK_NOT_READY;
#if !defined(CONFIG_SIDEWALK_LINK_MASK_FSK) && !defined(CONFIG_SIDEWALK_LINK_MASK_LORA)
	app_context->connection_request = false;
#endif /* !defined(CONFIG_SIDEWALK_LINK_MASK_FSK) && !defined(CONFIG_SIDEWALK_LINK_MASK_LORA) */
	return SID_ERROR_NONE;
}

static void sidewalk_thread(void *context, void *u2, void *u3)
{
	ARG_UNUSED(u2);
	ARG_UNUSED(u3);

	app_context_t *sid_app_ctx = (app_context_t *)context;

	if (SID_ERROR_NONE != sid_pal_init()) {
		return;
	}

	if (SID_ERROR_NONE != sid_lib_run(sid_app_ctx)) {
		return;
	}

	LOG_INF("Starting sidewalk thread ...");

	while (1) {
		enum event_type event;

		if (!k_msgq_get(&sid_msgq, &event, K_FOREVER)) {
			switch (event) {
			case EVENT_TYPE_SIDEWALK:
			{
				sid_error_t ret = sid_process(sid_app_ctx->sidewalk_handle);
				if (ret) {
					LOG_WRN("Process error (code %d)", ret);
				}
				break;
			}
			case EVENT_TYPE_SEND_HELLO:
			{
				send_message(sid_app_ctx);
				break;
			}
			case EVENT_TYPE_SET_BATTERY_LEVEL:
			{
				set_battery_level(sid_app_ctx);
				break;
			}
#if !defined(CONFIG_SIDEWALK_LINK_MASK_FSK) && !defined(CONFIG_SIDEWALK_LINK_MASK_LORA)
			case EVENT_TYPE_CONNECTION_REQUEST:
			{
				connection_request(sid_app_ctx);
				break;
			}
#else /* !defined(CONFIG_SIDEWALK_LINK_MASK_FSK) && !defined(CONFIG_SIDEWALK_LINK_MASK_LORA) */
			case EVENT_TYPE_SET_DEVICE_PROFILE:
			{
				set_device_profile(sid_app_ctx, &set_dp_cfg);
				break;
			}
#endif /* !defined(CONFIG_SIDEWALK_LINK_MASK_FSK) && !defined(CONFIG_SIDEWALK_LINK_MASK_LORA) */
			case EVENT_TYPE_FACTORY_RESET:
			{
				factory_reset(sid_app_ctx);
				break;
			}
			default: break;
			}
		}
	}
}

void sidewalk_thread_message_q_write(enum event_type event)
{
	while (0 != k_msgq_put(&sid_msgq, &event, K_NO_WAIT)) {
		k_msgq_purge(&sid_msgq);
	}
}

void sidewalk_thread_enable(void)
{
	static app_context_t sid_app_ctx = {
		.sidewalk_handle = NULL,
		.state = STATE_INIT,
	};

	sid_tid = k_thread_create(&sid_thread, sid_stack_area,
				  K_THREAD_STACK_SIZEOF(sid_stack_area),
				  sidewalk_thread, &sid_app_ctx, NULL, NULL,
				  CONFIG_SIDEWALK_THREAD_PRIORITY, 0, K_NO_WAIT);
	ARG_UNUSED(sid_tid);
	k_thread_name_set(&sid_thread, "sidewalk");
}
