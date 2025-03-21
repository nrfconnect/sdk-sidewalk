/**
 * @file      sx126x_timings.c
 *
 * @brief     SX126x timing helper functions implementation
 *
 * Revised BSD License
 * Copyright Semtech Corporation 2020. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Semtech corporation nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL SEMTECH CORPORATION BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * -----------------------------------------------------------------------------
 * --- DEPENDENCIES ------------------------------------------------------------
 */

#include "sx126x_timings.h"

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE MACROS-----------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE CONSTANTS -------------------------------------------------------
 */

#define RX_DONE_IRQ_PROCESSING_TIME_IN_US 54
#define TX_DONE_IRQ_PROCESSING_TIME_IN_US 53

#define LORA_OVERSAMPLING_RATE 32

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE TYPES -----------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE VARIABLES -------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS DECLARATION -------------------------------------------
 */

/**
 * @brief Get the power amplifier ramp time for a given power amplifier ramp time parameter
 *
 * @param [in] ramp_time Power amplifier ramp time parameter
 *
 * @returns Ramp time in microsecond
 */
static uint32_t sx126x_timings_get_pa_ramp_time_in_us( const sx126x_ramp_time_t ramp_time );

/**
 * @brief Get the LoRa clock frequency
 *
 * @param [in] bw LoRa bandwidth
 *
 * @returns LoRa clock frequency in Hz
 */
static uint32_t sx126x_timings_get_lora_clock_freq_in_hz( const sx126x_lora_bw_t bw );

/**
 * @brief Get the LoRa reception input delay
 *
 * @param [in] bw LoRa bandwidth
 *
 * @returns LoRa reception input delay in microsecond
 */
static uint32_t sx126x_timings_get_lora_rx_input_delay_in_us( const sx126x_lora_bw_t bw );

/**
 * @brief Get the LoRa reception base delay
 *
 * @param [in] sf LoRa spreading factor
 *
 * @returns LoRa reception base delay in tick
 */
static uint32_t sx126x_timings_get_lora_rx_base_delay_in_tick( const sx126x_lora_sf_t sf );

/**
 * @brief Get the LoRa reception symbol delay
 *
 * @param [in] sf LoRa spreading factor
 *
 * @returns LoRa reception symbol delay in tick
 */
static uint32_t sx126x_timings_get_lora_rx_symb_delay_in_tick( const sx126x_lora_sf_t sf );

/**
 * @brief Get the LoRa reception data delay
 *
 * @param [in] mod_params Pointer to a structure holding the LoRa modulation parameters used for the computation
 * @param [in] pkt_params Pointer to a structure holding the LoRa packet parameters used for the computation
 *
 * @returns LoRa reception data delay in tick
 */
static uint32_t sx126x_timings_get_lora_rx_data_delay_in_tick( const sx126x_mod_params_lora_t* mod_params,
                                                               const sx126x_pkt_params_lora_t* pkt_params );

/**
 * @brief Get the LoRa physical layer delay
 *
 * @param [in] mod_params Pointer to a structure holding the LoRa modulation parameters used for the computation
 * @param [in] pkt_params Pointer to a structure holding the LoRa packet parameters used for the computation
 *
 * @returns LoRa physical layer delay in microsecond
 */
static uint32_t sx126x_timings_get_lora_rx_phy_delay_in_us( const sx126x_mod_params_lora_t* mod_params,
                                                            const sx126x_pkt_params_lora_t* pkt_params );

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC FUNCTIONS DEFINITION ---------------------------------------------
 */

uint32_t sx126x_timings_get_delay_between_last_bit_sent_and_rx_done_in_us( const sx126x_mod_params_lora_t* mod_params,
                                                                           const sx126x_pkt_params_lora_t* pkt_params )
{
    return sx126x_timings_get_lora_rx_input_delay_in_us( mod_params->bw ) +
           sx126x_timings_get_lora_rx_phy_delay_in_us( mod_params, pkt_params ) + RX_DONE_IRQ_PROCESSING_TIME_IN_US;
}

uint32_t sx126x_timings_get_delay_between_last_bit_sent_and_tx_done_in_us( const sx126x_ramp_time_t ramp_time )
{
    return sx126x_timings_get_pa_ramp_time_in_us( ramp_time ) + TX_DONE_IRQ_PROCESSING_TIME_IN_US;
}

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS DEFINITION --------------------------------------------
 */

static uint32_t sx126x_timings_get_pa_ramp_time_in_us( const sx126x_ramp_time_t ramp_time )
{
    switch( ramp_time )
    {
    case SX126X_RAMP_10_US:
    {
        return 10;
    }
    case SX126X_RAMP_20_US:
    {
        return 20;
    }
    case SX126X_RAMP_40_US:
    {
        return 40;
    }
    case SX126X_RAMP_80_US:
    {
        return 80;
    }
    case SX126X_RAMP_200_US:
    {
        return 200;
    }
    case SX126X_RAMP_800_US:
    {
        return 800;
    }
    case SX126X_RAMP_1700_US:
    {
        return 1700;
    }
    case SX126X_RAMP_3400_US:
    {
        return 3400;
    }
    default:
        return 0;
    }
}

static uint32_t sx126x_timings_get_lora_clock_freq_in_hz( const sx126x_lora_bw_t bw )
{
    switch( bw )
    {
    case SX126X_LORA_BW_500:
    {
        return LORA_OVERSAMPLING_RATE * 500000;
    }
    case SX126X_LORA_BW_250:
    {
        return LORA_OVERSAMPLING_RATE * 250000;
    }
    case SX126X_LORA_BW_125:
    {
        return LORA_OVERSAMPLING_RATE * 125000;
    }
    default:
    {
        return 0;
    }
    }
}

static uint32_t sx126x_timings_get_lora_rx_input_delay_in_us( sx126x_lora_bw_t bw )
{
    switch( bw )
    {
    case SX126X_LORA_BW_500:
    {
        return 16;
    }
    case SX126X_LORA_BW_250:
    {
        return 31;
    }
    case SX126X_LORA_BW_125:
    {
        return 57;
    }
    default:
    {
        return 0;
    }
    }
}

static uint32_t sx126x_timings_get_lora_rx_base_delay_in_tick( const sx126x_lora_sf_t sf )
{
    return ( sf + 1 ) << ( sf + 1 );
}

static uint32_t sx126x_timings_get_lora_rx_symb_delay_in_tick( const sx126x_lora_sf_t sf )
{
    return ( 1 << sf );
}

static uint32_t sx126x_timings_get_lora_rx_data_delay_in_tick( const sx126x_mod_params_lora_t* mod_params,
                                                               const sx126x_pkt_params_lora_t* pkt_params )
{
    const uint32_t fine_sync =
        ( ( mod_params->sf == SX126X_LORA_SF5 ) || ( mod_params->sf == SX126X_LORA_SF6 ) ) ? 1 : 0;
    const uint32_t cr_len  = ( ( mod_params->cr == 7 ) ? ( mod_params->cr + 1 ) : mod_params->cr );
    const uint32_t crc_len = ( ( pkt_params->crc_is_on == true ) ? 1 : 0 );

    if( ( mod_params->cr == SX126X_LORA_CR_4_5 ) || ( mod_params->cr == SX126X_LORA_CR_4_6 ) ||
        ( mod_params->cr == SX126X_LORA_CR_4_7 ) || ( mod_params->cr == SX126X_LORA_CR_4_8 ) )
    {
        return ( 2 * ( pkt_params->pld_len_in_bytes + 2 * crc_len ) - ( mod_params->sf + 2 * fine_sync - 7 ) ) %
                   mod_params->sf * 16 * ( 4 + cr_len ) +
               5;
    }
    else
    {
        uint32_t pld_len = pkt_params->pld_len_in_bytes + 2 * crc_len - ( mod_params->sf + 2 * fine_sync - 7 ) / 2;

        return ( 2 * pld_len * ( 5 + cr_len ) ) + ( 2 * pld_len * cr_len % mod_params->sf ) + 4;
    }
}

static uint32_t sx126x_timings_get_lora_rx_phy_delay_in_us( const sx126x_mod_params_lora_t* mod_params,
                                                            const sx126x_pkt_params_lora_t* pkt_params )
{
    uint32_t rx_delay_in_clock_cycle = sx126x_timings_get_lora_rx_base_delay_in_tick( mod_params->sf ) +
                                       sx126x_timings_get_lora_rx_symb_delay_in_tick( mod_params->sf ) +
                                       sx126x_timings_get_lora_rx_data_delay_in_tick( mod_params, pkt_params );
    uint32_t clock_freq_in_mhz = sx126x_timings_get_lora_clock_freq_in_hz( mod_params->bw ) / 1000000;

    if( clock_freq_in_mhz != 0 )
    {
        // Perform integral ceil()
        return ( rx_delay_in_clock_cycle + clock_freq_in_mhz - 1 ) / clock_freq_in_mhz;
    }
    else
    {
        return 0;
    }
}

/* --- EOF ------------------------------------------------------------------ */
