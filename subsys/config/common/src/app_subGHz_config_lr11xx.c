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
#include <lr11xx_config.h>

#include <app_subGHz_config.h>

#define REGION_US915
#define RADIO_REGION RADIO_REGION_NA

#define RADIO_LR11XX_MAX_TX_POWER 22
#define RADIO_LR11XX_MIN_TX_POWER -9

#define RADIO_MAX_TX_POWER_NA 20
#define RADIO_MAX_TX_POWER_EU 14

#if defined(REGION_ALL)
#define RADIO_REGION RADIO_REGION_NONE
#elif defined(REGION_US915)
#define RADIO_REGION RADIO_REGION_NA
#elif defined(REGION_EU868)
#define RADIO_REGION RADIO_REGION_EU
#endif

#define RADIO_LR11XX_SPI_BUFFER_SIZE 255
#define RADIO_RX_LNA_GAIN 0
#define RADIO_ANT_GAIN(X) ((X) * 100)

#define NULL_STRUCT_INITIALIZER { 0 }
#define INVALID_DT_GPIO NULL_STRUCT_INITIALIZER
#define SPI_FREQUENCY_DEFAULT (8UL*1000*1000)
#define LORA_DT DT_NODELABEL(lora_semtech_lr11xxmb1xxs)

/* Default values for optional DTS properties */
#define LR11XX_DEFAULT_TX_POWER_OFFSET 0
#define LR11XX_DEFAULT_RX_BOOSTED false
#define LR11XX_DEFAULT_RSSI_NO_SIGNAL_OFFSET 0
#define LR11XX_DEFAULT_WAKEUP_DELAY_US 0

static uint8_t radio_lr11xx_buffer[RADIO_LR11XX_SPI_BUFFER_SIZE] = { 0 };

static const struct sid_pal_serial_bus_factory radio_spi_factory = {
	.create = sid_pal_serial_bus_nordic_spi_create,
	.config = NULL,
};

// TODO: Isn't it duplicated in dts?
const uint8_t pa_duty_cycles[] = {
	//  -9,   -8,   -7,   -6,   -5,   -4,   -3,   -2,   -1,	0,	1,	2,	3,	4,	5,	6,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00,
	//   7,	8,   9,	10,   11,   12,   13,   14,   15,   16,   17,   18,   19,   20,   21,   22
	0x00, 0x00, 0x00, 0x01, 0x02, 0x02, 0x04, 0x05, 0x07, 0x03, 0x04, 0x02, 0x05, 0x03, 0x04,
	0x04
};
const uint8_t pa_hp_sels[] = {
	//  -9,   -8,   -7,   -6,   -5,   -4,   -3,   -2,   -1,	0,	1,	2,	3,	4,	5,	6,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00,
	//   7,	8,   9,	10,   11,   12,   13,   14,   15,   16,   17,   18,   19,   20,   21,   22
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x03, 0x05, 0x04, 0x07, 0x07,
	0x07
};
const uint8_t powers[] = {
	//  9, -8, -7, -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5,  6,
	-6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 3, 4, 7, 8, 9, 10,
	// 7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22
	12, 13, 14, 14, 13, 14, 14, 14, 14, 22, 22, 22, 22, 22, 21, 22
};


void on_wifi_scan_done(const void *);   // implement in application


__weak void on_wifi_scan_done(const void *) {}

__weak void *gnss_scan_done_context = NULL;
__weak void *wifi_scan_done_context = NULL;

// <pre>src/smtc_shield_lr1110mb1dxs_common.c</pre>
static int32_t radio_lr11xx_pa_cfg(int8_t tx_power, radio_lr11xx_pa_cfg_t *pa_cfg)
{
	int8_t pwr = tx_power;

	if (tx_power > RADIO_LR11XX_MAX_TX_POWER) {
		pwr = RADIO_LR11XX_MAX_TX_POWER;
	}

	if (tx_power < RADIO_LR11XX_MIN_TX_POWER) {
		pwr = RADIO_LR11XX_MIN_TX_POWER;
	}

	if (pwr > 15) {
		pa_cfg->pa_cfg.pa_reg_supply = LR11XX_RADIO_PA_REG_SUPPLY_VBAT;
		pa_cfg->pa_cfg.pa_sel = LR11XX_RADIO_PA_SEL_HP;
	} else {
		pa_cfg->pa_cfg.pa_reg_supply = LR11XX_RADIO_PA_REG_SUPPLY_VREG;
		pa_cfg->pa_cfg.pa_sel = LR11XX_RADIO_PA_SEL_LP;
	}
	pa_cfg->pa_cfg.pa_duty_cycle = pa_duty_cycles[pwr + 9];
	pa_cfg->pa_cfg.pa_hp_sel = pa_hp_sels[pwr + 9];

	pa_cfg->ramp_time = LR11XX_RADIO_RAMP_48_US;
	pa_cfg->tx_power_in_dbm = powers[pwr + 9];
	pa_cfg->enable_ext_pa = false;

	return 0;
}

static const radio_lr11xx_regional_param_t radio_lr11xx_regional_param[] = {
#if defined(REGION_ALL) || defined(REGION_US915)
	{ .param_region = RADIO_REGION_NA,
	  .max_tx_power = { RADIO_MAX_TX_POWER_NA, RADIO_MAX_TX_POWER_NA, RADIO_MAX_TX_POWER_NA,
			    RADIO_MAX_TX_POWER_NA, RADIO_MAX_TX_POWER_NA, RADIO_MAX_TX_POWER_NA },
	  .cca_level_adjust = { 0, 0, 0, 0, 0, 0 },
	  .ant_dbi = RADIO_ANT_GAIN(2.15) },
#endif
#if defined(REGION_ALL) || defined(REGION_EU868)
	{ .param_region = RADIO_REGION_EU,
	  .max_tx_power = { RADIO_MAX_TX_POWER_EU, RADIO_MAX_TX_POWER_EU, RADIO_MAX_TX_POWER_EU,
			    RADIO_MAX_TX_POWER_EU, RADIO_MAX_TX_POWER_EU, RADIO_MAX_TX_POWER_EU },
	  .cca_level_adjust = { 0, 0, 0, 0, 0, 0 },
	  .ant_dbi = RADIO_ANT_GAIN(2.15) },
#endif
};

static radio_lr11xx_device_config_t radio_lr11xx_cfg = {
    .regulator_mode = DT_PROP_OR(LORA_DT, reg_mode, LR11XX_SYSTEM_REG_MODE_DCDC),
    .rx_boost = DT_PROP_OR(LORA_DT, rx_boosted, LR11XX_DEFAULT_RX_BOOSTED),
    .lna_gain = RADIO_RX_LNA_GAIN,
    .bus_factory = &radio_spi_factory,

    .lfclock_cfg = DT_PROP_OR(LORA_DT, lf_clk, LR11XX_SYSTEM_LFCLK_XTAL),

    .wakeup_delay_us = LR11XX_DEFAULT_WAKEUP_DELAY_US,
    .pa_cfg_callback = radio_lr11xx_pa_cfg,
    .tcxo_config =
        {
            .ctrl = DT_PROP_OR(LORA_DT, tcxo_wakeup_time, 0) > 0 ?
                LR11XX_TCXO_CTRL_DIO3 : LR11XX_TCXO_CTRL_NONE,
            .tune = DT_PROP_OR(LORA_DT, tcxo_voltage, LR11XX_SYSTEM_TCXO_CTRL_1_8V),
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

    .rssi_no_signal_offset = LR11XX_DEFAULT_RSSI_NO_SIGNAL_OFFSET,

    .regional_config =
        {
            .radio_region = RADIO_REGION,
            .reg_param_table_size = sizeof(radio_lr11xx_regional_param) / sizeof(radio_lr11xx_regional_param[0]),
            .reg_param_table = radio_lr11xx_regional_param,
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
            .p = radio_lr11xx_buffer,
            .size = sizeof(radio_lr11xx_buffer),
        },

    .mitigations =
        {
            /* prevents servicing of GNSS interrupt */
            .irq_noise_during_sleep = true,
            .lbd_clear_on_wakeup = true,
        },
};

const void *get_radio_cfg(void)
{
	radio_lr11xx_cfg.gpios.led_tx =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec)GPIO_DT_SPEC_GET_OR(
			DT_PATH(zephyr_user), led_tx_gpios, INVALID_DT_GPIO));
	radio_lr11xx_cfg.gpios.led_rx =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec)GPIO_DT_SPEC_GET_OR(
			DT_PATH(zephyr_user), led_rx_gpios, INVALID_DT_GPIO));
	radio_lr11xx_cfg.gpios.gnss_lna =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec)GPIO_DT_SPEC_GET_OR(
		    DT_NODELABEL(lora_semtech_lr11xxmb1xxs), gnss_lna_gpios, INVALID_DT_GPIO));
	radio_lr11xx_cfg.gpios.led_sniff =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec)GPIO_DT_SPEC_GET_OR(
			DT_PATH(zephyr_user), led_sniff_gpios, INVALID_DT_GPIO));

	radio_lr11xx_cfg.gpios.power =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec)GPIO_DT_SPEC_GET_OR(
			LORA_DT, reset_gpios, INVALID_DT_GPIO));
	radio_lr11xx_cfg.gpios.int1 =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec)GPIO_DT_SPEC_GET_OR(
			LORA_DT, event_gpios, INVALID_DT_GPIO));
	radio_lr11xx_cfg.gpios.radio_busy =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec)GPIO_DT_SPEC_GET_OR(
			LORA_DT, busy_gpios, INVALID_DT_GPIO));
	radio_lr11xx_cfg.gpios.rf_sw_ena = GPIO_UNUSED_PIN;
	radio_lr11xx_cfg.gpios.tx_bypass = GPIO_UNUSED_PIN;
	radio_lr11xx_cfg.gpios.txrx = GPIO_UNUSED_PIN;
	radio_lr11xx_cfg.gpios.power_cb = NULL;
	radio_lr11xx_cfg.gpios.arg = NULL;
	radio_lr11xx_cfg.bus_selector.client_selector = 
		sid_gpio_utils_register_gpio((struct gpio_dt_spec)SPI_CS_GPIOS_DT_SPEC_GET(LORA_DT));
	radio_lr11xx_cfg.bus_selector.speed_hz = 
		DT_PROP_OR(LORA_DT, spi_max_frequency, SPI_FREQUENCY_DEFAULT);

	__ASSERT(radio_lr11xx_cfg.gpios.power < GPIO_UNUSED_PIN, "gpio_power invalid GPIO");
	__ASSERT(radio_lr11xx_cfg.gpios.int1 < GPIO_UNUSED_PIN, "gpio_int1 invalid GPIO");
	__ASSERT(radio_lr11xx_cfg.gpios.radio_busy < GPIO_UNUSED_PIN,
		 "gpio_radio_busy invalid GPIO");
	__ASSERT(radio_lr11xx_cfg.bus_selector.client_selector < GPIO_UNUSED_PIN,
		 "client_selector invalid GPIO");
	__ASSERT(radio_lr11xx_cfg.bus_selector.speed_hz != 0, "invalid speed of SPI = %d",
		 radio_lr11xx_cfg.bus_selector.speed_hz);


	radio_lr11xx_cfg.wifi_scan.post_hook = on_wifi_scan_done;
	radio_lr11xx_cfg.gnss_scan.arg = gnss_scan_done_context;
	radio_lr11xx_cfg.wifi_scan.arg = wifi_scan_done_context;

#ifdef CONFIG_LR11XX_CLI
	radio_lr11xx_cfg.state_timings.sleep_to_full_power_us = CLI_sleep_to_full_power_us;
#endif /* CONFIG_LR11XX_CLI */

	return &radio_lr11xx_cfg;
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
