/*
 * Copyright 2021-2025 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.TXT file. This file is a
 * Modifiable File, as defined in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#include <zephyr/kernel.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/spi.h>

#include <sid_pal_serial_bus_ifc.h>
#include <sid_pal_serial_bus_spi_config.h>
#include <sid_gpio_utils.h>
#include <lr1110_config.h>

#include <app_subGHz_config.h>

#define RADIO_MAX_CAD_SYMBOL SID_PAL_RADIO_LORA_CAD_04_SYMBOL

#define RADIO_REGION RADIO_REGION_NA
#define RADIO_MAX_TX_POWER_NA 0
#define RADIO_MAX_TX_POWER_EU 14
#define RADIO_LR1110_MAX_TX_POWER 22
#define RADIO_LR1110_MIN_TX_POWER 0

#if defined(REGION_ALL)
#define RADIO_REGION RADIO_REGION_NONE
#elif defined(REGION_US915)
#define RADIO_REGION RADIO_REGION_NA
#elif defined(REGION_EU868)
#define RADIO_REGION RADIO_REGION_EU
#endif

// Measurement based on RADIO_MAX_TX_POWER_NA, need one to one mapping for all power level
#define RADIO_MAX_TX_POWER_EXT_PA 5

#define RADIO_LR1110_SPI_BUFFER_SIZE 520
#define RADIO_RX_LNA_GAIN 0
#define RADIO_ANT_GAIN(X) ((X)*100)

#define NULL_STRUCT_INITIALIZER                                                                    \
	{                                                                                          \
		0                                                                                  \
	}
#define INVALID_DT_GPIO NULL_STRUCT_INITIALIZER
#define SPI_FREQUENCY_DEFAULT (8UL*1000*1000)
#define LORA_DT DT_NODELABEL(lora_semtech_lr11xxmb1xxs)

/* Default values for optional DTS properties */
#define LR1110_DEFAULT_TX_POWER_OFFSET 0
#define LR1110_DEFAULT_RX_BOOSTED false
#define LR1110_DEFAULT_RSSI_NO_SIGNAL_OFFSET 0
#define LR1110_DEFAULT_WAKEUP_DELAY_US 0

static uint8_t radio_lr1110_buffer[RADIO_LR1110_SPI_BUFFER_SIZE] = {0};

static const struct sid_pal_serial_bus_factory radio_spi_factory = {
    .create = sid_pal_serial_bus_nordic_spi_create,
    .config = NULL,
};

static int32_t radio_lr1110_pa_cfg(int8_t tx_power, radio_lr1110_pa_cfg_t *pa_cfg)
{
    if (tx_power > RADIO_MAX_TX_POWER_EXT_PA) {
        pa_cfg->tx_power_in_dbm = RADIO_MAX_TX_POWER_EXT_PA;
    } else {
        pa_cfg->tx_power_in_dbm = tx_power;
    }

    *pa_cfg = (radio_lr1110_pa_cfg_t){
        .pa_cfg =
            {
                .pa_sel = (pa_cfg->tx_power_in_dbm > 14) ? LR1110_RADIO_PA_SEL_HP : LR1110_RADIO_PA_SEL_LP,
                .pa_reg_supply =
                    (pa_cfg->tx_power_in_dbm > 14) ? LR1110_RADIO_PA_REG_SUPPLY_VBAT : LR1110_RADIO_PA_REG_SUPPLY_VREG,
                .pa_duty_cycle = 0x04,
                .pa_hp_sel = 0x07,
            },
        .ramp_time = LR1110_RADIO_RAMP_48_US,
        .tx_power_in_dbm = pa_cfg->tx_power_in_dbm,
    };

    return 0;
}

#define GPIO_WIFI_RF_SWITCH 16
#define GPIO_WIFI_RF_SWITCH_STATE 0
#define WIFI_SCAN_DELAY_MS 1

static void wifi_pre_scan(void *arg) {}

static void wifi_post_scan(void *arg) {}

#define GPIO_GNSS_RF_SWITCH 17
#define GPIO_GNSS_RF_SWITCH_STATE 1

static void gnss_pre_scan(void *arg) {}

static void gnss_post_scan(void *arg) {}

static const radio_lr1110_regional_param_t radio_lr1110_regional_param[] = {
#if defined(REGION_ALL) || defined(REGION_US915)
    {.param_region = RADIO_REGION_NA,
     .max_tx_power = {RADIO_MAX_TX_POWER_NA, RADIO_MAX_TX_POWER_NA, RADIO_MAX_TX_POWER_NA, RADIO_MAX_TX_POWER_NA,
                      RADIO_MAX_TX_POWER_NA, RADIO_MAX_TX_POWER_NA},
     .cca_level_adjust = {0, 0, 0, 0, 0, 0},
     .ant_dbi = RADIO_ANT_GAIN(2.15)},
#endif
#if defined(REGION_ALL) || defined(REGION_EU868)
    {.param_region = RADIO_REGION_EU,
     .max_tx_power = {RADIO_MAX_TX_POWER_EU, RADIO_MAX_TX_POWER_EU, RADIO_MAX_TX_POWER_EU, RADIO_MAX_TX_POWER_EU,
                      RADIO_MAX_TX_POWER_EU, RADIO_MAX_TX_POWER_EU},
     .cca_level_adjust = {0, 0, 0, 0, 0, 0},
     .ant_dbi = RADIO_ANT_GAIN(2.15)},
#endif
};

static radio_lr1110_device_config_t radio_lr1110_cfg = {
    .regulator_mode = DT_PROP_OR(LORA_DT, reg_mode, LR1110_SYSTEM_REG_MODE_DCDC),
    .rx_boost = DT_PROP_OR(LORA_DT, rx_boosted, LR1110_DEFAULT_RX_BOOSTED),
    .lna_gain = RADIO_RX_LNA_GAIN,
    .bus_factory = &radio_spi_factory,

    .lfclock_cfg = DT_PROP_OR(LORA_DT, lf_clk, LR1110_SYSTEM_LFCLK_XTAL),

    .wakeup_delay_us = LR1110_DEFAULT_WAKEUP_DELAY_US,
    .pa_cfg_callback = radio_lr1110_pa_cfg,
    .tcxo_config =
        {
            .ctrl = DT_PROP_OR(LORA_DT, tcxo_wakeup_time, 0) > 0 ?
                LR1110_TCXO_CTRL_DIO3 : LR1110_TCXO_CTRL_NONE,
            .tune = DT_PROP_OR(LORA_DT, tcxo_voltage, LR1110_SYSTEM_TCXO_CTRL_1_8V),
            // Convert ms to 30.25 us ticks (1 ms = 33.06 ticks)
            .timeout = DT_PROP_OR(LORA_DT, tcxo_wakeup_time, 0),
        },

    .rfswitch =
        {
            .enable = DT_PROP_OR(LORA_DT, rf_sw_enable, 0),
            .standby = DT_PROP_OR(LORA_DT, rf_sw_standby_mode, 0),
            .rx = DT_PROP_OR(LORA_DT, rf_sw_rx_mode, 0),
            .tx = DT_PROP_OR(LORA_DT, rf_sw_tx_mode, 0),
            .tx_hp = DT_PROP_OR(LORA_DT, rf_sw_tx_hp_mode, 0),
            .tx_hf = DT_PROP_OR(LORA_DT, rf_sw_tx_hf_mode, 0),
            .gnss = DT_PROP_OR(LORA_DT, rf_sw_gnss_mode, 0),
            .wifi = DT_PROP_OR(LORA_DT, rf_sw_wifi_mode, 0),
        },

    .rssi_no_signal_offset = LR1110_DEFAULT_RSSI_NO_SIGNAL_OFFSET,

    .wifi_scan =
        {
            .pre_hook = wifi_pre_scan,
            .post_hook = wifi_post_scan,
        },

    .gnss_scan =
        {
            .pre_hook = gnss_pre_scan,
            .post_hook = gnss_post_scan,
        },

    .regional_config =
        {
            .radio_region = RADIO_REGION,
            .reg_param_table_size = sizeof(radio_lr1110_regional_param) / sizeof(radio_lr1110_regional_param[0]),
            .reg_param_table = radio_lr1110_regional_param,
        },

    .state_timings =
        {
            .sleep_to_full_power_us = 643,   // 406, Revisit as part of HALO-54667
            .full_power_to_sleep_us = 0,
            .rx_to_tx_us = 0,
            .tx_to_rx_us = 0,
            .tcxo_delay_us = 0,
        },

    .internal_buffer =
        {
            .p = radio_lr1110_buffer,
            .size = sizeof(radio_lr1110_buffer),
        },

    .mitigations =
        {
            .lbd_clear_on_wakeup = false,
            .irq_noise_during_sleep = false,
        },
};

const void *get_radio_cfg(void)
{
	radio_lr1110_cfg.gpios.power =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec)GPIO_DT_SPEC_GET_OR(
			LORA_DT, reset_gpios, INVALID_DT_GPIO));
	radio_lr1110_cfg.gpios.int1 =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec)GPIO_DT_SPEC_GET_OR(
			LORA_DT, event_gpios, INVALID_DT_GPIO));
	radio_lr1110_cfg.gpios.radio_busy =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec)GPIO_DT_SPEC_GET_OR(
			LORA_DT, busy_gpios, INVALID_DT_GPIO));
	radio_lr1110_cfg.gpios.rf_sw_ena = GPIO_UNUSED_PIN;
	radio_lr1110_cfg.gpios.tx_bypass = GPIO_UNUSED_PIN;
	radio_lr1110_cfg.gpios.txrx = GPIO_UNUSED_PIN;
	radio_lr1110_cfg.gpios.power_cb = NULL;
	radio_lr1110_cfg.gpios.arg = NULL;
	radio_lr1110_cfg.bus_selector.client_selector = 
		sid_gpio_utils_register_gpio((struct gpio_dt_spec)SPI_CS_GPIOS_DT_SPEC_GET(LORA_DT));
	radio_lr1110_cfg.bus_selector.speed_hz = 
		DT_PROP_OR(LORA_DT, spi_max_frequency, SPI_FREQUENCY_DEFAULT);

	__ASSERT(radio_lr1110_cfg.gpios.power < GPIO_UNUSED_PIN, "gpio_power invalid GPIO");
	__ASSERT(radio_lr1110_cfg.gpios.int1 < GPIO_UNUSED_PIN, "gpio_int1 invalid GPIO");
	__ASSERT(radio_lr1110_cfg.gpios.radio_busy < GPIO_UNUSED_PIN, "gpio_radio_busy invalid GPIO");
	__ASSERT(radio_lr1110_cfg.bus_selector.client_selector < GPIO_UNUSED_PIN, "client_selector invalid GPIO");
	__ASSERT(radio_lr1110_cfg.bus_selector.speed_hz != 0, "invalid speed of SPI = %d", radio_lr1110_cfg.bus_selector.speed_hz);

	return &radio_lr1110_cfg;
}

struct sid_sub_ghz_links_config sub_ghz_link_config = {
    .enable_link_metrics = true,
    .metrics_msg_retries = 3,
    .sar_dcr = 100,
    .registration_config =
        {
            .enable = true,
            .periodicity_s = UINT32_MAX,
        },
    .link2_max_tx_power_in_dbm = RADIO_MAX_TX_POWER_NA,
    .link3_max_tx_power_in_dbm = RADIO_MAX_TX_POWER_NA,
};

struct sid_sub_ghz_links_config *app_get_sub_ghz_config(void)
{
    return &sub_ghz_link_config;
}
