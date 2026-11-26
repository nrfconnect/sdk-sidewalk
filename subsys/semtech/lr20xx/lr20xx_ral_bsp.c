/*
 * Copyright (c) 2025 Semtech Corporation. All rights reserved.
 * Copyright (c) 2026 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * LR20xx RAL BSP for the Sidewalk PAL. Adapted from usp_zephyr
 * drivers/usp/lr20xx/lr20xx_ral_bsp.c + lr20xx_ral_bsp_calibration.c,
 * with signatures corrected to match the vendored ral_lr20xx_bsp.h.
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "halo_lr20xx_radio.h"
#include "lr20xx_hal_context.h"
#include "lr20xx_radio_common.h"
#include "ral_lr20xx_bsp.h"
#include "sl_sidewalk_log_app.h"

#include <zephyr/kernel.h>

/* PA power range constants (usp_zephyr lr20xx_ral_bsp_calibration.c).
 * LF (sub-GHz): -10..22 dBm; HF (2.4GHz): -17..12 dBm. Tables come from the
 * hal_cfg (app_subGHz_config_lr20xx.c), populated from usp measured data. */
#define LR20XX_LF_MIN_OUTPUT_POWER (-10)
#define LR20XX_LF_MAX_OUTPUT_POWER 22
#define LR20XX_HF_MIN_OUTPUT_POWER (-17)
#define LR20XX_HF_MAX_OUTPUT_POWER 12

/* GFSK RX consumption in uA (usp examples/radio_hal/ral_lr20xx_bsp.c,
 * measured at 3.3V). Chip-level constants, not board-specific. */
#define LR20XX_GFSK_RX_CONSUMPTION_DCDC        5410
#define LR20XX_GFSK_RX_BOOSTED_CONSUMPTION_DCDC 6970
#define LR20XX_GFSK_RX_CONSUMPTION_LDO         9500
#define LR20XX_GFSK_RX_BOOSTED_CONSUMPTION_LDO 11730

/* ---- Helper ---- */
static const lr20xx_dio_cfg_t* lr20xx_get_dio_cfg( const struct lr20xx_hal_context_cfg_t* cfg,
                                                    lr20xx_system_dio_t dio )
{
    for( int i = 0; i < cfg->dios_config_num; i++ )
    {
        if( cfg->dios_config[i].dio == dio )
        {
            return &cfg->dios_config[i];
        }
    }
    return NULL;
}

/* ---- BSP config functions ---- */

void ral_lr20xx_bsp_get_rx_cfg( const void* context, const uint32_t freq_in_hz,
                                lr20xx_radio_common_rx_path_t* rx_path,
                                lr20xx_radio_common_rx_path_boost_mode_t* boost_mode )
{
    const halo_drv_lr20xx_ctx_t* ctx = context;
    *rx_path = ( freq_in_hz >= 1500000000 ) ? LR20XX_RADIO_COMMON_RX_PATH_HF
                                             : LR20XX_RADIO_COMMON_RX_PATH_LF;
    *boost_mode = ctx->cfg->rx_boosted_cfg;
}

void ral_lr20xx_bsp_get_dio_function( const void* context, lr20xx_system_dio_t dio,
                                       lr20xx_system_dio_func_t* function )
{
    const halo_drv_lr20xx_ctx_t* ctx = context;
    const lr20xx_dio_cfg_t* dc = lr20xx_get_dio_cfg( ctx->cfg, dio );
    if( dc ) { *function = dc->function; }
}

void ral_lr20xx_bsp_get_dio_sleep_drive( const void* context, lr20xx_system_dio_t dio,
                                         lr20xx_system_dio_drive_t* drive )
{
    const halo_drv_lr20xx_ctx_t* ctx = context;
    const lr20xx_dio_cfg_t* dc = lr20xx_get_dio_cfg( ctx->cfg, dio );
    if( dc ) { *drive = dc->sleep_drive; }
}

void ral_lr20xx_bsp_get_dio_irq_mask( const void* context, lr20xx_system_dio_t dio,
                                      lr20xx_system_irq_mask_t* irq_mask )
{
    const halo_drv_lr20xx_ctx_t* ctx = context;
    const lr20xx_dio_cfg_t* dc = lr20xx_get_dio_cfg( ctx->cfg, dio );
    if( dc ) { *irq_mask = dc->irq_mask; }
}

void ral_lr20xx_bsp_get_dio_rf_switch_cfg( const void* context, lr20xx_system_dio_t dio,
                                           lr20xx_system_dio_rf_switch_cfg_t* rf_switch_cfg )
{
    const halo_drv_lr20xx_ctx_t* ctx = context;
    const lr20xx_dio_cfg_t* dc = lr20xx_get_dio_cfg( ctx->cfg, dio );
    *rf_switch_cfg = dc ? dc->rf_switch_cfg : 0;
}

void ral_lr20xx_bsp_get_reg_mode( const void* context, lr20xx_system_reg_mode_t* reg_mode )
{
    const halo_drv_lr20xx_ctx_t* ctx = context;
    *reg_mode = ctx->cfg->reg_mode;
}

void ral_lr20xx_bsp_get_dio_hf_clk_scaling_cfg( const void* context,
                                                 lr20xx_system_hf_clk_scaling_t* hf_clk_scaling )
{
    const halo_drv_lr20xx_ctx_t* ctx = context;
    *hf_clk_scaling = ctx->cfg->hf_clk_out_scaling;
}

void ral_bsp_lr20xx_get_lfclk_cfg( const void* context, lr20xx_system_lfclk_cfg_t* lfclk_cfg )
{
    const halo_drv_lr20xx_ctx_t* ctx = context;
    *lfclk_cfg = ctx->cfg->lf_clck_cfg.lf_clk_cfg;
}

void ral_lr20xx_bsp_get_xosc_cfg( const void* context, ral_xosc_cfg_t* xosc_cfg,
                                  lr20xx_system_tcxo_supply_voltage_t* supply_voltage,
                                  uint32_t* startup_time_in_tick )
{
    const halo_drv_lr20xx_ctx_t* ctx = context;
    const struct lr20xx_hal_context_tcxo_cfg_t tcxo = ctx->cfg->tcxo_cfg;
    *xosc_cfg = tcxo.xosc_cfg;
    *supply_voltage = tcxo.voltage;
    /* TCXO startup timer uses 32 MHz ticks, NOT the 32 kHz RTC.
     * lr20xx_radio_common_convert_time_in_ms_to_rtc_step uses LR20XX_RTC_FREQ_IN_HZ=32768
     * (32 kHz) which gives 1000x too short a delay → HF_XOSC_START_ERR.
     * Reference nRF5 SDK project: 2ms → 64000 ticks (= 2ms * 32MHz / 1000). */
    *startup_time_in_tick = tcxo.wakeup_time_ms * 32000000UL / 1000;
}

void ral_lr20xx_bsp_get_lora_cad_det_peak( const void* context, ral_lora_sf_t sf,
                                            ral_lora_cad_symbs_t nb_symbol, uint8_t* in_out_cad_det_peak )
{
    (void) context; (void) sf; (void) nb_symbol; (void) in_out_cad_det_peak;
}

/* ---- TX config (PA table lookup, config-based per usp lr20xx_ral_bsp_calibration.c) ---- */

void ral_lr20xx_bsp_get_tx_cfg( const void* context,
                                const ral_lr20xx_bsp_tx_cfg_input_params_t* input_params,
                                ral_lr20xx_bsp_tx_cfg_output_params_t* output_params )
{
    const halo_drv_lr20xx_ctx_t* ctx = context;
    int8_t power = input_params->system_output_pwr_in_dbm + ctx->cfg->tx_power_offset_db;
    output_params->pa_ramp_time = ctx->cfg->pa_ramp_time;

    if( input_params->freq_in_hz < 1500000000 )
    {
        /* LF path (sub-GHz, @915MHz). */
        if( power < LR20XX_LF_MIN_OUTPUT_POWER ) power = LR20XX_LF_MIN_OUTPUT_POWER;
        if( power > LR20XX_LF_MAX_OUTPUT_POWER ) power = LR20XX_LF_MAX_OUTPUT_POWER;
        const lr20xx_pa_pwr_cfg_t* pwr_cfg = &ctx->cfg->pa_lf_cfg_table[power - LR20XX_LF_MIN_OUTPUT_POWER];
        output_params->pa_cfg.pa_sel           = LR20XX_RADIO_COMMON_PA_SEL_LF;
        output_params->pa_cfg.pa_lf_mode       = LR20XX_RADIO_COMMON_PA_LF_MODE_FSM;
        output_params->pa_cfg.pa_lf_slices     = pwr_cfg->pa_lf_slices;
        output_params->pa_cfg.pa_lf_duty_cycle = pwr_cfg->pa_duty_cycle;
        output_params->pa_cfg.pa_hf_duty_cycle = 16;
        output_params->chip_output_half_pwr_in_dbm_configured = pwr_cfg->half_power;
        output_params->chip_output_pwr_in_dbm_expected        = power;
    }
    else
    {
        /* HF path (2.4GHz, @2445MHz). */
        if( power < LR20XX_HF_MIN_OUTPUT_POWER ) power = LR20XX_HF_MIN_OUTPUT_POWER;
        if( power > LR20XX_HF_MAX_OUTPUT_POWER ) power = LR20XX_HF_MAX_OUTPUT_POWER;
        const lr20xx_pa_pwr_cfg_t* pwr_cfg = &ctx->cfg->pa_hf_cfg_table[power - LR20XX_HF_MIN_OUTPUT_POWER];
        output_params->pa_cfg.pa_sel           = LR20XX_RADIO_COMMON_PA_SEL_HF;
        output_params->pa_cfg.pa_lf_mode       = LR20XX_RADIO_COMMON_PA_LF_MODE_FSM;
        output_params->pa_cfg.pa_lf_slices     = 7;
        output_params->pa_cfg.pa_lf_duty_cycle = 0;
        output_params->pa_cfg.pa_hf_duty_cycle = pwr_cfg->pa_duty_cycle;
        output_params->chip_output_half_pwr_in_dbm_configured = pwr_cfg->half_power;
        output_params->chip_output_pwr_in_dbm_expected        = power;
    }
}

/* ---- Front-end calibration ---- */

void ral_lr20xx_bsp_get_front_end_calibration_cfg(
    const void* context,
    lr20xx_radio_common_front_end_calibration_value_t front_end_calibration_structures[3] )
{
    const halo_drv_lr20xx_ctx_t* ctx    = context;
    const uint32_t*              freqs  = ctx->cfg->calibration_freqs;

    if( freqs == NULL )
    {
        /* No calibration points configured: leave the array zeroed. */
        memset( front_end_calibration_structures, 0,
                3 * sizeof( lr20xx_radio_common_front_end_calibration_value_t ) );
        return;
    }

    /* usp ral_lr20xx_bsp.c: three calibration points (870 MHz LF, 920 MHz LF,
     * 2441 MHz HF); the RX path for each frequency comes from get_rx_cfg. */
    for( uint8_t i = 0; i < 3; i++ )
    {
        lr20xx_radio_common_rx_path_t            rx_path    = LR20XX_RADIO_COMMON_RX_PATH_LF;
        lr20xx_radio_common_rx_path_boost_mode_t boost_mode = LR20XX_RADIO_COMMON_RX_PATH_BOOST_MODE_NONE;

        ral_lr20xx_bsp_get_rx_cfg( context, freqs[i], &rx_path, &boost_mode );
        front_end_calibration_structures[i].rx_path            = rx_path;
        front_end_calibration_structures[i].frequency_in_hertz = freqs[i];
    }
}

/* ---- Power consumption (usp examples/radio_hal/ral_lr20xx_bsp.c measured
 * tables, exposed through hal_cfg; NULL table keeps the previous stub
 * behaviour of OK + 0) ---- */

ral_status_t ral_lr20xx_bsp_get_instantaneous_tx_power_consumption(
    const void* context,
    const ral_lr20xx_bsp_tx_cfg_output_params_t* tx_cfg,
    lr20xx_system_reg_mode_t radio_reg_mode,
    uint32_t* pwr_consumption_in_ua )
{
    const halo_drv_lr20xx_ctx_t* ctx = context;
    uint8_t                      index = 0;

    if( tx_cfg->pa_cfg.pa_sel == LR20XX_RADIO_COMMON_PA_SEL_LF )
    {
        if( tx_cfg->chip_output_pwr_in_dbm_expected > LR20XX_LF_MAX_OUTPUT_POWER )
        {
            index = LR20XX_LF_MAX_OUTPUT_POWER - LR20XX_LF_MIN_OUTPUT_POWER;
        }
        else if( tx_cfg->chip_output_pwr_in_dbm_expected < LR20XX_LF_MIN_OUTPUT_POWER )
        {
            index = 0;
        }
        else
        {
            index = tx_cfg->chip_output_pwr_in_dbm_expected - LR20XX_LF_MIN_OUTPUT_POWER;
        }

        const uint32_t* table = ( radio_reg_mode == LR20XX_SYSTEM_REG_MODE_DCDC )
                                    ? ctx->cfg->tx_dbm_to_ua_reg_mode_dcdc_lf_vreg
                                    : ctx->cfg->tx_dbm_to_ua_reg_mode_ldo_lf_vreg;

        if( table == NULL )
        {
            *pwr_consumption_in_ua = 0;
            return RAL_STATUS_OK;
        }
        *pwr_consumption_in_ua = table[index];
    }
    else if( tx_cfg->pa_cfg.pa_sel == LR20XX_RADIO_COMMON_PA_SEL_HF )
    {
        if( radio_reg_mode != LR20XX_SYSTEM_REG_MODE_DCDC )
        {
            return RAL_STATUS_UNSUPPORTED_FEATURE;
        }

        if( tx_cfg->chip_output_pwr_in_dbm_expected > LR20XX_HF_MAX_OUTPUT_POWER )
        {
            index = LR20XX_HF_MAX_OUTPUT_POWER - LR20XX_HF_MIN_OUTPUT_POWER;
        }
        else if( tx_cfg->chip_output_pwr_in_dbm_expected < LR20XX_HF_MIN_OUTPUT_POWER )
        {
            index = 0;
        }
        else
        {
            index = tx_cfg->chip_output_pwr_in_dbm_expected - LR20XX_HF_MIN_OUTPUT_POWER;
        }

        const uint32_t* table = ctx->cfg->tx_dbm_to_ua_reg_mode_dcdc_hf_vreg;

        if( table == NULL )
        {
            *pwr_consumption_in_ua = 0;
            return RAL_STATUS_OK;
        }
        *pwr_consumption_in_ua = table[index];
    }
    else
    {
        return RAL_STATUS_UNKNOWN_VALUE;
    }

    return RAL_STATUS_OK;
}

ral_status_t ral_lr20xx_bsp_get_instantaneous_gfsk_rx_power_consumption(
    const void* context,
    lr20xx_system_reg_mode_t radio_reg_mode,
    bool rx_boosted,
    uint32_t* pwr_consumption_in_ua )
{
    (void) context;

    if( radio_reg_mode == LR20XX_SYSTEM_REG_MODE_DCDC )
    {
        *pwr_consumption_in_ua =
            ( rx_boosted ) ? LR20XX_GFSK_RX_BOOSTED_CONSUMPTION_DCDC : LR20XX_GFSK_RX_CONSUMPTION_DCDC;
    }
    else
    {
        *pwr_consumption_in_ua =
            ( rx_boosted ) ? LR20XX_GFSK_RX_BOOSTED_CONSUMPTION_LDO : LR20XX_GFSK_RX_CONSUMPTION_LDO;
    }

    return RAL_STATUS_OK;
}

ral_status_t ral_lr20xx_bsp_get_instantaneous_lora_rx_power_consumption(
    const void* context,
    const lr20xx_system_reg_mode_t radio_reg_mode,
    const ral_lora_bw_t bw,
    bool rx_boosted,
    uint32_t* pwr_consumption_in_ua )
{
    const halo_drv_lr20xx_ctx_t* ctx = context;

    lr20xx_radio_common_rx_path_t rfi   = LR20XX_RADIO_COMMON_RX_PATH_LF;
    uint8_t                       index = 0;

    if( ( bw <= RAL_LORA_BW_125_KHZ ) || ( bw == RAL_LORA_BW_083_KHZ ) )
    {
        /* BW_083_KHZ (enum value 15) is LR20xx-only and low-band: map it to the
         * 125 kHz LF entry instead of falling through to the >= 1 MHz HF case. */
        rfi   = LR20XX_RADIO_COMMON_RX_PATH_LF;
        index = 0;
    }
    else if( bw == RAL_LORA_BW_250_KHZ )
    {
        rfi   = LR20XX_RADIO_COMMON_RX_PATH_LF;
        index = 1;
    }
    else if( bw == RAL_LORA_BW_400_KHZ )
    {
        rfi   = LR20XX_RADIO_COMMON_RX_PATH_LF;
        index = 2;
    }
    else if( bw == RAL_LORA_BW_500_KHZ )
    {
        rfi   = LR20XX_RADIO_COMMON_RX_PATH_LF;
        index = 3;
    }
    else if( bw == RAL_LORA_BW_800_KHZ )
    {
        /* Select Rx HF path */
        rfi   = LR20XX_RADIO_COMMON_RX_PATH_HF;
        index = 4;
    }
    else if( bw >= RAL_LORA_BW_1000_KHZ )
    {
        /* Select Rx HF path */
        rfi   = LR20XX_RADIO_COMMON_RX_PATH_HF;
        index = 5;
    }

    /* Note: the usp reference selects the boosted table when rx_boosted is
     * false and vice versa — inverted. The boosted tables carry the higher
     * currents, so boost maps to boost here. */
    const uint32_t* table = NULL;

    if( radio_reg_mode == LR20XX_SYSTEM_REG_MODE_DCDC )
    {
        if( rfi == LR20XX_RADIO_COMMON_RX_PATH_LF )
        {
            table = ( rx_boosted ) ? ctx->cfg->rx_bw_to_ua_reg_mode_dcdc_lf_vreg_boosted
                                   : ctx->cfg->rx_bw_to_ua_reg_mode_dcdc_lf_vreg;
        }
        else
        {
            table = ( rx_boosted ) ? ctx->cfg->rx_bw_to_ua_reg_mode_dcdc_hf_vreg_boosted
                                   : ctx->cfg->rx_bw_to_ua_reg_mode_dcdc_hf_vreg;
        }
    }
    else
    {
        if( rfi == LR20XX_RADIO_COMMON_RX_PATH_LF )
        {
            table = ( rx_boosted ) ? ctx->cfg->rx_bw_to_ua_reg_mode_ldo_lf_vreg_boosted
                                   : ctx->cfg->rx_bw_to_ua_reg_mode_ldo_lf_vreg;
        }
        else
        {
            table = ( rx_boosted ) ? ctx->cfg->rx_bw_to_ua_reg_mode_ldo_hf_vreg_boosted
                                   : ctx->cfg->rx_bw_to_ua_reg_mode_ldo_hf_vreg;
        }
    }

    if( table == NULL )
    {
        *pwr_consumption_in_ua = 0;
        return RAL_STATUS_OK;
    }
    *pwr_consumption_in_ua = table[index];

    return RAL_STATUS_OK;
}

/* --- EOF ------------------------------------------------------------------ */
