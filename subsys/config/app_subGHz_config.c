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

#include <sid_pal_serial_bus_ifc.h>
#include <sid_pal_serial_bus_spi_config.h>
#include <sx126x_config.h>

#include "app_subGHz_config.h"

#define REGION_US915

/* This product has no external PA and SX1262 can support max of 22dBm*/
#define RADIO_SX1262_MAX_TX_POWER                                  22
#define RADIO_SX1262_MIN_TX_POWER                                  -9

#define RADIO_MAX_TX_POWER_NA                                      20
#define RADIO_MAX_TX_POWER_EU                                      14

#define RADIO_REGION                                               RADIO_REGION_NA

#define RADIO_SX1262_SPI_BUFFER_SIZE                               255

#define RADIO_SX1262_PA_DUTY_CYCLE                                 0x04
#define RADIO_SX1262_HP_MAX                                        0x07
#define RADIO_SX1262_DEVICE_SEL                                    0x00
#define RADIO_SX1262_PA_LUT                                        0x01

#define RADIO_RX_LNA_GAIN                                          0
#define RADIO_MAX_CAD_SYMBOL                                       SID_PAL_RADIO_LORA_CAD_04_SYMBOL
#define RADIO_ANT_GAIN(X)                                          ((X) * 100)

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

	.pa_cfg_callback = radio_sx1262_pa_cfg,

	.bus_selector = {
		.client_selector = 40, // sx1262_NSS
		.speed_hz = 0,
		.bit_order = SID_PAL_SERIAL_BUS_BIT_ORDER_MSB_FIRST,
		.mode = 0,
	},

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

const radio_sx126x_device_config_t *get_radio_cfg(void)
{
	return &radio_sx1262_cfg;
}

const struct sid_sub_ghz_links_config sub_ghz_link_config = {
	.enable_link_metrics = true,
	.registration_config = {
		.enable = true,
		.periodicity_s = UINT32_MAX,
	},
};

const struct sid_sub_ghz_links_config *app_get_sub_ghz_config(void)
{
	return &sub_ghz_link_config;
}
