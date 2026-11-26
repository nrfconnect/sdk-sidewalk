/*
 * Copyright (c) 2026 Seeed Technology Co., Ltd.
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 *
 * Devicetree → radio_lr20xx_device_config_t translation.
 *
 * Builds the usp_zephyr-style hal_cfg (spi_dt_spec / gpio_dt_spec) directly
 * from DT — no sid_pal_serial_bus indirection. The HAL reads ctx->cfg which
 * points to config->hal_cfg.
 *
 * Fills SPI/reset/busy/reg-mode/lf-clk and the DIO IRQ routing; the optional
 * current-draw tables are left NULL (board-measured data not available).
 */

#include <zephyr/kernel.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/spi.h>

#include <lr20xx_config.h>
#include <lr20xx_system_types.h>

#include <app_subGHz_config.h>

#define REGION_US915
#define RADIO_REGION RADIO_REGION_NA

#define LORA_DT DT_NODELABEL(lora_semtech_lr2021)

/* Regional parameters: NA (US915). Max TX power 20 dBm, antenna gain 2.15 dBi.
 * cca_level_adjust 0 pending board-measured values. Mirrors the lr11xx NA
 * configuration. */
#define RADIO_ANT_GAIN( X ) ( ( X ) * 100 )
#define RADIO_MAX_TX_POWER_NA 20

static const radio_lr20xx_regional_param_t radio_lr20xx_regional_param[] = {
	{
		.param_region = RADIO_REGION_NA,
		.max_tx_power = { RADIO_MAX_TX_POWER_NA, RADIO_MAX_TX_POWER_NA, RADIO_MAX_TX_POWER_NA,
				  RADIO_MAX_TX_POWER_NA, RADIO_MAX_TX_POWER_NA, RADIO_MAX_TX_POWER_NA,
				  RADIO_MAX_TX_POWER_NA },
		.cca_level_adjust = { 0, 0, 0, 0, 0, 0, 0 },
		.ant_dbi = RADIO_ANT_GAIN( 2.15 ),
	},
};

/*
 * DIO config: route ALL chip IRQs to DIO8 (the IRQ-capable DIO wired to the
 * nRF event-gpios pin). usp_zephyr semtech_wio_lr20xx_common.dtsi maps IRQ to
 * dio@8 on the LR2021 module. The nRF-side GPIO interrupt is set up in
 * sid_pal_radio_init.
 */
static lr20xx_dio_cfg_t lr20xx_dios[] = {
	{
		.dio = LR20XX_SYSTEM_DIO_8,
		.function = LR20XX_SYSTEM_DIO_FUNC_IRQ,
		.sleep_drive = LR20XX_SYSTEM_DIO_DRIVE_NONE,
		.irq_mask = LR20XX_SYSTEM_IRQ_ALL_MASK,
		.gpio = GPIO_DT_SPEC_GET( LORA_DT, event_gpios ),
		.rf_switch_cfg = 0,
	},
};

/*
 * LR2021 PA power tables (from usp_zephyr semtech_wio_lr20xx_region_eu_us.dtsi,
 * Semtech measured). LF = sub-GHz (@915MHz), HF = 2.4GHz (@2445MHz).
 * Index: LF[power - (-10)] for -10..22 dBm (33 entries);
 *        HF[power - (-17)] for -17..12 dBm (30 entries).
 * Each entry = { half_power, pa_duty_cycle, pa_lf_slices } (pa_lf_slices reused
 * as pa_hp_sel for HF). Replace with board-measured values when available.
 */
static lr20xx_pa_pwr_cfg_t lr20xx_pa_lf_cfg_table[] = {
	{ -8, 0x03, 0x00 }, { -6, 0x03, 0x00 }, {  0, 0x01, 0x00 }, {  2, 0x01, 0x00 },
	{  2, 0x02, 0x00 }, {  4, 0x02, 0x00 }, {  8, 0x00, 0x01 }, {  6, 0x01, 0x02 },
	{  6, 0x05, 0x00 }, { 12, 0x01, 0x01 }, { 16, 0x01, 0x00 }, { 18, 0x01, 0x00 },
	{ 18, 0x02, 0x00 }, { 20, 0x02, 0x00 }, { 22, 0x02, 0x00 }, { 16, 0x05, 0x01 },
	{ 28, 0x00, 0x01 }, { 28, 0x00, 0x02 }, { 28, 0x04, 0x00 }, { 30, 0x01, 0x02 },
	{ 32, 0x01, 0x02 }, { 28, 0x04, 0x02 }, { 30, 0x04, 0x02 }, { 32, 0x04, 0x02 },
	{ 34, 0x04, 0x02 }, { 36, 0x04, 0x02 }, { 34, 0x07, 0x03 }, { 38, 0x04, 0x04 },
	{ 38, 0x05, 0x07 }, { 40, 0x05, 0x07 }, { 42, 0x05, 0x06 }, { 44, 0x05, 0x06 },
	{ 44, 0x07, 0x07 },
};

static lr20xx_pa_pwr_cfg_t lr20xx_pa_hf_cfg_table[] = {
	{ -38, 30, 0x07 }, { -36, 27, 0x07 }, { -34, 24, 0x07 }, { -30, 28, 0x07 },
	{ -28, 25, 0x07 }, { -26, 22, 0x07 }, { -24, 20, 0x07 }, { -22, 16, 0x07 },
	{ -18, 27, 0x07 }, { -14, 30, 0x07 }, { -14, 24, 0x07 }, { -12, 24, 0x07 },
	{ -10, 22, 0x07 }, {  -4, 31, 0x07 }, {  -4, 29, 0x07 }, {  -4, 18, 0x07 },
	{   0, 28, 0x07 }, {   2, 28, 0x07 }, {   4, 28, 0x07 }, {   6, 27, 0x07 },
	{   8, 27, 0x07 }, {  10, 27, 0x07 }, {  12, 27, 0x07 }, {  14, 27, 0x07 },
	{  16, 27, 0x07 }, {  20, 30, 0x07 }, {  22, 30, 0x07 }, {  24, 30, 0x07 },
	{  24, 27, 0x07 }, {  24, 16, 0x07 },
};

/* Front-end calibration frequencies (usp_zephyr semtech_wio_lr20xx_region_eu_us.dtsi,
 * measured @915MHz / @2445MHz). Used by CalibFe during ral_lr20xx_init. */
static uint32_t lr20xx_calibration_freqs[] = {
	870000000,   /* Frequency 0 (range 850-890 MHz) */
	920000000,   /* Frequency 1 (range 900-940 MHz) */
	2441000000,  /* Frequency 2 (range 2.391-2.491 GHz) */
};

static radio_lr20xx_device_config_t radio_lr20xx_cfg = {
	.hal_cfg = {
		.spi = SPI_DT_SPEC_GET( LORA_DT, SPI_WORD_SET( 8 ) | SPI_TRANSFER_MSB ),
		.reset = GPIO_DT_SPEC_GET( LORA_DT, reset_gpios ),
		.busy = GPIO_DT_SPEC_GET( LORA_DT, busy_gpios ),
		.dios_config_num = ARRAY_SIZE( lr20xx_dios ),
		.dios_config = lr20xx_dios,
		.hf_clk_out_scaling = LR20XX_SYSTEM_HF_CLK_SCALING_32_MHZ,
		.tcxo_cfg = {
			/* tcxo-wakeup-time = 0 → XTAL mode (RAL_XOSC_CFG_XTAL);
			 * >0 → TCXO radio-controlled (RAL_XOSC_CFG_TCXO_RADIO_CTRL). */
			.xosc_cfg = ( DT_PROP( LORA_DT, tcxo_wakeup_time ) > 0 )
			              ? RAL_XOSC_CFG_TCXO_RADIO_CTRL
			              : RAL_XOSC_CFG_XTAL,
			.voltage = DT_PROP_OR( LORA_DT, tcxo_voltage, LR20XX_SYSTEM_TCXO_CTRL_1_8V ),
			.wakeup_time_ms = DT_PROP( LORA_DT, tcxo_wakeup_time ),
		},
		.lf_clck_cfg = { .lf_clk_cfg = DT_PROP_OR( LORA_DT, lf_clk, LR20XX_SYSTEM_LFCLK_RC ),
		                 .wait_32k_ready = false },
		.reg_mode = DT_PROP_OR( LORA_DT, reg_mode, LR20XX_SYSTEM_REG_MODE_DCDC ),
		.tx_power_offset_db = DT_PROP_OR( LORA_DT, tx_power_offset, 0 ),
		.rx_boosted_cfg = LR20XX_RADIO_COMMON_RX_PATH_BOOST_MODE_NONE,
		.pa_ramp_time = LR20XX_RADIO_COMMON_RAMP_48_US,
		/* PA tables: LR2021 measured (usp_zephyr), LF @915MHz + HF @2.4GHz.
		 * Optional current-draw tables (tx_dbm_to_ua / rx_bw_to_ua): NULL,
		 * board-measured data not available. */
		.pa_lf_cfg_table = lr20xx_pa_lf_cfg_table,
		.pa_hf_cfg_table = lr20xx_pa_hf_cfg_table,
		.tx_dbm_to_ua_reg_mode_dcdc_lf_vreg = NULL,
		.tx_dbm_to_ua_reg_mode_ldo_lf_vreg = NULL,
		.tx_dbm_to_ua_reg_mode_dcdc_hf_vreg = NULL,
		.rx_bw_to_ua_reg_mode_dcdc_lf_vreg = NULL,
		.rx_bw_to_ua_reg_mode_dcdc_hf_vreg = NULL,
		.rx_bw_to_ua_reg_mode_dcdc_lf_vreg_boosted = NULL,
		.rx_bw_to_ua_reg_mode_dcdc_hf_vreg_boosted = NULL,
		.rx_bw_to_ua_reg_mode_ldo_lf_vreg = NULL,
		.rx_bw_to_ua_reg_mode_ldo_hf_vreg = NULL,
		.rx_bw_to_ua_reg_mode_ldo_lf_vreg_boosted = NULL,
		.rx_bw_to_ua_reg_mode_ldo_hf_vreg_boosted = NULL,
		/* Front-end calibration frequencies (usp_zephyr semtech_wio_lr20xx_region_eu_us.dtsi).
		 * Without these, CalibFe runs with freq=0 → chip rejects → CMD_ERROR,
		 * which may block subsequent TX commands → HF_XOSC_START. */
		.calibration_freqs = lr20xx_calibration_freqs,
	},
	/* State-transition timing estimates. TCXO wakeup is 2 ms (tcxo-wakeup-time
	 * = <2> in the shield DTS), dominating sleep_to_full_power_us /
	 * tcxo_delay_us; tx/rx process delays borrow LR11xx values (same chip
	 * family) as a starting point. */
	.state_timings = {
		.sleep_to_full_power_us = 3400,  /* TCXO 2 ms + wake/SPI overhead */
		.full_power_to_sleep_us = 120,   /* enter sleep (RTC set + standby) */
		.rx_to_tx_us            = 200,   /* RX -> TX turnaround */
		.tx_to_rx_us            = 200,   /* TX last bit -> RX ready */
		.tcxo_delay_us          = 2000,  /* TCXO startup (tcxo-wakeup-time) */
		.tx_delay_us            = 336,   /* TX process delay (LR11xx estimate) */
		.rx_delay_us            = 286,   /* RX process delay (LR11xx estimate) */
	},
	.regional_config = {
		.radio_region = RADIO_REGION,
		.reg_param_table_size =
			sizeof( radio_lr20xx_regional_param ) / sizeof( radio_lr20xx_regional_param[0] ),
		.reg_param_table = radio_lr20xx_regional_param,
	},
};

const void *get_radio_cfg( void )
{
	return &radio_lr20xx_cfg;
}

/* Sub-GHz link configuration consumed by the sample app (app.c calls
 * app_get_sub_ghz_config). Mirrors app_subGHz_config_lr11xx.c. */
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

struct sid_sub_ghz_links_config *app_get_sub_ghz_config( void )
{
	return &sub_ghz_link_config;
}
