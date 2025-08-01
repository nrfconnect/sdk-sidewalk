/*
 * Copyright 2020-2022 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#include <sid_pal_serial_bus_ifc.h>
#include <sid_pal_serial_bus_spi_config.h>
#include <sid_gpio_utils.h>
#include <sx126x_config.h>

#include <app_subGHz_config.h>

#define REGION_US915

/* This product has no external PA and SX1262 can support max of 22dBm*/
#define RADIO_SX1262_MAX_TX_POWER 22
#define RADIO_SX1262_MIN_TX_POWER -9

#define RADIO_MAX_TX_POWER_NA 20
#define RADIO_MAX_TX_POWER_EU 14

#define RADIO_REGION RADIO_REGION_NA

#define RADIO_SX1262_SPI_BUFFER_SIZE 255

#define RADIO_SX1262_PA_DUTY_CYCLE 0x04
#define RADIO_SX1262_HP_MAX 0x07
#define RADIO_SX1262_DEVICE_SEL 0x00
#define RADIO_SX1262_PA_LUT 0x01

#define RADIO_RX_LNA_GAIN 0
#define RADIO_MAX_CAD_SYMBOL SID_PAL_RADIO_LORA_CAD_04_SYMBOL
#define RADIO_ANT_GAIN(X) ((X) * 100)

#define NULL_STRUCT_INITIALIZER { 0 }
#define INVALID_DT_GPIO NULL_STRUCT_INITIALIZER
#define SPI_FREQUENCY_DEFAULT (8UL * 1000 * 1000)

/* Default values for optional DTS properties */
#define SX126X_DEFAULT_XTAL_CAP_XTA 0x12
#define SX126X_DEFAULT_XTAL_CAP_XTB 0x12
#define SX126X_DEFAULT_TX_POWER_OFFSET 0
#define SX126X_DEFAULT_RX_BOOSTED false

static uint8_t radio_sx1262_buffer[RADIO_SX1262_SPI_BUFFER_SIZE] = { 0 };

static const struct sid_pal_serial_bus_factory radio_spi_factory = {
	.create = sid_pal_serial_bus_nordic_spi_create,
	.config = NULL,
};

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
	pa_cfg->tx_power = pwr; // one to one mapping between tx params and tx power
	pa_cfg->ramp_time = RADIO_SX126X_RAMP_40_US;

	return 0;
}

static int32_t radio_sx1262_trim_val(uint16_t *trim_val)
{
	*trim_val = CONFIG_SIDEWALK_SUBGHZ_TRIM_CAP_VAL;
	return 0;
}

const radio_sx126x_regional_param_t radio_sx126x_regional_param[] = {
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

static radio_sx126x_device_config_t radio_sx1262_cfg = {
	.id = SEMTECH_ID_SX1262,                     // chip id register not supported
	.regulator_mode = DT_PROP_OR(DT_NODELABEL(lora_semtech_sx126xmb2xxs), reg_mode, RADIO_SX126X_REGULATOR_DCDC),
	.rx_boost = DT_PROP_OR(DT_NODELABEL(lora_semtech_sx126xmb2xxs), rx_boosted, SX126X_DEFAULT_RX_BOOSTED),
	.lna_gain = RADIO_RX_LNA_GAIN,
	.bus_factory = &radio_spi_factory,

	.pa_cfg_callback = radio_sx1262_pa_cfg,

	.tcxo = {
		.ctrl = DT_PROP(DT_NODELABEL(lora_semtech_sx126xmb2xxs), dio3_as_tcxo_control) ?
			SX126X_TCXO_CTRL_DIO3 : SX126X_TCXO_CTRL_NONE,
		.voltage = DT_PROP_OR(DT_NODELABEL(lora_semtech_sx126xmb2xxs), tcxo_voltage, SX126X_TCXO_CTRL_1_8V),
		.timeout = DT_PROP_OR(DT_NODELABEL(lora_semtech_sx126xmb2xxs), tcxo_wakeup_time, 0),
	},

	.trim_cap_val_callback = radio_sx1262_trim_val,

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

	.bus_selector.speed_hz =
		DT_PROP_OR(DT_NODELABEL(lora_semtech_sx126xmb2xxs), spi_max_frequency, SPI_FREQUENCY_DEFAULT),
};

const radio_sx126x_device_config_t *get_radio_cfg(void)
{
	radio_sx1262_cfg.gpio_power =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec)GPIO_DT_SPEC_GET_OR(
			DT_NODELABEL(lora_semtech_sx126xmb2xxs), reset_gpios, INVALID_DT_GPIO));
	radio_sx1262_cfg.gpio_int1 =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec)GPIO_DT_SPEC_GET_OR(
			DT_NODELABEL(lora_semtech_sx126xmb2xxs), dio1_gpios, INVALID_DT_GPIO));
	radio_sx1262_cfg.gpio_radio_busy =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec)GPIO_DT_SPEC_GET_OR(
			DT_NODELABEL(lora_semtech_sx126xmb2xxs), busy_gpios, INVALID_DT_GPIO));
	radio_sx1262_cfg.bus_selector.client_selector =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec)GPIO_DT_SPEC_GET_OR(
			DT_NODELABEL(arduino_spi), cs_gpios, INVALID_DT_GPIO));

	__ASSERT(radio_sx1262_cfg.gpio_power < GPIO_UNUSED_PIN, "gpio_power invalid GPIO");
	__ASSERT(radio_sx1262_cfg.gpio_int1 < GPIO_UNUSED_PIN, "gpio_int1 invalid GPIO");
	__ASSERT(radio_sx1262_cfg.gpio_radio_busy < GPIO_UNUSED_PIN,
		 "gpio_radio_busy invalid GPIO");
	__ASSERT(radio_sx1262_cfg.bus_selector.client_selector < GPIO_UNUSED_PIN,
		 "client_selector invalid GPIO");
	__ASSERT(radio_sx1262_cfg.bus_selector.speed_hz != 0, "invalid speed of SPI = %d",
		 radio_sx1262_cfg.bus_selector.speed_hz);

	radio_sx1262_cfg.gpio_rf_sw_ena = GPIO_UNUSED_PIN;
	radio_sx1262_cfg.gpio_tx_bypass = GPIO_UNUSED_PIN;

	__ASSERT(radio_sx1262_cfg.gpio_tx_bypass <= GPIO_UNUSED_PIN, "gpio_tx_bypass invalid GPIO");

	return &radio_sx1262_cfg;
}

struct sid_sub_ghz_links_config sub_ghz_link_config = {
	.enable_link_metrics = true,
	.sar_dcr = 100,
	.registration_config = {
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
