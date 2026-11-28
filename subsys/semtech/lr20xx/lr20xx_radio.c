/*
 * Copyright (c) 2026 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * LR20xx Sidewalk PAL for the Semtech LR2021 (LoRa Plus) sub-GHz radio.
 *
 * sid_pal_radio_init() end-to-end: DT-spec GPIO setup → lr20xx_system_reset →
 * PRAM patch load + enable → version verification → standby/reg_mode/lfclk/
 * calibrate. HAL is the adapted usp_zephyr Zephyr-native HAL (spi_dt_spec).
 *
 * The sid_pal_radio_ifc functions are implemented as thin ral_lr20xx
 * adapters; the ones not applicable to the LR2021 return
 * RADIO_ERROR_NOT_SUPPORTED.
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "halo_lr20xx_radio.h"
#include "lr20xx_config.h"
#include "lr20xx_hal.h"
#include "lr20xx_system.h"
#include "lr20xx_patch.h"
#include "lr20xx_pram_load.h"
#include "lr20xx_patch_types.h"
#include "lr20xx_radio_common.h"
#include "ral_lr20xx.h"
#include "ral.h"

#include "sid_pal_radio_ifc.h"
#include "sl_sidewalk_log_app.h"

#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/spi.h>
#include <string.h>
#include <assert.h>

static halo_drv_lr20xx_ctx_t drv_ctx;

halo_drv_lr20xx_ctx_t* lr20xx_get_drv_ctx( void )
{
    return &drv_ctx;
}

void set_radio_lr20xx_device_config( const radio_lr20xx_device_config_t* cfg )
{
    assert( cfg != NULL );
    drv_ctx.config = cfg;
    drv_ctx.cfg    = &cfg->hal_cfg;   /* HAL reads ctx->cfg */
}

/* Forward declarations — defined later (DIO IRQ model). */
static void lr20xx_dio_irq_cb( const struct device* port, struct gpio_callback* cb, uint32_t pins );
static const struct gpio_dt_spec* lr20xx_find_irq_dio_gpio( void );

static int32_t radio_lr20xx_platform_init( void )
{
    const struct lr20xx_hal_context_cfg_t* h = drv_ctx.cfg;

    if( !spi_is_ready_dt( &h->spi ) )
    {
        SL_SID_LOG_APP_ERROR( "lr20xx: SPI not ready" );
        return RADIO_ERROR_IO_ERROR;
    }
    if( !device_is_ready( h->reset.port ) || !device_is_ready( h->busy.port ) )
    {
        SL_SID_LOG_APP_ERROR( "lr20xx: reset/busy gpio not ready" );
        return RADIO_ERROR_IO_ERROR;
    }

    if( gpio_pin_configure_dt( &h->reset, GPIO_OUTPUT_INACTIVE ) != 0 )
    {
        SL_SID_LOG_APP_ERROR( "lr20xx: reset gpio cfg fail" );
        return RADIO_ERROR_IO_ERROR;
    }
    if( gpio_pin_configure_dt( &h->busy, GPIO_INPUT ) != 0 )
    {
        SL_SID_LOG_APP_ERROR( "lr20xx: busy gpio cfg fail" );
        return RADIO_ERROR_IO_ERROR;
    }

    /* Set up the nRF-side GPIO interrupt on the LR2021 IRQ DIO. The chip
     * side (DIO function + irq_mask) is configured by ral_lr20xx_init via the
     * BSP callbacks; here we arm the nRF pin to fire lr20xx_dio_irq_cb on the
     * rising edge. Interrupt is ENABLED later in init after the chip IRQ mask
     * is applied (to avoid spurious fires during PRAM/calibrate). */
    drv_ctx.dio_irq_gpio = lr20xx_find_irq_dio_gpio( );
    if( drv_ctx.dio_irq_gpio != NULL )
    {
        if( !device_is_ready( drv_ctx.dio_irq_gpio->port ) )
        {
            SL_SID_LOG_APP_ERROR( "lr20xx: DIO irq gpio not ready" );
            return RADIO_ERROR_IO_ERROR;
        }
        if( gpio_pin_configure_dt( drv_ctx.dio_irq_gpio, GPIO_INPUT ) != 0 )
        {
            SL_SID_LOG_APP_ERROR( "lr20xx: DIO irq gpio cfg fail" );
            return RADIO_ERROR_IO_ERROR;
        }
        /* The stack calls sid_pal_radio_init twice (BLE+LoRa link setup); only
         * add the callback once — gpio_add_callback returns -EEXIST on re-add. */
        static bool dio_cb_registered = false;
        if( !dio_cb_registered )
        {
            gpio_init_callback( &drv_ctx.dio_cb, lr20xx_dio_irq_cb, BIT( drv_ctx.dio_irq_gpio->pin ) );
            int rc = gpio_add_callback( drv_ctx.dio_irq_gpio->port, &drv_ctx.dio_cb );
            if( rc != 0 && rc != -EEXIST )
            {
                SL_SID_LOG_APP_ERROR( "lr20xx: DIO irq add_callback fail (%d)", rc );
                return RADIO_ERROR_IO_ERROR;
            }
            dio_cb_registered = true;
        }
    }
    else
    {
        SL_SID_LOG_APP_WARNING( "lr20xx: no IRQ DIO in dios_config — radio events won't fire" );
    }

    return RADIO_ERROR_NONE;
}

int32_t sid_pal_radio_init( sid_pal_radio_event_notify_t notify, sid_pal_radio_irq_handler_t dio_irq_handler,
                            sid_pal_radio_rx_packet_t* rx_packet )
{
    int32_t err;

    if( notify == NULL || dio_irq_handler == NULL || rx_packet == NULL )
    {
        return RADIO_ERROR_INVALID_PARAMS;
    }

    drv_ctx.radio_rx_packet    = rx_packet;
    drv_ctx.report_radio_event = notify;
    drv_ctx.irq_handler        = dio_irq_handler;
    drv_ctx.modem              = SID_PAL_RADIO_MODEM_MODE_LORA;
    drv_ctx.radio_state        = SID_PAL_RADIO_UNKNOWN;

    if( ( err = radio_lr20xx_platform_init() ) != RADIO_ERROR_NONE )
    {
        return err;
    }

    /* Hardware reset. */
    if( lr20xx_system_reset( &drv_ctx ) != LR20XX_STATUS_OK )
    {
        SL_SID_LOG_APP_ERROR( "lr20xx: system_reset fail" );
        return RADIO_ERROR_IO_ERROR;
    }
    drv_ctx.sleeping = false;

    /* Read system version (pre-PRAM): LR2021 reports major 0x01 / minor 0x18. */
    if( lr20xx_system_get_version( &drv_ctx, &drv_ctx.ver ) != LR20XX_STATUS_OK )
    {
        SL_SID_LOG_APP_ERROR( "lr20xx: get_version (pre-PRAM) fail" );
        return RADIO_ERROR_IO_ERROR;
    }
    SL_SID_LOG_APP_INFO( "lr20xx: pre-PRAM version major 0x%.2X minor 0x%.2X",
                         drv_ctx.ver.major, drv_ctx.ver.minor );

    /* Standby RC before RAL init. */
    if( lr20xx_system_set_standby_mode( &drv_ctx, LR20XX_SYSTEM_STANDBY_MODE_RC ) != LR20XX_STATUS_OK )
    {
        SL_SID_LOG_APP_ERROR( "lr20xx: set_standby fail" );
        return RADIO_ERROR_IO_ERROR;
    }

    /* ral_lr20xx_init: PRAM load + reg_mode + TCXO + calibrate + lfclk + DIO +
     * front-end calibration. */
    ral_status_t ral_st = ral_lr20xx_init( &drv_ctx );
    if( ral_st != RAL_STATUS_OK )
    {
        SL_SID_LOG_APP_ERROR( "lr20xx: ral_lr20xx_init fail (%d)", (int)ral_st );
        return RADIO_ERROR_IO_ERROR;
    }
    SL_SID_LOG_APP_INFO( "lr20xx: RAL init OK" );

    drv_ctx.ral = (ral_t)RAL_LR20XX_INSTANTIATE( &drv_ctx );

    /* Verify PRAM patch version. */
    lr20xx_patch_version_t pram_ver;
    memset( &pram_ver, 0, sizeof( pram_ver ) );
    if( lr20xx_patch_get_version( &drv_ctx, &pram_ver ) != LR20XX_STATUS_OK || !pram_ver.is_pram_loaded )
    {
        SL_SID_LOG_APP_ERROR( "lr20xx: PRAM not loaded after ral_init" );
        return RADIO_ERROR_HARDWARE_ERROR;
    }
    SL_SID_LOG_APP_INFO( "lr20xx: PRAM patch loaded (type 0x%.2X ver 0x%.2X)",
                         pram_ver.pram_type, pram_ver.pram_version );

    /* Re-read system version after PRAM for sanity. */
    if( lr20xx_system_get_version( &drv_ctx, &drv_ctx.ver ) == LR20XX_STATUS_OK )
    {
        SL_SID_LOG_APP_INFO( "lr20xx: post-PRAM version major 0x%.2X minor 0x%.2X",
                             drv_ctx.ver.major, drv_ctx.ver.minor );
    }

    ( void ) lr20xx_system_clear_reset_status_info( &drv_ctx );
    ( void ) lr20xx_system_clear_errors( &drv_ctx );

    /* Apply the compile-time region. */
    if( drv_ctx.config )
    {
        ( void ) sid_pal_radio_set_region( drv_ctx.config->regional_config.radio_region );
    }

    /* Enable chip IRQs on the IRQ DIO + arm the nRF GPIO interrupt. */
    {
        ral_irq_t pending = 0;
        ( void ) ral_get_and_clear_irq_status( &drv_ctx.ral, &pending );
        const ral_irq_t default_irq = RAL_IRQ_TX_DONE | RAL_IRQ_RX_DONE | RAL_IRQ_RX_TIMEOUT |
                                      RAL_IRQ_RX_CRC_ERROR | RAL_IRQ_CAD_DONE |
                                      RAL_IRQ_ERROR | RAL_IRQ_CMD_ERROR;
        if( ral_set_dio_irq_params( &drv_ctx.ral, default_irq ) != RAL_STATUS_OK )
        {
            SL_SID_LOG_APP_WARNING( "lr20xx: set_dio_irq_params fail" );
        }
        if( drv_ctx.dio_irq_gpio != NULL )
        {
            if( gpio_pin_interrupt_configure_dt( drv_ctx.dio_irq_gpio, GPIO_INT_EDGE_TO_ACTIVE ) != 0 )
            {
                SL_SID_LOG_APP_WARNING( "lr20xx: DIO irq enable fail" );
            }
        }
    }

    drv_ctx.radio_state = SID_PAL_RADIO_STANDBY;
    SL_SID_LOG_APP_INFO( "lr20xx: init OK (LR2021, standby)" );
    return RADIO_ERROR_NONE;
}

/* ===========================================================================
 * ral_* thin adapters, matched to the vendored RAL API (usp v1.2.1):
 * - ral_irq_t (uint32_t), RAL_IRQ_RX_CRC_ERROR, RAL_IRQ_RX_TIMEOUT, RAL_IRQ_CAD_OK
 * - ral_set_tx(radio) no timeout; ral_set_rx(radio, ms); ral_set_tx_cw(radio)
 * - ral_set_sleep(radio, bool); ral_set_standby(radio, RAL_STANDBY_CFG_RC)
 * - ral_set_tx_cfg(radio, int8_t power, uint32_t freq)
 * - ral_get_lora_time_on_air_in_ms(radio, pkt*, mod*) returns uint32_t
 * - LoRa fields: preamble_len_in_symb, pld_len_in_bytes, crc_is_on, invert_iq_is_on
 * - FSK fields: br_in_bps, fdev_in_hz, pulse_shape, preamble_len_in_bits, etc.
 * - sid_pal fields: preamble_length, freq_dev, sync_word_length, crc_mode, etc.
 * =========================================================================== */

/* ral failure logger that captures the calling PAL function. */
static inline int32_t ral_to_radio_err_impl( ral_status_t s, const char* caller )
{
    if( s != RAL_STATUS_OK )
    {
        SL_SID_LOG_APP_ERROR( "ral err %d in %s", ( int ) s, caller );
    }
    return ( s == RAL_STATUS_OK ) ? RADIO_ERROR_NONE : RADIO_ERROR_IO_ERROR;
}
#define ral_to_radio_err( s ) ral_to_radio_err_impl( ( s ), __func__ )

/* PAL entry trace — DEBUG level (only shows with CONFIG_SIDEWALK_LOG_LEVEL=4). */
#define LR20XX_PAL_TRACE( ) SL_SID_LOG_APP_DEBUG( "pal> %s", __func__ )

static int32_t lr20xx_process_lora_rx_done( void );
static int32_t lr20xx_process_fsk_rx_done( void );

/* DIO IRQ ISR. Chip asserts the IRQ DIO (active-high) → this fires in
 * ISR context → drv_ctx.irq_handler (stack bottom-half) signals the stack to
 * run sid_pal_radio_irq_process from thread context. */
static void lr20xx_dio_irq_cb( const struct device* port, struct gpio_callback* cb, uint32_t pins )
{
    ( void ) port; ( void ) cb; ( void ) pins;
    if( drv_ctx.irq_handler )
    {
        drv_ctx.irq_handler( );
    }
}

/* Locate the IRQ-function DIO's nRF gpio in cfg->dios_config[]. */
static const struct gpio_dt_spec* lr20xx_find_irq_dio_gpio( void )
{
    if( drv_ctx.cfg == NULL )
    {
        return NULL;
    }
    for( uint8_t i = 0; i < drv_ctx.cfg->dios_config_num; i++ )
    {
        if( drv_ctx.cfg->dios_config[i].function == LR20XX_SYSTEM_DIO_FUNC_IRQ )
        {
            return &drv_ctx.cfg->dios_config[i].gpio;
        }
    }
    return NULL;
}

/* Re-apply the chip DIO IRQ config before each TX/RX (lr11xx pattern:
 * radio_enable_irq before start_tx/start_rx). Sleep/wakeup can drop the DIO
 * function set in ral_lr20xx_init — and ral_set_dio_irq_params ONLY arms a DIO
 * whose function is already IRQ, silently skipping it otherwise. So we must
 * re-set the DIO function (IRQ) first, then the mask, every TX/RX cycle. */
#define LR20XX_DEFAULT_IRQ_MASK ( RAL_IRQ_TX_DONE | RAL_IRQ_RX_DONE | RAL_IRQ_RX_TIMEOUT | \
                                  RAL_IRQ_RX_CRC_ERROR | RAL_IRQ_CAD_DONE | \
                                  RAL_IRQ_ERROR | RAL_IRQ_CMD_ERROR )
static void lr20xx_apply_dio_irq_mask( void )
{
    if( drv_ctx.cfg != NULL )
    {
        for( uint8_t i = 0; i < drv_ctx.cfg->dios_config_num; i++ )
        {
            const lr20xx_dio_cfg_t* dc = &drv_ctx.cfg->dios_config[i];
            if( dc->function == LR20XX_SYSTEM_DIO_FUNC_IRQ )
            {
                ( void ) lr20xx_system_set_dio_function( &drv_ctx, dc->dio, dc->function, dc->sleep_drive );
            }
        }
    }
    /* Clear any stale pending IRQ so the new TX_DONE/RX_DONE creates a fresh
     * DIO edge — some chips only assert DIO on the IRQ-becomes-pending
     * transition, not for already-pending bits. */
    ral_irq_t pending = 0;
    ( void ) ral_get_and_clear_irq_status( &drv_ctx.ral, &pending );
    ( void ) ral_set_dio_irq_params( &drv_ctx.ral, LR20XX_DEFAULT_IRQ_MASK );
}

/* ===========================================================================
 * LoRa data-rate mapping (US915) + symbol-time math + enum translation.
 *
 * CRITICAL — sid_pal and RAL LoRa enums do NOT share the same numeric space:
 *   SF : SID_PAL_RADIO_LORA_SF5..12 (5..12) == RAL_LORA_SF5..12   -> direct cast OK
 *   CR : SID_PAL_RADIO_LORA_CODING_RATE_* (1..7) == RAL_LORA_CR_* (1..7) -> OK
 *   BW : SID_PAL uses 0x04/0x05/0x06 for 125/250/500 kHz; RAL uses 7/9/11.
 *        A direct cast is WRONG (0x06 -> RAL_LORA_BW_062_KHZ). Must translate.
 *   header_type: SID_PAL VARIABLE_LENGTH(0)==RAL EXPLICIT(0), FIXED(1)==IMPLICIT(1) -> direct cast.
 *
 * Symbol-time table is pure LoRa math (2^SF / BW), chip-independent.
 * RX base/input/IRQ delay constants are estimates borrowed from the LR11xx
 * PAL (same chip family), to be refined with board measurements.
 * =========================================================================== */

/* LoRa symbol time in us, [bw_index][sf_index].
 * bw_index = bandwidth - 4  (125/250/500 kHz -> 0/1/2)
 * sf_index = 12 - spreading_factor (SF12..SF5 -> 0..7).  ts = 2^SF / BW. */
static const uint32_t lora_symb_time_us[3][8] = {
    { 32768, 16384, 8192, 4096, 2048, 1024, 512, 256 }, /* 125 kHz */
    { 16384, 8192, 4096, 2048, 1024, 512, 256, 128 },   /* 250 kHz */
    { 8192, 4096, 2048, 1024, 512, 256, 128, 64 },      /* 500 kHz */
};

/* Per-SF RX base delay (us), SF12..SF5 (LR11xx estimate). */
static const uint32_t lora_rx_base_delay_us[8]  = { 1792, 896, 384, 192, 80, 40, 16, 8 };
/* Per-BW RX input delay (us), 125/250/500 kHz (LR11xx estimate). */
static const uint32_t lora_rx_input_delay_us[3] = { 57, 31, 16 };
#define LR20XX_IRQ_PROCESS_TIME_US 74 /* LR11xx estimate */

static ral_lora_bw_t sid_pal_lora_bw_to_ral( uint8_t bw )
{
    switch( bw )
    {
    case SID_PAL_RADIO_LORA_BW_125KHZ: return RAL_LORA_BW_125_KHZ;
    case SID_PAL_RADIO_LORA_BW_250KHZ: return RAL_LORA_BW_250_KHZ;
    case SID_PAL_RADIO_LORA_BW_500KHZ: return RAL_LORA_BW_500_KHZ;
    case SID_PAL_RADIO_LORA_BW_62KHZ:  return RAL_LORA_BW_062_KHZ;
    case SID_PAL_RADIO_LORA_BW_31KHZ:  return RAL_LORA_BW_031_KHZ;
    case SID_PAL_RADIO_LORA_BW_41KHZ:  return RAL_LORA_BW_041_KHZ;
    case SID_PAL_RADIO_LORA_BW_15KHZ:  return RAL_LORA_BW_015_KHZ;
    case SID_PAL_RADIO_LORA_BW_20KHZ:  return RAL_LORA_BW_020_KHZ;
    case SID_PAL_RADIO_LORA_BW_10KHZ:  return RAL_LORA_BW_010_KHZ;
    case SID_PAL_RADIO_LORA_BW_7KHZ:   return RAL_LORA_BW_007_KHZ;
    default:                           return RAL_LORA_BW_125_KHZ;
    }
}

/* Build ral_lora_mod_params_t from sid_pal enums: translates BW + computes LDRO. */
static ral_lora_mod_params_t sid_pal_to_ral_lora_mod_params( const sid_pal_radio_lora_modulation_params_t* mp )
{
    ral_lora_mod_params_t ral_mp = {
        .sf  = ( ral_lora_sf_t ) mp->spreading_factor,
        .bw  = sid_pal_lora_bw_to_ral( mp->bandwidth ),
        .cr  = ( ral_lora_cr_t ) mp->coding_rate,
        .ldro = 0,
    };
    ral_mp.ldro = ral_compute_lora_ldro( ral_mp.sf, ral_mp.bw );
    return ral_mp;
}

static ral_lora_pkt_params_t sid_pal_to_ral_lora_pkt_params( const sid_pal_radio_lora_packet_params_t* pp,
                                                             uint8_t pld_len )
{
    ral_lora_pkt_params_t ral_pp = {
        .preamble_len_in_symb = pp->preamble_length,
        .header_type   = ( ral_lora_pkt_len_modes_t ) pp->header_type, /* values match: 0=explicit, 1=implicit */
        .pld_len_in_bytes = pld_len,
        .crc_is_on        = ( pp->crc_mode != SID_PAL_RADIO_LORA_CRC_OFF ),
        .invert_iq_is_on  = ( pp->invert_IQ != 0 ),
    };
    return ral_pp;
}

int32_t sid_pal_radio_deinit( void )
{
    LR20XX_PAL_TRACE( );
    return ral_to_radio_err( ral_reset( &drv_ctx.ral ) );
}

sid_pal_radio_irq_mask_t sid_pal_radio_configure_irq_mask( sid_pal_radio_irq_mask_t irq_mask )
{
    drv_ctx.irq_mask_cache = irq_mask;
    return irq_mask;
}

sid_pal_radio_irq_mask_t sid_pal_radio_get_current_config_irq_mask( void )
{
    return drv_ctx.irq_mask_cache;
}

int32_t sid_pal_radio_irq_process( void )
{
    LR20XX_PAL_TRACE( );
    ral_irq_t irq = 0;
    if( ral_get_and_clear_irq_status( &drv_ctx.ral, &irq ) != RAL_STATUS_OK )
        return RADIO_ERROR_IO_ERROR;

    sid_pal_radio_events_t evt = SID_PAL_RADIO_EVENT_UNKNOWN;

    if( irq & RAL_IRQ_TX_DONE )
    {
        evt = SID_PAL_RADIO_EVENT_TX_DONE;
    }
    else if( irq & RAL_IRQ_RX_CRC_ERROR )
    {
        evt = SID_PAL_RADIO_EVENT_RX_ERROR;
    }
    else if( irq & RAL_IRQ_RX_TIMEOUT )
    {
        evt = ( drv_ctx.radio_state == SID_PAL_RADIO_TX ) ? SID_PAL_RADIO_EVENT_TX_TIMEOUT
                                                          : SID_PAL_RADIO_EVENT_RX_TIMEOUT;
    }
    else if( irq & RAL_IRQ_CAD_DONE )
    {
        evt = ( irq & RAL_IRQ_CAD_OK ) ? SID_PAL_RADIO_EVENT_CAD_DONE : SID_PAL_RADIO_EVENT_CAD_TIMEOUT;
    }
    else if( irq & RAL_IRQ_RX_DONE )
    {
        int32_t rx_err = ( drv_ctx.modem == SID_PAL_RADIO_MODEM_MODE_LORA )
                             ? lr20xx_process_lora_rx_done( )
                             : lr20xx_process_fsk_rx_done( );
        if( rx_err == RADIO_ERROR_NONE )
        {
            memset( &drv_ctx.radio_rx_packet->fsk_rx_packet_status, 0,
                    sizeof( sid_pal_radio_fsk_rx_packet_status_t ) );
            evt = SID_PAL_RADIO_EVENT_RX_DONE;
        }
        else
        {
            evt = SID_PAL_RADIO_EVENT_RX_ERROR;
        }
    }

    if( evt != SID_PAL_RADIO_EVENT_UNKNOWN && drv_ctx.report_radio_event )
    {
        drv_ctx.report_radio_event( evt );
    }
    return RADIO_ERROR_NONE;
}

/* LoRa RX_DONE: read FIFO + packet status (RSSI/SNR). */
/* lr20xx_process_lora_rx_done / lr20xx_process_fsk_rx_done are forward-declared
 * above and DEFINED after the FSK CRC/whitening helpers (which
 * lr20xx_process_fsk_rx_done depends on). */

int32_t sid_pal_radio_set_frequency( uint32_t freq )
{
    LR20XX_PAL_TRACE( );
    drv_ctx.radio_freq_hz = freq;
    return ral_to_radio_err( ral_set_rf_freq( &drv_ctx.ral, freq ) );
}

int32_t sid_pal_radio_set_tx_power( int8_t power )
{
    LR20XX_PAL_TRACE( );
    return ral_to_radio_err( ral_set_tx_cfg( &drv_ctx.ral, power, drv_ctx.radio_freq_hz ) );
}

int32_t sid_pal_radio_get_max_tx_power( sid_pal_radio_data_rate_t data_rate, int8_t* tx_power )
{
    LR20XX_PAL_TRACE( );
    if( data_rate <= SID_PAL_RADIO_DATA_RATE_INVALID || data_rate > SID_PAL_RADIO_DATA_RATE_MAX_NUM )
    {
        return RADIO_ERROR_INVALID_PARAMS;
    }
    if( tx_power == NULL )
    {
        return RADIO_ERROR_INVALID_PARAMS;
    }
    *tx_power = drv_ctx.regional_radio_param.max_tx_power[data_rate - 1];
    return RADIO_ERROR_NONE;
}

int32_t sid_pal_radio_set_region( sid_pal_radio_region_code_t region )
{
    if( region <= SID_PAL_RADIO_RC_NONE || region >= SID_PAL_RADIO_RC_MAX )
    {
        return RADIO_ERROR_INVALID_PARAMS;
    }
    if( drv_ctx.config == NULL )
    {
        return RADIO_ERROR_INVALID_PARAMS;
    }
    for( uint8_t i = 0; i < drv_ctx.config->regional_config.reg_param_table_size; i++ )
    {
        if( region == drv_ctx.config->regional_config.reg_param_table[i].param_region )
        {
            drv_ctx.regional_radio_param = drv_ctx.config->regional_config.reg_param_table[i];
            return RADIO_ERROR_NONE;
        }
    }
    return RADIO_ERROR_NOT_SUPPORTED;
}

int32_t sid_pal_radio_sleep( uint32_t sleep_us )
{
    LR20XX_PAL_TRACE( );
    ( void ) sleep_us;

    ral_irq_t pending = 0;
    ( void ) ral_get_and_clear_irq_status( &drv_ctx.ral, &pending );

    int32_t ret = ral_to_radio_err( ral_set_sleep( &drv_ctx.ral, true ) );
    drv_ctx.radio_state = SID_PAL_RADIO_SLEEP;
    drv_ctx.sleeping = true;
    return ret;
}

int32_t sid_pal_radio_standby( void )
{
    LR20XX_PAL_TRACE( );
    int32_t ret = RADIO_ERROR_NONE;
    if( drv_ctx.sleeping )
    {
        ret = ral_to_radio_err( ral_wakeup( &drv_ctx.ral ) );
        drv_ctx.sleeping = false;
        if( ret != RADIO_ERROR_NONE )
        {
            return ret;
        }
        ral_irq_t pending = 0;
        ( void ) ral_get_and_clear_irq_status( &drv_ctx.ral, &pending );
    }
    ret = ral_to_radio_err( ral_set_standby( &drv_ctx.ral, RAL_STANDBY_CFG_RC ) );
    drv_ctx.radio_state = SID_PAL_RADIO_STANDBY;
    return ret;
}

int32_t sid_pal_set_radio_busy( void ) { drv_ctx.radio_state = SID_PAL_RADIO_BUSY; return RADIO_ERROR_NONE; }

int32_t sid_pal_radio_start_carrier_sense( const sid_pal_radio_fsk_cad_params_t* cad_params,
                                           sid_pal_radio_cad_param_exit_mode_t exit_mode )
{ LR20XX_PAL_TRACE( ); (void) cad_params; (void) exit_mode;
  return ral_to_radio_err( ral_set_lora_cad( &drv_ctx.ral ) ); }

int32_t sid_pal_radio_start_rx( uint32_t timeout )
{
    LR20XX_PAL_TRACE( );
    lr20xx_apply_dio_irq_mask( );
    int32_t ret = ral_to_radio_err( ral_set_rx( &drv_ctx.ral, timeout / 1000 ) );
    drv_ctx.radio_state = SID_PAL_RADIO_RX;
    return ret;
}

int32_t sid_pal_radio_start_continuous_rx( void )
{
    LR20XX_PAL_TRACE( );
    lr20xx_apply_dio_irq_mask( );
    int32_t ret = ral_to_radio_err( ral_set_rx( &drv_ctx.ral, 0 ) );
    drv_ctx.radio_state = SID_PAL_RADIO_RX;
    return ret;
}

int32_t sid_pal_radio_set_rx_duty_cycle( uint32_t rx_time, uint32_t sleep_time )
{
    return ral_to_radio_err( ral_set_rx_duty_cycle( &drv_ctx.ral, rx_time, sleep_time ) );
}

int32_t sid_pal_radio_set_tx_continuous_wave( uint32_t freq, int8_t power )
{ (void) freq; (void) power;
  return ral_to_radio_err( ral_set_tx_cw( &drv_ctx.ral ) ); }

int32_t sid_pal_radio_set_tx_continuous_preamble( uint32_t freq, int8_t power )
{ (void) freq; (void) power;
  return ral_to_radio_err( ral_set_tx_infinite_preamble( &drv_ctx.ral ) ); }

int32_t sid_pal_radio_set_tx_payload( const uint8_t* buffer, uint8_t size )
{
    LR20XX_PAL_TRACE( );
    return ral_to_radio_err( ral_set_pkt_payload( &drv_ctx.ral, buffer, size ) );
}

int32_t sid_pal_radio_start_tx( uint32_t timeout )
{ LR20XX_PAL_TRACE( ); (void) timeout;
  lr20xx_apply_dio_irq_mask( );
  int32_t ret = ral_to_radio_err( ral_set_tx( &drv_ctx.ral ) );
  drv_ctx.radio_state = SID_PAL_RADIO_TX;
  return ret; }

uint8_t sid_pal_radio_get_status( void )
{
    LR20XX_PAL_TRACE( );
    return drv_ctx.radio_state;
}
sid_pal_radio_modem_mode_t sid_pal_radio_get_modem_mode( void ) { LR20XX_PAL_TRACE( ); return drv_ctx.modem; }

int32_t sid_pal_radio_set_modem_mode( sid_pal_radio_modem_mode_t mode )
{
    LR20XX_PAL_TRACE( );
    drv_ctx.modem = mode;
    ral_pkt_type_t pt = ( mode == SID_PAL_RADIO_MODEM_MODE_LORA ) ? RAL_PKT_TYPE_LORA : RAL_PKT_TYPE_GFSK;
    return ral_to_radio_err( ral_set_pkt_type( &drv_ctx.ral, pt ) );
}

int32_t sid_pal_radio_is_channel_free( uint32_t freq, int16_t threshold, uint32_t delay_us, bool* is_channel_free )
{
    LR20XX_PAL_TRACE( );
    if( !is_channel_free ) return RADIO_ERROR_INVALID_PARAMS;
    ral_set_rf_freq( &drv_ctx.ral, freq );
    ral_set_rx( &drv_ctx.ral, delay_us / 1000 );
    int16_t rssi_val = 0;
    ral_get_rssi_inst( &drv_ctx.ral, &rssi_val );
    *is_channel_free = ( rssi_val < threshold );
    ral_set_standby( &drv_ctx.ral, RAL_STANDBY_CFG_RC );
    return RADIO_ERROR_NONE;
}

int32_t sid_pal_radio_get_chan_noise( uint32_t freq, int16_t* noise )
{
    if( !noise ) return RADIO_ERROR_INVALID_PARAMS;
    ral_set_rf_freq( &drv_ctx.ral, freq );
    ral_set_rx( &drv_ctx.ral, 1 );
    ral_get_rssi_inst( &drv_ctx.ral, noise );
    ral_set_standby( &drv_ctx.ral, RAL_STANDBY_CFG_RC );
    return RADIO_ERROR_NONE;
}

int16_t sid_pal_radio_rssi( void )
{ int16_t r = 0; ral_get_rssi_inst( &drv_ctx.ral, &r ); return r; }

int32_t sid_pal_radio_random( uint32_t* random )
{ return ral_to_radio_err( ral_get_random_numbers( &drv_ctx.ral, random, 1 ) ); }

int16_t sid_pal_radio_get_ant_dbi( void )
{ LR20XX_PAL_TRACE( ); return drv_ctx.regional_radio_param.ant_dbi; }

int32_t sid_pal_radio_get_cca_level_adjust( sid_pal_radio_data_rate_t data_rate, int8_t* adj_level )
{
    LR20XX_PAL_TRACE( );
    if( data_rate <= SID_PAL_RADIO_DATA_RATE_INVALID || data_rate > SID_PAL_RADIO_DATA_RATE_MAX_NUM )
    {
        return RADIO_ERROR_INVALID_PARAMS;
    }
    if( adj_level == NULL )
    {
        return RADIO_ERROR_INVALID_PARAMS;
    }
    *adj_level = drv_ctx.regional_radio_param.cca_level_adjust[data_rate - 1];
    return RADIO_ERROR_NONE;
}

int32_t sid_pal_radio_get_radio_state_transition_delays( sid_pal_radio_state_transition_timings_t* state_delay )
{ LR20XX_PAL_TRACE( );
  if( state_delay && drv_ctx.config ) { *state_delay = drv_ctx.config->state_timings; return RADIO_ERROR_NONE; }
  return RADIO_ERROR_INVALID_PARAMS; }

int32_t sid_pal_radio_is_cad_exit_mode( sid_pal_radio_cad_param_exit_mode_t mode )
{
    LR20XX_PAL_TRACE( );
    /* Mirror lr11xx: support the CS_* and ED_* exit modes the stack may probe
     * during init. Returning NOT_SUPPORTED here maps to SID_ERROR_NOSUPPORT(-6),
     * so accept all defined CS/ED modes. */
    if( mode == SID_PAL_RADIO_CAD_EXIT_MODE_CS_ONLY ||
        mode == SID_PAL_RADIO_CAD_EXIT_MODE_CS_RX ||
        mode == SID_PAL_RADIO_CAD_EXIT_MODE_CS_LBT ||
        mode == SID_PAL_RADIO_CAD_EXIT_MODE_CS_LBT_RX ||
        mode == SID_PAL_RADIO_CAD_EXIT_MODE_ED_ONLY ||
        mode == SID_PAL_RADIO_CAD_EXIT_MODE_ED_RX ||
        mode == SID_PAL_RADIO_CAD_EXIT_MODE_ED_LBT )
    {
        return RADIO_ERROR_NONE;
    }
    return RADIO_ERROR_INVALID_PARAMS;
}

/* ---- LoRa ---- */

int32_t sid_pal_radio_set_lora_symbol_timeout( uint8_t num_of_symbols )
{ LR20XX_PAL_TRACE( );
  return ral_to_radio_err( ral_set_lora_symb_nb_timeout( &drv_ctx.ral, num_of_symbols ) ); }

int32_t sid_pal_radio_set_lora_sync_word( uint16_t sync_word )
{
    LR20XX_PAL_TRACE( );
    /* Sidewalk passes 0x1424 (private) / 0x3444 (public); lr20xx takes a single
     * syncword byte (0x12 private / 0x34 public), same as lr11xx. */
    uint8_t byte = ( sync_word == 0x1424 ) ? 0x12 : 0x34;
    return ral_to_radio_err( ral_set_lora_sync_word( &drv_ctx.ral, byte ) );
}

int32_t sid_pal_radio_set_lora_modulation_params( const sid_pal_radio_lora_modulation_params_t* mod_params )
{
    LR20XX_PAL_TRACE( );
    if( !mod_params ) return RADIO_ERROR_INVALID_PARAMS;
    drv_ctx.settings_cache.lora_mod_params = *mod_params;
    ral_lora_mod_params_t ral_mp = sid_pal_to_ral_lora_mod_params( mod_params );
    return ral_to_radio_err( ral_set_lora_mod_params( &drv_ctx.ral, &ral_mp ) );
}

int32_t sid_pal_radio_set_lora_packet_params( const sid_pal_radio_lora_packet_params_t* packet_params )
{
    LR20XX_PAL_TRACE( );
    if( !packet_params ) return RADIO_ERROR_INVALID_PARAMS;
    drv_ctx.settings_cache.lora_pkt_params = *packet_params;
    ral_lora_pkt_params_t ral_pp = sid_pal_to_ral_lora_pkt_params( packet_params, packet_params->payload_length );
    return ral_to_radio_err( ral_set_lora_pkt_params( &drv_ctx.ral, &ral_pp ) );
}

int32_t sid_pal_radio_set_lora_cad_params( const sid_pal_radio_lora_cad_params_t* cad_params )
{ (void) cad_params; return RADIO_ERROR_NONE; }

int32_t sid_pal_radio_lora_start_cad( void )
{ return ral_to_radio_err( ral_set_lora_cad( &drv_ctx.ral ) ); }

sid_pal_radio_data_rate_t sid_pal_radio_lora_mod_params_to_data_rate( const sid_pal_radio_lora_modulation_params_t* mod_params )
{
    /* US915 Sidewalk LoRa data rates (all BW500): see lora_data_rate_to_mod_params. */
    if( mod_params == NULL )
    {
        return SID_PAL_RADIO_DATA_RATE_INVALID;
    }
    if( mod_params->spreading_factor == SID_PAL_RADIO_LORA_SF7 &&
        mod_params->bandwidth == SID_PAL_RADIO_LORA_BW_500KHZ &&
        mod_params->coding_rate == SID_PAL_RADIO_LORA_CODING_RATE_4_6 )
    {
        return SID_PAL_RADIO_DATA_RATE_22KBPS;
    }
    if( mod_params->spreading_factor == SID_PAL_RADIO_LORA_SF8 &&
        mod_params->bandwidth == SID_PAL_RADIO_LORA_BW_500KHZ &&
        mod_params->coding_rate == SID_PAL_RADIO_LORA_CODING_RATE_4_5_LI )
    {
        return SID_PAL_RADIO_DATA_RATE_12_5KBPS;
    }
    if( mod_params->spreading_factor == SID_PAL_RADIO_LORA_SF11 &&
        mod_params->bandwidth == SID_PAL_RADIO_LORA_BW_500KHZ &&
        ( mod_params->coding_rate == SID_PAL_RADIO_LORA_CODING_RATE_4_5 ||
          mod_params->coding_rate == SID_PAL_RADIO_LORA_CODING_RATE_4_5_LI ) )
    {
        return SID_PAL_RADIO_DATA_RATE_2KBPS;
    }
    return SID_PAL_RADIO_DATA_RATE_INVALID;
}

int32_t sid_pal_radio_lora_data_rate_to_mod_params( sid_pal_radio_lora_modulation_params_t* mod_params,
                                          sid_pal_radio_data_rate_t data_rate, uint8_t li_enable )
{
    /* US915 Sidewalk LoRa data rates (all on BW500):
     *   22kbps  : SF7  / CR4/6
     *   12.5kbps: SF8  / CR4/5-LI
     *   2kbps   : SF11 / CR4/5 (LI variant if li_enable)
     */
    if( mod_params == NULL )
    {
        return RADIO_ERROR_INVALID_PARAMS;
    }
    switch( data_rate )
    {
    case SID_PAL_RADIO_DATA_RATE_22KBPS:
        mod_params->spreading_factor = SID_PAL_RADIO_LORA_SF7;
        mod_params->bandwidth        = SID_PAL_RADIO_LORA_BW_500KHZ;
        mod_params->coding_rate      = SID_PAL_RADIO_LORA_CODING_RATE_4_6;
        break;
    case SID_PAL_RADIO_DATA_RATE_12_5KBPS:
        mod_params->spreading_factor = SID_PAL_RADIO_LORA_SF8;
        mod_params->bandwidth        = SID_PAL_RADIO_LORA_BW_500KHZ;
        mod_params->coding_rate      = SID_PAL_RADIO_LORA_CODING_RATE_4_5_LI;
        break;
    case SID_PAL_RADIO_DATA_RATE_2KBPS:
        mod_params->spreading_factor = SID_PAL_RADIO_LORA_SF11;
        mod_params->bandwidth        = SID_PAL_RADIO_LORA_BW_500KHZ;
        mod_params->coding_rate      = li_enable ? SID_PAL_RADIO_LORA_CODING_RATE_4_5_LI
                                                 : SID_PAL_RADIO_LORA_CODING_RATE_4_5;
        break;
    default:
        return RADIO_ERROR_NOT_SUPPORTED;
    }
    return RADIO_ERROR_NONE;
}

uint32_t sid_pal_radio_lora_time_on_air( const sid_pal_radio_lora_modulation_params_t* mod_params,
                               const sid_pal_radio_lora_packet_params_t* packet_params, uint8_t packet_len )
{
    LR20XX_PAL_TRACE( );
    if( mod_params == NULL || packet_params == NULL )
    {
        return 0;
    }
    ral_lora_mod_params_t ral_mp = sid_pal_to_ral_lora_mod_params( mod_params );
    ral_lora_pkt_params_t ral_pp = sid_pal_to_ral_lora_pkt_params( packet_params, packet_len );
    uint32_t toa = ral_get_lora_time_on_air_in_ms( &drv_ctx.ral, &ral_pp, &ral_mp );
    return toa;
}

uint32_t sid_pal_radio_lora_cad_duration( uint8_t symbol, const sid_pal_radio_lora_modulation_params_t* mod_params )
{
    /* CAD duration ~= n_symbols * symbol_time + fixed overhead. Estimate:
     * symbol-based; fixed overhead not calibrated (returns symbol air-time only). */
    ( void ) symbol;
    if( mod_params == NULL || mod_params->bandwidth < SID_PAL_RADIO_LORA_BW_125KHZ ||
        mod_params->bandwidth > SID_PAL_RADIO_LORA_BW_500KHZ ||
        mod_params->spreading_factor < SID_PAL_RADIO_LORA_SF5 ||
        mod_params->spreading_factor > SID_PAL_RADIO_LORA_SF12 )
    {
        return 0;
    }
    uint32_t ts = lora_symb_time_us[mod_params->bandwidth - SID_PAL_RADIO_LORA_BW_125KHZ]
                                       [SID_PAL_RADIO_LORA_SF12 - mod_params->spreading_factor];
    return ts;
}

uint32_t sid_pal_radio_lora_get_lora_number_of_symbols( const sid_pal_radio_lora_modulation_params_t* mod_params,
                                               uint32_t delay_micro_sec )
{
    LR20XX_PAL_TRACE( );
    /* Minimum number of symbols whose air time exceeds delay_micro_sec (round up). */
    if( mod_params == NULL || mod_params->bandwidth < SID_PAL_RADIO_LORA_BW_125KHZ ||
        mod_params->bandwidth > SID_PAL_RADIO_LORA_BW_500KHZ ||
        mod_params->spreading_factor < SID_PAL_RADIO_LORA_SF5 ||
        mod_params->spreading_factor > SID_PAL_RADIO_LORA_SF12 )
    {
        return 0;
    }
    uint32_t ts = lora_symb_time_us[mod_params->bandwidth - SID_PAL_RADIO_LORA_BW_125KHZ]
                                       [SID_PAL_RADIO_LORA_SF12 - mod_params->spreading_factor];
    uint32_t numsymbols = delay_micro_sec / ts;
    if( delay_micro_sec > ts * numsymbols )
    {
        ++numsymbols;
    }
    return numsymbols;
}

uint32_t sid_pal_radio_get_lora_rx_done_delay( const sid_pal_radio_lora_modulation_params_t* mod_params,
                                            const sid_pal_radio_lora_packet_params_t* pkt_params )
{ LR20XX_PAL_TRACE( ); (void) mod_params; (void) pkt_params;
  return drv_ctx.config ? drv_ctx.config->state_timings.rx_delay_us : 0; }

uint32_t sid_pal_radio_get_lora_tx_process_delay( void )
{ LR20XX_PAL_TRACE( );
  return drv_ctx.config ? drv_ctx.config->state_timings.tx_delay_us : 0; }

uint32_t sid_pal_radio_get_lora_rx_process_delay( void )
{ LR20XX_PAL_TRACE( );
  return drv_ctx.config ? drv_ctx.config->state_timings.rx_delay_us : 0; }

uint32_t sid_pal_radio_get_lora_symbol_timeout_us( sid_pal_radio_lora_modulation_params_t* mod_params, uint8_t number_of_symbol )
{
    LR20XX_PAL_TRACE( );
    /* symbol_timeout = RX base delay + RX input delay + IRQ processing + n_symbols * symbol_time.
     * Delay constants are estimates (LR11xx values, same chip family). */
    if( mod_params == NULL || mod_params->bandwidth < SID_PAL_RADIO_LORA_BW_125KHZ ||
        mod_params->bandwidth > SID_PAL_RADIO_LORA_BW_500KHZ ||
        mod_params->spreading_factor < SID_PAL_RADIO_LORA_SF5 ||
        mod_params->spreading_factor > SID_PAL_RADIO_LORA_SF12 )
    {
        return 0;
    }
    uint32_t bw_idx = mod_params->bandwidth - SID_PAL_RADIO_LORA_BW_125KHZ;
    uint32_t sf_idx = SID_PAL_RADIO_LORA_SF12 - mod_params->spreading_factor;
    uint32_t symbol_time = lora_symb_time_us[bw_idx][sf_idx] * number_of_symbol;

    uint32_t timeout_us = lora_rx_base_delay_us[sf_idx];
    timeout_us += lora_rx_input_delay_us[bw_idx];
    timeout_us += LR20XX_IRQ_PROCESS_TIME_US;
    timeout_us += symbol_time;
    return timeout_us;
}

/* ===========================================================================
 * FSK framing layer — Sidewalk FSK-over-GFSK PHR
 * construction + software whitening (seed 0x01FF) + 3-byte sync word + CRC.
 * Chip-independent (operates on the pkt_cfg struct only, no radio HW); ported
 * verbatim from lr11xx_radio_fsk.c / lr11xx_halo.c. prepare_fsk_for_{tx,rx}
 * are referenced by libsidewalk_sdk.a even on a LoRa-only link — returning
 * NOT_SUPPORTED here maps to SID_ERROR_NOSUPPORT(-6) and aborts sid_init.
 * =========================================================================== */

#define POLYNOMIAL_CRC16 0x1021
#define POLYNOMIAL_CRC32 0x04C11DB7

#define LR20XX_FSK_PHY_HEADER_LENGTH      2
#define LR20XX_FSK_WHITENING_SEED         0x01FF
#define LR20XX_FSK_MAX_PAYLOAD_LENGTH     255
#define LR20XX_FSK_SYNC_WORD_LENGTH_IN_RX 3
#define LR20XX_FSK_MICRO_SECS_PER_SYMBOL  250
#define MAX_PAYLOAD_LENGTH_WITH_FCS_TYPE_0 251
#define MAX_PAYLOAD_LENGTH_WITH_FCS_TYPE_1 253

#define LR20XX_US_IN_SEC  ( 1000000UL )
#define LR20XX_US_TO_SYMBOLS( time_in_us, bit_rate ) \
    ( ( uint32_t ) ( ( ( uint64_t ) ( time_in_us ) * ( uint64_t ) ( bit_rate ) ) / LR20XX_US_IN_SEC ) )

#define RADIO_FSK_BR_50KBPS    50000
#define RADIO_FSK_BR_150KBPS   150000
#define RADIO_FSK_BR_250KBPS   250000
#define RADIO_FSK_FDEV_25KHZ   25000
#define RADIO_FSK_FDEV_37_5KHZ 37500
#define RADIO_FSK_FDEV_62_5KHZ 62500

static void fsk_perform_data_whitening( uint16_t seed, const uint8_t* buffer_in, uint8_t* buffer_out, uint16_t length )
{
    uint16_t lfsr = seed;
    for( uint16_t index = 0; index < length; index++ )
    {
        uint16_t xor_out = 0;
        uint8_t  ret     = 0;
        uint8_t  result  = 0;

        xor_out = ( ( lfsr >> 5 ) & 0x0F ) ^ ( lfsr & 0x0F );
        lfsr    = ( lfsr >> 4 ) | ( xor_out << 5 );
        ret |= ( lfsr >> 5 ) & 0x0F;

        xor_out = ( ( lfsr >> 5 ) & 0x0F ) ^ ( lfsr & 0x0F );
        lfsr    = ( lfsr >> 4 ) | ( xor_out << 5 );
        ret |= ( ( lfsr >> 1 ) & 0xf0 );

        result |= ( ret & 0x80 ) >> 7;
        result |= ( ret & 0x40 ) >> 5;
        result |= ( ret & 0x20 ) >> 3;
        result |= ( ret & 0x10 ) >> 1;
        result |= ( ret & 0x08 ) << 1;
        result |= ( ret & 0x04 ) << 3;
        result |= ( ret & 0x02 ) << 5;
        result |= ( ret & 0x01 ) << 7;

        buffer_out[index] = buffer_in[index] ^ result;
    }
}

static uint16_t fsk_compute_crc16( const uint8_t* buffer, uint16_t length )
{
    if( buffer == NULL || length == 0 )
    {
        return 0;
    }
    uint16_t crc16 = 0x0000;
    for( uint16_t i = 0; i < length; i++ )
    {
        crc16 ^= ( ( uint16_t ) buffer[i] << 8 );
        for( uint8_t b = 0; b < 8; b++ )
        {
            crc16 = ( crc16 & 0x8000 ) ? ( crc16 << 1 ) ^ POLYNOMIAL_CRC16 : ( crc16 << 1 );
        }
    }
    return crc16;
}

static uint32_t fsk_compute_crc32( const uint8_t* buffer, uint16_t length )
{
    if( buffer == NULL || length == 0 )
    {
        return 0;
    }
    uint8_t  temp_buffer[sizeof( uint32_t )] = { 0 };
    uint32_t crc32          = 0xFFFFFFFF;
    const uint8_t* buf_local;
    uint16_t len_local;

    if( length < sizeof( uint32_t ) )
    {
        memcpy( temp_buffer, buffer, length );
        len_local = sizeof( uint32_t );
        buf_local = temp_buffer;
    }
    else
    {
        len_local = length;
        buf_local = buffer;
    }
    for( uint16_t i = 0; i < len_local; i++ )
    {
        crc32 ^= ( i < length ) ? ( ( uint32_t ) buf_local[i] << 24 ) : 0x00000000;
        for( uint8_t b = 0; b < 8; b++ )
        {
            crc32 = ( crc32 & 0x80000000 ) ? ( crc32 << 1 ) ^ POLYNOMIAL_CRC32 : ( crc32 << 1 );
        }
    }
    return ~crc32;
}

/* ---- RX-done processing (depends on the FSK helpers above) ---- */

/* LoRa RX_DONE: read FIFO + packet status (RSSI/SNR). */
static int32_t lr20xx_process_lora_rx_done( void )
{
    uint16_t size = 0;
    if( ral_get_pkt_size( &drv_ctx.ral, &size ) != RAL_STATUS_OK )
        return RADIO_ERROR_IO_ERROR;
    if( size > SID_PAL_RADIO_RX_PAYLOAD_MAX_SIZE )
        size = SID_PAL_RADIO_RX_PAYLOAD_MAX_SIZE;
    if( ral_get_data_rx_buffer( &drv_ctx.ral, drv_ctx.radio_rx_packet->rcv_payload, size ) != RAL_STATUS_OK )
        return RADIO_ERROR_IO_ERROR;
    drv_ctx.radio_rx_packet->payload_len = ( uint8_t ) size;

    ral_lora_rx_pkt_status_t st = { 0 };
    if( ral_get_lora_rx_pkt_status( &drv_ctx.ral, &st ) != RAL_STATUS_OK )
        return RADIO_ERROR_IO_ERROR;
    sid_pal_radio_lora_rx_packet_status_t* s = &drv_ctx.radio_rx_packet->lora_rx_packet_status;
    s->rssi        = st.rssi_pkt_in_dbm;
    s->snr         = ( int8_t ) st.snr_pkt_in_db;
    s->signal_rssi = ( int8_t ) st.signal_rssi_pkt_in_dbm;
    /* lr11xx saturation compensation: raw rssi saturates ~-106 dBm; fold neg SNR in. */
    if( s->snr < 0 )
    {
        s->rssi += s->snr;
    }
    s->is_crc_present = ( drv_ctx.settings_cache.lora_pkt_params.crc_mode != SID_PAL_RADIO_LORA_CRC_OFF )
                            ? SID_PAL_RADIO_CRC_PRESENT_ON
                            : SID_PAL_RADIO_CRC_PRESENT_OFF;
    return RADIO_ERROR_NONE;
}

/* FSK RX_DONE: read FIFO, parse PHR, de-whiten, verify CRC, strip PHR+CRC.
 * NOTE: Sidewalk FSK uses FIX_LEN radio config with a variable-length payload
 * signalled in the PHR. lr11xx handles this via a sync_word_detected buffer-
 * pointer workaround (chip-specific regmem). LR20xx reads the FIFO on RX_DONE;
 * the variable-length handling here assumes the FIFO contains PHR+payload+CRC.
 * Needs validation with FSK traffic. */
static int32_t lr20xx_process_fsk_rx_done( void )
{
    uint16_t size = 0;
    if( ral_get_pkt_size( &drv_ctx.ral, &size ) != RAL_STATUS_OK )
        return RADIO_ERROR_IO_ERROR;
    if( size > SID_PAL_RADIO_RX_PAYLOAD_MAX_SIZE )
        size = SID_PAL_RADIO_RX_PAYLOAD_MAX_SIZE;
    if( ral_get_data_rx_buffer( &drv_ctx.ral, drv_ctx.radio_rx_packet->rcv_payload, size ) != RAL_STATUS_OK )
        return RADIO_ERROR_IO_ERROR;
    if( size < LR20XX_FSK_PHY_HEADER_LENGTH )
        return RADIO_ERROR_GENERIC;

    uint8_t* buf      = drv_ctx.radio_rx_packet->rcv_payload;
    uint8_t  fcs_type = buf[0] >> 4;
    bool     whitening = ( buf[0] & ( 1 << 3 ) ) ? true : false;
    uint8_t  psdu_len = buf[1]; /* payload + CRC bytes (per Sidewalk PHR) */
    uint8_t* payload  = buf + LR20XX_FSK_PHY_HEADER_LENGTH;

    if( psdu_len == 0 || psdu_len > ( size - LR20XX_FSK_PHY_HEADER_LENGTH ) )
        return RADIO_ERROR_GENERIC;

    if( whitening )
    {
        fsk_perform_data_whitening( LR20XX_FSK_WHITENING_SEED, payload, payload, psdu_len );
    }

    uint8_t crc_length = 0;
    if( fcs_type == RADIO_FSK_FCS_TYPE_0 )
    {
        crc_length = sizeof( uint32_t );
        uint32_t crc = fsk_compute_crc32( payload, psdu_len - crc_length );
        for( uint8_t i = 0; i < crc_length; i++ )
        {
            if( payload[psdu_len - crc_length + i] != ( uint8_t ) ( crc >> ( 8 * ( crc_length - i - 1 ) ) ) )
                return RADIO_ERROR_GENERIC;
        }
    }
    else if( fcs_type == RADIO_FSK_FCS_TYPE_1 )
    {
        crc_length = sizeof( uint16_t );
        uint16_t crc = fsk_compute_crc16( payload, psdu_len - crc_length );
        for( uint8_t i = 0; i < crc_length; i++ )
        {
            if( payload[psdu_len - crc_length + i] != ( uint8_t ) ( crc >> ( 8 * ( crc_length - i - 1 ) ) ) )
                return RADIO_ERROR_GENERIC;
        }
    }
    else
    {
        return RADIO_ERROR_NOT_SUPPORTED;
    }

    /* Strip PHR + CRC: move payload to front of rcv_payload. */
    drv_ctx.radio_rx_packet->payload_len = psdu_len - crc_length;
    memmove( buf, payload, drv_ctx.radio_rx_packet->payload_len );

    ral_gfsk_rx_pkt_status_t st = { 0 };
    if( ral_get_gfsk_rx_pkt_status( &drv_ctx.ral, &st ) == RAL_STATUS_OK )
    {
        drv_ctx.radio_rx_packet->fsk_rx_packet_status.rssi_sync = ( int8_t ) st.rssi_sync_in_dbm;
        drv_ctx.radio_rx_packet->fsk_rx_packet_status.rssi_avg  = ( int8_t ) st.rssi_avg_in_dbm;
    }
    return RADIO_ERROR_NONE;
}

/* ---- FSK ---- */

/* sid_pal ↔ RAL GFSK enum translations. header_type / address_filtering /
 * dc_free match numerically (direct cast); crc_type / preamble_detector /
 * pulse_shape do NOT (sid_pal reuses LR11xx numbering, RAL uses its own). */
static ral_gfsk_crc_type_t sid_pal_fsk_crc_to_ral( uint8_t crc )
{
    switch( crc )
    {
    case SID_PAL_RADIO_FSK_CRC_OFF:        return RAL_GFSK_CRC_OFF;
    case SID_PAL_RADIO_FSK_CRC_1_BYTES:    return RAL_GFSK_CRC_1_BYTE;
    case SID_PAL_RADIO_FSK_CRC_2_BYTES:    return RAL_GFSK_CRC_2_BYTES;
    case SID_PAL_RADIO_FSK_CRC_1_BYTES_INV: return RAL_GFSK_CRC_1_BYTE_INV;
    case SID_PAL_RADIO_FSK_CRC_2_BYTES_INV: return RAL_GFSK_CRC_2_BYTES_INV;
    default:                               return RAL_GFSK_CRC_OFF; /* IBM/CCIT not supported */
    }
}

static ral_gfsk_preamble_detector_t sid_pal_fsk_preamble_detector_to_ral( uint8_t d )
{
    switch( d )
    {
    case SID_PAL_RADIO_FSK_PREAMBLE_DETECTOR_OFF:      return RAL_GFSK_PREAMBLE_DETECTOR_OFF;
    case SID_PAL_RADIO_FSK_PREAMBLE_DETECTOR_08_BITS:  return RAL_GFSK_PREAMBLE_DETECTOR_MIN_8BITS;
    case SID_PAL_RADIO_FSK_PREAMBLE_DETECTOR_16_BITS:  return RAL_GFSK_PREAMBLE_DETECTOR_MIN_16BITS;
    case SID_PAL_RADIO_FSK_PREAMBLE_DETECTOR_24_BITS:  return RAL_GFSK_PREAMBLE_DETECTOR_MIN_24BITS;
    case SID_PAL_RADIO_FSK_PREAMBLE_DETECTOR_32_BITS:  return RAL_GFSK_PREAMBLE_DETECTOR_MIN_32BITS;
    default:                                           return RAL_GFSK_PREAMBLE_DETECTOR_MIN_8BITS;
    }
}

static ral_gfsk_pulse_shape_t sid_pal_fsk_pulse_shape_to_ral( uint8_t s )
{
    switch( s )
    {
    case SID_PAL_RADIO_FSK_MOD_SHAPING_OFF:    return RAL_GFSK_PULSE_SHAPE_OFF;
    case SID_PAL_RADIO_FSK_MOD_SHAPING_G_BT_03: return RAL_GFSK_PULSE_SHAPE_BT_03;
    case SID_PAL_RADIO_FSK_MOD_SHAPING_G_BT_05: return RAL_GFSK_PULSE_SHAPE_BT_05;
    case SID_PAL_RADIO_FSK_MOD_SHAPING_G_BT_07: return RAL_GFSK_PULSE_SHAPE_BT_07;
    case SID_PAL_RADIO_FSK_MOD_SHAPING_G_BT_1:  return RAL_GFSK_PULSE_SHAPE_BT_1;
    default:                                   return RAL_GFSK_PULSE_SHAPE_BT_05;
    }
}

int32_t sid_pal_radio_set_fsk_sync_word( const uint8_t* sync_word, uint8_t sync_word_length )
{
    LR20XX_PAL_TRACE( );
    return ral_to_radio_err( ral_set_gfsk_sync_word( &drv_ctx.ral, sync_word, sync_word_length ) );
}

int32_t sid_pal_radio_set_fsk_whitening_seed( uint16_t seed )
{
    LR20XX_PAL_TRACE( );
    return ral_to_radio_err( ral_set_gfsk_whitening_seed( &drv_ctx.ral, seed ) );
}

int32_t sid_pal_radio_set_fsk_modulation_params( const sid_pal_radio_fsk_modulation_params_t* mod_params )
{
    LR20XX_PAL_TRACE( );
    if( !mod_params ) return RADIO_ERROR_INVALID_PARAMS;
    drv_ctx.settings_cache.fsk_mod_params = *mod_params;
    ral_gfsk_mod_params_t ral_mp = {
        .br_in_bps   = mod_params->bit_rate,
        .fdev_in_hz  = mod_params->freq_dev,
        .bw_dsb_in_hz = 0,  /* lr20xx derives RX BW from br/fdev; 0 = auto */
        .pulse_shape = sid_pal_fsk_pulse_shape_to_ral( mod_params->mod_shaping ),
    };
    return ral_to_radio_err( ral_set_gfsk_mod_params( &drv_ctx.ral, &ral_mp ) );
}

int32_t sid_pal_radio_set_fsk_packet_params( const sid_pal_radio_fsk_packet_params_t* packet_params )
{
    LR20XX_PAL_TRACE( );
    if( !packet_params ) return RADIO_ERROR_INVALID_PARAMS;
    drv_ctx.settings_cache.fsk_pkt_params = *packet_params;
    /* preamble_length is in bytes → bits, (len-1)<<3 like lr11xx (forces preamble
     * polarity; first byte handled by detector). */
    uint32_t preamble_bits = ( packet_params->preamble_length > 1 )
                             ? ( ( uint32_t ) ( packet_params->preamble_length - 1 ) << 3 )
                             : 0;
    ral_gfsk_pkt_params_t ral_pp = {
        .preamble_len_in_bits    = preamble_bits,
        .preamble_detector       = sid_pal_fsk_preamble_detector_to_ral( packet_params->preamble_min_detect ),
        .sync_word_len_in_bits   = ( uint32_t ) packet_params->sync_word_length * 8,
        .address_filtering       = ( ral_gfsk_address_filtering_t ) packet_params->addr_comp, /* 0/1/2 match */
        .header_type             = ( ral_gfsk_pkt_len_modes_t ) packet_params->header_type,   /* 0=fix,1=var match */
        .pld_len_in_bytes        = packet_params->payload_length,
        .crc_type                = sid_pal_fsk_crc_to_ral( packet_params->crc_type ),
        .dc_free                 = ( ral_gfsk_dc_free_t ) packet_params->radio_whitening_mode, /* 0/1 match */
    };
    return ral_to_radio_err( ral_set_gfsk_pkt_params( &drv_ctx.ral, &ral_pp ) );
}

sid_pal_radio_data_rate_t sid_pal_radio_fsk_mod_params_to_data_rate( const sid_pal_radio_fsk_modulation_params_t* mp )
{
    if( mp == NULL )
    {
        return SID_PAL_RADIO_DATA_RATE_INVALID;
    }
    if( mp->bit_rate == RADIO_FSK_BR_50KBPS && mp->freq_dev == RADIO_FSK_FDEV_25KHZ &&
        mp->bandwidth == SID_PAL_RADIO_FSK_BW_156200 )
    {
        return SID_PAL_RADIO_DATA_RATE_50KBPS;
    }
    if( mp->bit_rate == RADIO_FSK_BR_150KBPS && mp->freq_dev == RADIO_FSK_FDEV_37_5KHZ &&
        mp->bandwidth == SID_PAL_RADIO_FSK_BW_312000 )
    {
        return SID_PAL_RADIO_DATA_RATE_150KBPS;
    }
    if( mp->bit_rate == RADIO_FSK_BR_250KBPS && mp->freq_dev == RADIO_FSK_FDEV_62_5KHZ &&
        mp->bandwidth == SID_PAL_RADIO_FSK_BW_467000 )
    {
        return SID_PAL_RADIO_DATA_RATE_250KBPS;
    }
    return SID_PAL_RADIO_DATA_RATE_INVALID;
}

int32_t sid_pal_radio_fsk_data_rate_to_mod_params( sid_pal_radio_fsk_modulation_params_t* mod_params,
                                          sid_pal_radio_data_rate_t data_rate )
{
    if( mod_params == NULL )
    {
        return RADIO_ERROR_INVALID_PARAMS;
    }
    switch( data_rate )
    {
    case SID_PAL_RADIO_DATA_RATE_50KBPS:
        mod_params->bit_rate    = RADIO_FSK_BR_50KBPS;
        mod_params->freq_dev    = RADIO_FSK_FDEV_25KHZ;
        mod_params->bandwidth   = SID_PAL_RADIO_FSK_BW_156200;
        mod_params->mod_shaping = SID_PAL_RADIO_FSK_MOD_SHAPING_G_BT_1;
        break;
    case SID_PAL_RADIO_DATA_RATE_150KBPS:
        mod_params->bit_rate    = RADIO_FSK_BR_150KBPS;
        mod_params->freq_dev    = RADIO_FSK_FDEV_37_5KHZ;
        mod_params->bandwidth   = SID_PAL_RADIO_FSK_BW_312000;
        mod_params->mod_shaping = SID_PAL_RADIO_FSK_MOD_SHAPING_G_BT_05;
        break;
    case SID_PAL_RADIO_DATA_RATE_250KBPS:
        mod_params->bit_rate    = RADIO_FSK_BR_250KBPS;
        mod_params->freq_dev    = RADIO_FSK_FDEV_62_5KHZ;
        mod_params->bandwidth   = SID_PAL_RADIO_FSK_BW_467000;
        mod_params->mod_shaping = SID_PAL_RADIO_FSK_MOD_SHAPING_G_BT_05;
        break;
    default:
        return RADIO_ERROR_INVALID_PARAMS;
    }
    return RADIO_ERROR_NONE;
}

uint32_t sid_pal_radio_fsk_time_on_air( const sid_pal_radio_fsk_modulation_params_t* mod_params,
                              const sid_pal_radio_fsk_packet_params_t* packet_params, uint8_t packet_len )
{
    LR20XX_PAL_TRACE( );
    ral_gfsk_mod_params_t ral_mp = {
        .br_in_bps = mod_params->bit_rate,
        .fdev_in_hz = mod_params->freq_dev,
        .bw_dsb_in_hz = 0,
        .pulse_shape = RAL_GFSK_PULSE_SHAPE_BT_05,
    };
    ral_gfsk_pkt_params_t ral_pp = {
        .preamble_len_in_bits = packet_params->preamble_length,
        .sync_word_len_in_bits = packet_params->sync_word_length * 8,
        .pld_len_in_bytes = packet_len,
        .crc_type = ( packet_params->crc_type != 0 ) ? RAL_GFSK_CRC_2_BYTES_INV : RAL_GFSK_CRC_OFF,
    };
    return ral_get_gfsk_time_on_air_in_ms( &drv_ctx.ral, &ral_pp, &ral_mp );
}

uint32_t sid_pal_radio_fsk_get_fsk_number_of_symbols( const sid_pal_radio_fsk_modulation_params_t* mod_params,
                                             uint32_t delay_micro_secs )
{
    LR20XX_PAL_TRACE( );
    if( mod_params == NULL || mod_params->bit_rate == 0 )
    {
        return 0;
    }
    return LR20XX_US_TO_SYMBOLS( delay_micro_secs, mod_params->bit_rate );
}

uint32_t sid_pal_radio_get_fsk_tx_process_delay( void )
{ LR20XX_PAL_TRACE( );
  return drv_ctx.config ? drv_ctx.config->state_timings.tx_delay_us : 0; }

uint32_t sid_pal_radio_get_fsk_rx_process_delay( void )
{ LR20XX_PAL_TRACE( );
  return drv_ctx.config ? drv_ctx.config->state_timings.rx_delay_us : 0; }

int32_t sid_pal_radio_prepare_fsk_for_rx( sid_pal_radio_fsk_pkt_cfg_t* rx_pkt_cfg )
{
    LR20XX_PAL_TRACE( );
    if( rx_pkt_cfg == NULL || rx_pkt_cfg->phy_hdr == NULL ||
        rx_pkt_cfg->packet_params == NULL || rx_pkt_cfg->sync_word == NULL )
    {
        return RADIO_ERROR_INVALID_PARAMS;
    }

    sid_pal_radio_fsk_packet_params_t* f_pp = rx_pkt_cfg->packet_params;
    uint8_t*                           sw   = rx_pkt_cfg->sync_word;

    f_pp->preamble_min_detect  = SID_PAL_RADIO_FSK_PREAMBLE_DETECTOR_16_BITS;
    f_pp->sync_word_length     = LR20XX_FSK_SYNC_WORD_LENGTH_IN_RX;
    f_pp->addr_comp            = SID_PAL_RADIO_FSK_ADDRESSCOMP_FILT_OFF;
    f_pp->header_type          = SID_PAL_RADIO_FSK_RADIO_PACKET_FIXED_LENGTH;
    f_pp->payload_length       = LR20XX_FSK_MAX_PAYLOAD_LENGTH;
    f_pp->crc_type             = SID_PAL_RADIO_FSK_CRC_OFF;
    f_pp->radio_whitening_mode = SID_PAL_RADIO_FSK_DC_FREE_OFF;

    /* 3-byte sync word (length > preamble min detect of 16 bits); 0x55 forces
     * preamble polarity. Same as lr11xx. */
    sw[0] = 0x55;
    sw[1] = rx_pkt_cfg->phy_hdr->is_fec_enabled ? 0x6F : 0x90;
    sw[2] = 0x4E;

    return RADIO_ERROR_NONE;
}

int32_t sid_pal_radio_prepare_fsk_for_tx( sid_pal_radio_fsk_pkt_cfg_t* tx_pkt_cfg )
{
    LR20XX_PAL_TRACE( );
    if( tx_pkt_cfg == NULL || tx_pkt_cfg->phy_hdr == NULL ||
        tx_pkt_cfg->packet_params == NULL || tx_pkt_cfg->sync_word == NULL ||
        tx_pkt_cfg->payload == NULL )
    {
        return RADIO_ERROR_INVALID_PARAMS;
    }

    sid_pal_radio_fsk_packet_params_t* f_pp = tx_pkt_cfg->packet_params;
    if( f_pp->payload_length == 0 || f_pp->preamble_length == 0 )
    {
        return RADIO_ERROR_INVALID_PARAMS;
    }

    sid_pal_radio_fsk_phy_hdr_t* phr = tx_pkt_cfg->phy_hdr;
    if( ( phr->fcs_type == RADIO_FSK_FCS_TYPE_0 && f_pp->payload_length > MAX_PAYLOAD_LENGTH_WITH_FCS_TYPE_0 ) ||
        ( phr->fcs_type == RADIO_FSK_FCS_TYPE_1 && f_pp->payload_length > MAX_PAYLOAD_LENGTH_WITH_FCS_TYPE_1 ) )
    {
        return RADIO_ERROR_INVALID_PARAMS;
    }

    uint8_t  psdu_length = f_pp->payload_length;
    uint8_t* tx_buffer   = tx_pkt_cfg->payload;

    if( phr->fcs_type == RADIO_FSK_FCS_TYPE_0 )
    {
        uint32_t crc = fsk_compute_crc32( tx_pkt_cfg->payload, f_pp->payload_length );
        tx_buffer[LR20XX_FSK_PHY_HEADER_LENGTH + psdu_length++] = ( uint8_t ) ( crc >> 24 );
        tx_buffer[LR20XX_FSK_PHY_HEADER_LENGTH + psdu_length++] = ( uint8_t ) ( crc >> 16 );
        tx_buffer[LR20XX_FSK_PHY_HEADER_LENGTH + psdu_length++] = ( uint8_t ) ( crc >> 8 );
        tx_buffer[LR20XX_FSK_PHY_HEADER_LENGTH + psdu_length++] = ( uint8_t ) ( crc >> 0 );
    }
    else if( phr->fcs_type == RADIO_FSK_FCS_TYPE_1 )
    {
        uint16_t crc = fsk_compute_crc16( tx_pkt_cfg->payload, f_pp->payload_length );
        tx_buffer[LR20XX_FSK_PHY_HEADER_LENGTH + psdu_length++] = ( uint8_t ) ( crc >> 8 );
        tx_buffer[LR20XX_FSK_PHY_HEADER_LENGTH + psdu_length++] = ( uint8_t ) ( crc >> 0 );
    }
    else
    {
        return RADIO_ERROR_NOT_SUPPORTED;
    }

    uint8_t* sync_word = tx_pkt_cfg->sync_word;
    /* Shift payload right by PHR length, then build the 2-byte PHR at front. */
    memmove( tx_buffer + LR20XX_FSK_PHY_HEADER_LENGTH, tx_buffer, f_pp->payload_length );
    tx_buffer[0] = ( uint8_t ) ( phr->fcs_type << 4 );
    tx_buffer[0] += ( ( phr->is_data_whitening_enabled == true ) ? 1 : 0 ) << 3;
    tx_buffer[1] = psdu_length;

    if( phr->is_data_whitening_enabled == true )
    {
        fsk_perform_data_whitening( LR20XX_FSK_WHITENING_SEED,
                                    tx_buffer + LR20XX_FSK_PHY_HEADER_LENGTH,
                                    tx_buffer + LR20XX_FSK_PHY_HEADER_LENGTH, psdu_length );
    }

    /* 3-byte sync word. */
    sync_word[0] = 0x55;
    sync_word[1] = ( phr->is_fec_enabled == true ) ? 0x6F : 0x90;
    sync_word[2] = 0x4E;

    f_pp->sync_word_length     = LR20XX_FSK_SYNC_WORD_LENGTH_IN_RX;
    f_pp->addr_comp            = SID_PAL_RADIO_FSK_ADDRESSCOMP_FILT_OFF;
    f_pp->header_type          = SID_PAL_RADIO_FSK_RADIO_PACKET_FIXED_LENGTH;
    f_pp->payload_length       = LR20XX_FSK_PHY_HEADER_LENGTH + psdu_length;
    f_pp->crc_type             = SID_PAL_RADIO_FSK_CRC_OFF;
    f_pp->radio_whitening_mode = SID_PAL_RADIO_FSK_DC_FREE_OFF;

    return RADIO_ERROR_NONE;
}

int32_t sid_pal_radio_set_fsk_crc_polynomial( uint16_t crc_polynomial, uint16_t crc_seed )
{
    return ral_to_radio_err( ral_set_gfsk_crc_params( &drv_ctx.ral, crc_seed, crc_polynomial ) );
}

/* --- EOF ------------------------------------------------------------------ */
