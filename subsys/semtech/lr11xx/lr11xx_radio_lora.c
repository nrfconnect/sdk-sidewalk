#ifndef SID_PAL_RADIO_MUTEX
/*
 * Copyright (c) 2019-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * This file supports radio HAL interface specific to LoRa
 * This code was modified by Semtech
 */

/*
 * -----------------------------------------------------------------------------
 * --- DEPENDENCIES ------------------------------------------------------------
 */

#include <assert.h>

#include "lr11xx_radio.h"
#include "lr11xx_regmem.h"
#include "lr11xx_radio_timings.h"
#include "halo_lr11xx_radio.h"
#include <sid_pal_radio_lora_defs.h>

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE MACROS-----------------------------------------------------------
 */

#define UNUSED( x ) ( void ) ( x )

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE CONSTANTS -------------------------------------------------------
 */

/*
 * This function returns the tx processing delay of LoRa.
 * (refer to https://issues.labcollab.net/browse/HALO-9632)
 * Tx processing delay is defined as the time difference between the time
 * when mac schedules to send tx and the first bit of tx.
 * The measured value of tx processing delay is around 336 us.(refer to the
 * value of "t0" in the JIRA).
 */
#define LR11XX_TX_PROCESS_DELAY_US 336

#define LR11XX_IRQ_PROCESS_TIME_US 74

// Radio Rx process time = 286 us
// HALO-9631: Compensate the rx process time
#define LR11XX_RX_PROCESS_DELAY_US 286

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE TYPES -----------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE VARIABLES -------------------------------------------------------
 */

static const uint32_t radio_lora_symb_time[3][8] = { { 32768, 16384, 8192, 4096, 2048, 1024, 512, 256 },  // 125 KHz
                                                     { 16384, 8192, 4096, 2048, 1024, 512, 256, 128 },    // 250 KHz
                                                     { 8192, 4096, 2048, 1024, 512, 256, 128, 64 } };     // 500 KHz

/*
 * From Semtech: For symbol-based timeout, LR11XX requires an extra-delay to process if there is a LoRa symbol.
 * (refer to https://issues.labcollab.net/browse/HALO-14924)
 * This extra-delay in the LR11XX can be computed as follows: Rx base delay + Rx input delay + IRQ processing time
 * Rx base delay: symbol_time * ((SF + 1) >> 1)+ 1) / 16 * BW / 1000000
 * Rx input delay: 57us in BW125; 31us in BW250; 16us in BW500
 * IRQ processing time: 74us
 */
static const uint32_t radio_lora_rx_base_delay_us[8]  = { 1792, 896, 384, 192, 80, 40, 16, 8 };  // SF12~SF5
static const uint32_t radio_lora_rx_input_delay_us[3] = { 57, 31, 16 };                          // BW125, BW250, BW500

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS DECLARATION -------------------------------------------
 */

static bool lora_low_data_rate_optimize( lr11xx_radio_lora_sf_t sf, lr11xx_radio_lora_bw_t bw );

static void radio_to_lr11xx_lora_modulation_params( lr11xx_radio_mod_params_lora_t*               sx_m,
                                                    const sid_pal_radio_lora_modulation_params_t* rd_m );

static void radio_to_lr11xx_lora_packet_params( lr11xx_radio_pkt_params_lora_t*           sx_p,
                                                const sid_pal_radio_lora_packet_params_t* rd_p );

static int32_t get_payload( halo_drv_semtech_ctx_t* drv_ctx, uint8_t* buffer, uint8_t* size, uint8_t max_size );

static int32_t get_lora_crc_present_in_header( halo_drv_semtech_ctx_t*           drv_ctx,
                                               sid_pal_radio_lora_crc_present_t* is_crc_present );

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC FUNCTIONS DEFINITION ---------------------------------------------
 */

int32_t radio_lora_process_rx_done( halo_drv_semtech_ctx_t* drv_ctx )
{
    int32_t                    err;
    sid_pal_radio_rx_packet_t* radio_rx_packet = drv_ctx->radio_rx_packet;

    do
    {
        if( ( err = get_payload( drv_ctx, radio_rx_packet->rcv_payload, &radio_rx_packet->payload_len,
                                 SID_PAL_RADIO_RX_PAYLOAD_MAX_SIZE ) ) != RADIO_ERROR_NONE )
        {
            break;
        }

        lr11xx_radio_pkt_status_lora_t lora_pkt_status = { 0 };
        if( lr11xx_radio_get_lora_pkt_status( drv_ctx, &lora_pkt_status ) != LR11XX_STATUS_OK )
        {
            err = RADIO_ERROR_IO_ERROR;
            break;
        }

        sid_pal_radio_lora_rx_packet_status_t* lora_rx_packet_status = &radio_rx_packet->lora_rx_packet_status;
        lora_rx_packet_status->rssi        = lora_pkt_status.rssi_pkt_in_dbm - drv_ctx->config->lna_gain;
        lora_rx_packet_status->snr         = lora_pkt_status.snr_pkt_in_db;
        lora_rx_packet_status->signal_rssi = lora_pkt_status.signal_rssi_pkt_in_dbm - drv_ctx->config->lna_gain;
        /*
         * NOTE: By default sx126x radio chip's rssi saturates at -106 dBm (or some cases -107 dBm)
         * After that the signal deterioration is reflected on the snr change.
         * to make the rssi carry more significance to the upper layer consumers, we need to figure out
         * a derived rssi value combining the rssi and snr in a meaningful way.
         *
         * Based on the empirical data, the formula is: <adjusted rssi = raw rssi + snr (when snr is negative)>.
         * This adjusted value is reported and used in all the ring products.
         */
        if( lora_rx_packet_status->snr < 0 )
        {
            lora_rx_packet_status->rssi += lora_rx_packet_status->snr;
        }

        if( ( err = get_lora_crc_present_in_header( drv_ctx, &lora_rx_packet_status->is_crc_present ) ) !=
            RADIO_ERROR_NONE )
        {
            break;
        }

    } while( 0 );

    return err;
}

sid_pal_radio_data_rate_t sid_pal_radio_lora_mod_params_to_data_rate(
    const sid_pal_radio_lora_modulation_params_t* mod_params )
{
    if( SID_PAL_RADIO_LORA_SF7 == mod_params->spreading_factor &&
        SID_PAL_RADIO_LORA_BW_500KHZ == mod_params->bandwidth &&
        SID_PAL_RADIO_LORA_CODING_RATE_4_6 == mod_params->coding_rate )
    {
        return SID_PAL_RADIO_DATA_RATE_22KBPS;
    }
    if( SID_PAL_RADIO_LORA_SF8 == mod_params->spreading_factor &&
        SID_PAL_RADIO_LORA_BW_500KHZ == mod_params->bandwidth &&
        SID_PAL_RADIO_LORA_CODING_RATE_4_5_LI == mod_params->coding_rate )
    {
        return SID_PAL_RADIO_DATA_RATE_12_5KBPS;
    }
    if( SID_PAL_RADIO_LORA_SF11 == mod_params->spreading_factor &&
        SID_PAL_RADIO_LORA_BW_500KHZ == mod_params->bandwidth &&
        ( ( SID_PAL_RADIO_LORA_CODING_RATE_4_5 == mod_params->coding_rate ) ||
          ( SID_PAL_RADIO_LORA_CODING_RATE_4_5_LI == mod_params->coding_rate ) ) )
    {
        return SID_PAL_RADIO_DATA_RATE_2KBPS;
    }

    return SID_PAL_RADIO_DATA_RATE_INVALID;
}

int32_t sid_pal_radio_lora_data_rate_to_mod_params( sid_pal_radio_lora_modulation_params_t* mod_params,
                                                    sid_pal_radio_data_rate_t data_rate, uint8_t li_enable )
{
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
        if( li_enable )
        {
            mod_params->coding_rate = SID_PAL_RADIO_LORA_CODING_RATE_4_5_LI;
        }
        else
        {
            mod_params->coding_rate = SID_PAL_RADIO_LORA_CODING_RATE_4_5;
        }
        break;
    default:
        return RADIO_ERROR_NOT_SUPPORTED;
    }

    return RADIO_ERROR_NONE;
}

int32_t sid_pal_radio_set_lora_sync_word( uint16_t sync_word )
{
    uint8_t syncword;
    halo_drv_semtech_ctx_t* drv = lr11xx_get_drv_ctx();
    if (drv->radio_state == SID_PAL_RADIO_SCAN)
      return RADIO_ERROR_NONE;

    if( sync_word == 0x1424 )
    {
        syncword = 0x12;
    }
    else
    {
        syncword = 0x34;
    }

    if( lr11xx_get_drv_ctx( )->ver.fw >= NOT_FULL_FUNCTIONING_VERSION )
    {
        if( lr11xx_radio_set_lora_sync_word( lr11xx_get_drv_ctx( ), syncword ) != LR11XX_STATUS_OK )
        {
            return RADIO_ERROR_HARDWARE_ERROR;
        }
    }
    else
    {
        if( lr11xx_radio_set_lora_public_network( lr11xx_get_drv_ctx( ), syncword ) != LR11XX_STATUS_OK )
        {
            return RADIO_ERROR_HARDWARE_ERROR;
        }
    }

    return RADIO_ERROR_NONE;
}

int32_t sid_pal_radio_set_lora_symbol_timeout( uint8_t num_of_symbols )
{
    halo_drv_semtech_ctx_t *drv = lr11xx_get_drv_ctx();
    if (drv->radio_state == SID_PAL_RADIO_SCAN)
      return RADIO_ERROR_NONE;

    lr11xx_status_t status = lr11xx_radio_set_lora_sync_timeout( lr11xx_get_drv_ctx( ), num_of_symbols );

    return ( status == LR11XX_STATUS_OK ) ? RADIO_ERROR_NONE : RADIO_ERROR_HARDWARE_ERROR;
}

int32_t sid_pal_radio_set_lora_modulation_params( const sid_pal_radio_lora_modulation_params_t* mod_params )
{
    lr11xx_radio_mod_params_lora_t lora_mod_params;
    halo_drv_semtech_ctx_t* drv = lr11xx_get_drv_ctx( );
    assert( drv );
    if (drv->radio_state == SID_PAL_RADIO_SCAN)
      return RADIO_ERROR_NONE;

    radio_to_lr11xx_lora_modulation_params( &lora_mod_params, mod_params );
    if( lr11xx_radio_set_lora_mod_params( drv, &lora_mod_params ) != LR11XX_STATUS_OK )
    {
        return RADIO_ERROR_HARDWARE_ERROR;
    }
    drv->settings_cache.lora_mod_params = *mod_params;

    return RADIO_ERROR_NONE;
}

int32_t sid_pal_radio_set_lora_packet_params( const sid_pal_radio_lora_packet_params_t* packet_params )
{
    lr11xx_radio_pkt_params_lora_t lora_packet_params;
    halo_drv_semtech_ctx_t* drv = lr11xx_get_drv_ctx( );
    assert( drv );
    if (drv->radio_state == SID_PAL_RADIO_SCAN)
      return RADIO_ERROR_NONE;

    radio_to_lr11xx_lora_packet_params( &lora_packet_params, packet_params );
    if( lr11xx_radio_set_lora_pkt_params( drv, &lora_packet_params ) != LR11XX_STATUS_OK )
    {
        return RADIO_ERROR_HARDWARE_ERROR;
    }
    drv->settings_cache.lora_pkt_params = *packet_params;

    return RADIO_ERROR_NONE;
}

int32_t sid_pal_radio_set_lora_cad_params( const sid_pal_radio_lora_cad_params_t* cad_params )
{
    lr11xx_radio_cad_params_t lora_cad_params;
    halo_drv_semtech_ctx_t* drv = lr11xx_get_drv_ctx();
    if (drv->radio_state == SID_PAL_RADIO_SCAN)
      return RADIO_ERROR_NONE;

    lora_cad_params.cad_symb_nb     = cad_params->cad_symbol_num;
    lora_cad_params.cad_detect_peak = cad_params->cad_detect_peak;
    lora_cad_params.cad_detect_min  = cad_params->cad_detect_min;

    switch( cad_params->cad_exit_mode )
    {
    case SID_PAL_RADIO_CAD_EXIT_MODE_CS_ONLY:
        lora_cad_params.cad_exit_mode = LR11XX_RADIO_CAD_EXIT_MODE_STANDBYRC;
        break;
    case SID_PAL_RADIO_CAD_EXIT_MODE_CS_RX:
        lora_cad_params.cad_exit_mode = LR11XX_RADIO_CAD_EXIT_MODE_RX;
        break;
    case SID_PAL_RADIO_CAD_EXIT_MODE_CS_LBT:
        lora_cad_params.cad_exit_mode = LR11XX_RADIO_CAD_EXIT_MODE_TX;
        break;
    default:
        return RADIO_ERROR_INVALID_PARAMS;
    }

    set_lora_exit_mode( cad_params->cad_exit_mode );
    lora_cad_params.cad_timeout = cad_params->cad_timeout;

    if( lr11xx_radio_set_cad_params( lr11xx_get_drv_ctx( ), &lora_cad_params ) != LR11XX_STATUS_OK )
    {
        return RADIO_ERROR_HARDWARE_ERROR;
    }

    return RADIO_ERROR_NONE;
}

uint32_t sid_pal_radio_lora_cad_duration( uint8_t symbol, const sid_pal_radio_lora_modulation_params_t* mod_params )
{
    UNUSED( symbol );
    lr11xx_radio_mod_params_lora_t lora_mod_params;
    radio_to_lr11xx_lora_modulation_params( &lora_mod_params, mod_params );
    assert( 0 );  // TODO: Not yev available in the driver drop from Semtech
    // return sx126x_get_lora_cad_duration_microsecs(symbol, &lora_mod_params);
    return 0;
}

uint32_t sid_pal_radio_lora_time_on_air( const sid_pal_radio_lora_modulation_params_t* mod_params,
                                         const sid_pal_radio_lora_packet_params_t* packet_params, uint8_t packet_len )
{
    lr11xx_radio_pkt_params_lora_t lora_packet_params;
    lr11xx_radio_mod_params_lora_t lora_mod_params;

    radio_to_lr11xx_lora_modulation_params( &lora_mod_params, mod_params );
    radio_to_lr11xx_lora_packet_params( &lora_packet_params, packet_params );
    lora_packet_params.pld_len_in_bytes = packet_len;

    return lr11xx_radio_get_lora_time_on_air_in_ms( &lora_packet_params, &lora_mod_params );
}

/**
 * @brief Calculates the minimum number of symbols that takes more than delay_micro_sec of air time.
 *
 * In the case of a fractional number of symbols, the return value is rounded up to the next integer.
 * Does not affect the radio state and can be executed without radio ownership.
 * In the case of an error, 0 is returned.
 *
 *  @param[in] mod_params Current modulation parameters that phy uses. If null, zero will be returned.
 *  @param[in] delay_micro_sec Input amount of time that will be translated to number of symbols.
 *  @return number of symbols
 *
 */
uint32_t sid_pal_radio_lora_get_lora_number_of_symbols( const sid_pal_radio_lora_modulation_params_t* mod_params,
                                                        uint32_t                                      delay_micro_sec )
{
    uint32_t numsymbols = 0;
    if( mod_params )
    {
        uint32_t ts = radio_lora_symb_time[mod_params->bandwidth - 4][12 - mod_params->spreading_factor];
        numsymbols  = delay_micro_sec / ts;
        if( delay_micro_sec > ts * numsymbols )
        {
            ++numsymbols;
        }
    }
    return numsymbols;
}

uint32_t sid_pal_radio_get_lora_rx_done_delay( const sid_pal_radio_lora_modulation_params_t* mod_params,
                                               const sid_pal_radio_lora_packet_params_t*     pkt_params )
{
    UNUSED( pkt_params );
    lr11xx_radio_mod_params_lora_t lora_mod_params;

    radio_to_lr11xx_lora_modulation_params( &lora_mod_params, mod_params );
    return lr11xx_radio_timings_get_delay_between_last_bit_sent_and_rx_done_in_us( &lora_mod_params );
}

uint32_t sid_pal_radio_get_lora_tx_process_delay( void )
{
    return LR11XX_TX_PROCESS_DELAY_US;
}

uint32_t sid_pal_radio_get_lora_rx_process_delay( void )
{
    return LR11XX_RX_PROCESS_DELAY_US;
}

/**
 * @brief Calculates the lora symbol timeout in us.
 *
 * In the case of an error, 0 is returned.
 *
 *  @param[in] mod_params Current modulation parameters that phy uses. If null, zero will be returned.
 *  @param[in] number_of_symbol Input number of symbol .
 *  @return lora symbol timeout in us
 *
 */
uint32_t sid_pal_radio_get_lora_symbol_timeout_us( sid_pal_radio_lora_modulation_params_t* mod_params,
                                                   uint8_t                                 number_of_symbol )
{
    uint32_t timeout_us = 0;
    if( mod_params )
    {
        uint32_t symbol_time =
            radio_lora_symb_time[mod_params->bandwidth - 4][12 - mod_params->spreading_factor] * number_of_symbol;

        timeout_us += radio_lora_rx_base_delay_us[12 - mod_params->spreading_factor];
        timeout_us += radio_lora_rx_input_delay_us[mod_params->bandwidth - 4];
        timeout_us += LR11XX_IRQ_PROCESS_TIME_US;
        timeout_us += symbol_time;
    }
    return timeout_us;
}
/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS DEFINITION --------------------------------------------
 */

static bool lora_low_data_rate_optimize( lr11xx_radio_lora_sf_t sf, lr11xx_radio_lora_bw_t bw )
{
    if( ( ( bw == LR11XX_RADIO_LORA_BW_125 ) &&
          ( ( sf == LR11XX_RADIO_LORA_SF11 ) || ( sf == LR11XX_RADIO_LORA_SF12 ) ) ) ||
        ( ( bw == LR11XX_RADIO_LORA_BW_250 ) && ( sf == LR11XX_RADIO_LORA_SF12 ) ) )
    {
        return true;
    }
    return false;
}

static void radio_to_lr11xx_lora_modulation_params( lr11xx_radio_mod_params_lora_t*               sx_m,
                                                    const sid_pal_radio_lora_modulation_params_t* rd_m )
{
    _Static_assert( LR11XX_RADIO_LORA_SF5 == SID_PAL_RADIO_LORA_SF5, "Invalid convertion" );
    _Static_assert( LR11XX_RADIO_LORA_SF6 == SID_PAL_RADIO_LORA_SF6, "Invalid convertion" );
    _Static_assert( LR11XX_RADIO_LORA_SF7 == SID_PAL_RADIO_LORA_SF7, "Invalid convertion" );
    _Static_assert( LR11XX_RADIO_LORA_SF8 == SID_PAL_RADIO_LORA_SF8, "Invalid convertion" );
    _Static_assert( LR11XX_RADIO_LORA_SF9 == SID_PAL_RADIO_LORA_SF9, "Invalid convertion" );
    _Static_assert( LR11XX_RADIO_LORA_SF10 == SID_PAL_RADIO_LORA_SF10, "Invalid convertion" );
    _Static_assert( LR11XX_RADIO_LORA_SF11 == SID_PAL_RADIO_LORA_SF11, "Invalid convertion" );
    _Static_assert( LR11XX_RADIO_LORA_SF12 == SID_PAL_RADIO_LORA_SF12, "Invalid convertion" );
    sx_m->sf = ( lr11xx_radio_lora_sf_t ) rd_m->spreading_factor;

    _Static_assert( LR11XX_RADIO_LORA_BW_10 == SID_PAL_RADIO_LORA_BW_10KHZ, "Invalid convertion" );
    _Static_assert( LR11XX_RADIO_LORA_BW_15 == SID_PAL_RADIO_LORA_BW_15KHZ, "Invalid convertion" );
    _Static_assert( LR11XX_RADIO_LORA_BW_20 == SID_PAL_RADIO_LORA_BW_20KHZ, "Invalid convertion" );
    _Static_assert( LR11XX_RADIO_LORA_BW_31 == SID_PAL_RADIO_LORA_BW_31KHZ, "Invalid convertion" );
    _Static_assert( LR11XX_RADIO_LORA_BW_41 == SID_PAL_RADIO_LORA_BW_41KHZ, "Invalid convertion" );
    _Static_assert( LR11XX_RADIO_LORA_BW_62 == SID_PAL_RADIO_LORA_BW_62KHZ, "Invalid convertion" );
    _Static_assert( LR11XX_RADIO_LORA_BW_125 == SID_PAL_RADIO_LORA_BW_125KHZ, "Invalid convertion" );
    _Static_assert( LR11XX_RADIO_LORA_BW_250 == SID_PAL_RADIO_LORA_BW_250KHZ, "Invalid convertion" );
    _Static_assert( LR11XX_RADIO_LORA_BW_500 == SID_PAL_RADIO_LORA_BW_500KHZ, "Invalid convertion" );
    assert( rd_m->bandwidth != SID_PAL_RADIO_LORA_BW_7KHZ );  // Not supported
    sx_m->bw = ( lr11xx_radio_lora_bw_t ) rd_m->bandwidth;

    _Static_assert( LR11XX_RADIO_LORA_CR_4_5 == SID_PAL_RADIO_LORA_CODING_RATE_4_5, "Invalid convertion" );
    _Static_assert( LR11XX_RADIO_LORA_CR_4_6 == SID_PAL_RADIO_LORA_CODING_RATE_4_6, "Invalid convertion" );
    _Static_assert( LR11XX_RADIO_LORA_CR_4_7 == SID_PAL_RADIO_LORA_CODING_RATE_4_7, "Invalid convertion" );
    _Static_assert( LR11XX_RADIO_LORA_CR_4_8 == SID_PAL_RADIO_LORA_CODING_RATE_4_8, "Invalid convertion" );
    _Static_assert( LR11XX_RADIO_LORA_CR_LI_4_5 == SID_PAL_RADIO_LORA_CODING_RATE_4_5_LI, "Invalid convertion" );
    _Static_assert( LR11XX_RADIO_LORA_CR_LI_4_6 == SID_PAL_RADIO_LORA_CODING_RATE_4_6_LI, "Invalid convertion" );
    _Static_assert( LR11XX_RADIO_LORA_CR_LI_4_8 == SID_PAL_RADIO_LORA_CODING_RATE_4_8_LI, "Invalid convertion" );

    sx_m->cr = ( lr11xx_radio_lora_cr_t ) rd_m->coding_rate;

    sx_m->ldro = lora_low_data_rate_optimize( rd_m->spreading_factor, rd_m->bandwidth );
}

static void radio_to_lr11xx_lora_packet_params( lr11xx_radio_pkt_params_lora_t*           sx_p,
                                                const sid_pal_radio_lora_packet_params_t* rd_p )
{
    sx_p->preamble_len_in_symb = rd_p->preamble_length;

    _Static_assert( LR11XX_RADIO_LORA_PKT_EXPLICIT == SID_PAL_RADIO_LORA_HEADER_TYPE_VARIABLE_LENGTH,
                    "Invalid convertion" );
    _Static_assert( LR11XX_RADIO_LORA_PKT_IMPLICIT == SID_PAL_RADIO_LORA_HEADER_TYPE_FIXED_LENGTH,
                    "Invalid convertion" );
    sx_p->header_type      = ( lr11xx_radio_lora_pkt_len_modes_t ) rd_p->header_type;
    sx_p->pld_len_in_bytes = rd_p->payload_length;

    _Static_assert( LR11XX_RADIO_LORA_CRC_OFF == SID_PAL_RADIO_LORA_CRC_OFF, "Invalid convertion" );
    _Static_assert( LR11XX_RADIO_LORA_CRC_ON == SID_PAL_RADIO_LORA_CRC_ON, "Invalid convertion" );
    sx_p->crc = ( lr11xx_radio_lora_crc_t ) rd_p->crc_mode;

    _Static_assert( LR11XX_RADIO_LORA_IQ_STANDARD == SID_PAL_RADIO_LORA_IQ_NORMAL, "Invalid convertion" );
    _Static_assert( LR11XX_RADIO_LORA_IQ_INVERTED == SID_PAL_RADIO_LORA_IQ_INVERTED, "Invalid convertion" );
    sx_p->iq = rd_p->invert_IQ;
}

static int32_t get_payload( halo_drv_semtech_ctx_t* drv_ctx, uint8_t* buffer, uint8_t* size, uint8_t max_size )
{
    int32_t                         err;
    lr11xx_radio_rx_buffer_status_t rx_buffer_status = { 0 };

    if( lr11xx_radio_get_rx_buffer_status( drv_ctx, &rx_buffer_status ) != LR11XX_STATUS_OK )
    {
        err = RADIO_ERROR_IO_ERROR;
    }
    else if( rx_buffer_status.pld_len_in_bytes > max_size )
    {
        err = RADIO_ERROR_NOMEM;
    }
    else if( lr11xx_regmem_read_buffer8( drv_ctx, buffer, rx_buffer_status.buffer_start_pointer,
                                         rx_buffer_status.pld_len_in_bytes ) != LR11XX_STATUS_OK )
    {
        err = RADIO_ERROR_IO_ERROR;
    }
    else
    {
        *size = rx_buffer_status.pld_len_in_bytes;
        err   = RADIO_ERROR_NONE;
    }
    return err;
}

static int32_t get_lora_crc_present_in_header( halo_drv_semtech_ctx_t*           drv_ctx,
                                               sid_pal_radio_lora_crc_present_t* is_crc_present )
{
    lr11xx_radio_lora_cr_t cr;
    bool                   is_crc_on;
    if( lr11xx_radio_get_lora_rx_info( drv_ctx, &is_crc_on, &cr ) != LR11XX_STATUS_OK )
    {
        *is_crc_present = SID_PAL_RADIO_CRC_PRESENT_INVALID;
        return RADIO_ERROR_GENERIC;
    }

    *is_crc_present = ( is_crc_on ) ? SID_PAL_RADIO_CRC_PRESENT_ON : SID_PAL_RADIO_CRC_PRESENT_OFF;
    return RADIO_ERROR_NONE;
}

#endif /* !SID_PAL_RADIO_MUTEX */
/* --- EOF ------------------------------------------------------------------ */
