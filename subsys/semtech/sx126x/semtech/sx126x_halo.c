/*!
 * @file      sx126x_halo.c
 *
 * @brief     SX126x-based HALO implementation
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
 * ARE DISCLAIMED. IN NO EVENT SHALL SEMTECH S.A. BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <string.h>
#include "sx126x_halo.h"

void perform_data_whitening( uint16_t seed, const uint8_t* buffer_in, uint8_t* buffer_out, uint16_t length )
{
    uint16_t lfsr    = seed;
    uint16_t xor_out = 0;
    uint8_t  ret     = 0;
    uint8_t  result  = 0;

    for( uint16_t index = 0; index < length; index++ )
    {
        xor_out = 0;
        ret     = 0;
        result  = 0;

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

uint16_t compute_crc16( const uint8_t* buffer, uint16_t length )
{
    if (!buffer || !length) {
        return 0;
    }

    uint16_t crc16 = 0x0000;

    for( uint16_t index_buffer = 0; index_buffer < length; index_buffer++ )
    {
        crc16 ^= ( ( uint16_t ) buffer[index_buffer] << 8 );

        for( uint8_t i = 0; i < 8; i++ )
        {
            crc16 = ( crc16 & 0x8000 ) ? ( crc16 << 1 ) ^ POLYNOMIAL_CRC16 : ( crc16 << 1 );
        }
    }

    return crc16;
}

uint32_t compute_crc32( const uint8_t* buffer, uint16_t length )
{
    if (!buffer || !length) {
        return 0;
    }

    uint8_t temp_buffer[sizeof(uint32_t)] = {0};
    uint32_t crc32                        = 0xFFFFFFFF;
    const uint8_t *buffer_local;
    uint16_t length_local;

    if (length < sizeof(uint32_t)) {
        memcpy(temp_buffer, buffer, length);
        length_local = sizeof(uint32_t);
        buffer_local = temp_buffer;
    } else {
        length_local = length;
        buffer_local = buffer;
    }

    for( uint16_t index_buffer = 0; index_buffer < length_local; index_buffer++ )
    {
        crc32 ^= ( index_buffer < length ) ? ( ( uint32_t ) buffer_local[index_buffer] << 24 ) : 0x00000000;

        for( uint8_t i = 0; i < 8; i++ )
        {
            crc32 = ( crc32 & 0x80000000 ) ? ( crc32 << 1 ) ^ POLYNOMIAL_CRC32 : ( crc32 << 1 );
        }
    }

    return ~crc32;
}
