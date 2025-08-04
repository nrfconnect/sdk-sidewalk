/**
 * @file      lr1110_halo.h
 *
 * @brief     FSK support for HALO platforms
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

#ifndef __LR1110_HALO_H__
#define __LR1110_HALO_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
 * -----------------------------------------------------------------------------
 * --- DEPENDENCIES ------------------------------------------------------------
 */

#include <stdint.h>
#include <stdbool.h>

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC MACROS -----------------------------------------------------------
 */
#define POLYNOMIAL_CRC16                        0x1021
#define POLYNOMIAL_CRC32                        0x04C11DB7
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
 * perform data whitening
 *
 * @param [in] seed             whitening seed
 * @param [in] buffer_in        pointer to the buffer that needs to be updated
 * @param [in] buffer_out       pointer to the updated buffer
 * @param [in] length           length of the buffer to perform data whitnening
 *
 */
void perform_data_whitening( uint16_t seed, const uint8_t* buffer_in, uint8_t* buffer_out, uint16_t length );

/**
 * compute crc16
 *
 *
 * @param [in] context          buffer on which crc16 needed to be calculated
 * @param [in] command          length of the buffer that is passed
 *
 * @retval status     calculated crc16 value
 */
uint16_t compute_crc16( const uint8_t* buffer, uint16_t length );

/**
 * compute crc32
 *
 *
 * @param [in] context          buffer on which crc32 needed to be calculated
 * @param [in] command          length of the buffer that is passed
 *
 * @retval status     calculated crc32 value
 */
uint32_t compute_crc32( const uint8_t* buffer, uint16_t length );

#ifdef __cplusplus
}
#endif

#endif  // __LR1110_HALO_H__

/* --- EOF ------------------------------------------------------------------ */
