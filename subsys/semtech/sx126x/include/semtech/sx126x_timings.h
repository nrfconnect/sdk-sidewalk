/**
 * @file      sx126x_timings.h
 *
 * @brief     SX126x timing helper functions definition
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

#ifndef __SX126X_TIMINGS_H__
#define __SX126X_TIMINGS_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
 * -----------------------------------------------------------------------------
 * --- DEPENDENCIES ------------------------------------------------------------
 */

#include <sx126x.h>

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC MACROS -----------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC CONSTANTS --------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC TYPES ------------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC FUNCTIONS PROTOTYPES ---------------------------------------------
 */

/**
 * @brief Get the time between the last bit sent (on Tx side) and the Rx done event (on Rx side)
 *
 * @param [in] mod_params Pointer to a structure holding the LoRa modulation parameters used for the computation
 * @param [in] pkt_params Pointer to a structure holding the LoRa packet parameters used for the computation
 *
 * @returns Delay in microsecond
 */
uint32_t sx126x_timings_get_delay_between_last_bit_sent_and_rx_done_in_us( const sx126x_mod_params_lora_t* mod_params,
                                                                           const sx126x_pkt_params_lora_t* pkt_params );

/**
 * @brief Get the time between the last bit sent and the Tx done event
 *
 * @param [in] ramp_time Power amplifier ramp time
 *
 * @returns Delay in microsecond
 */
uint32_t sx126x_timings_get_delay_between_last_bit_sent_and_tx_done_in_us( const sx126x_ramp_time_t ramp_time );

#ifdef __cplusplus
}
#endif

#endif  // __SX126X_TIMINGS_H__

/* --- EOF ------------------------------------------------------------------ */
