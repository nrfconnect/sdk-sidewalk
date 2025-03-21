/**
 * @file      sx126x.c
 *
 * @brief     SX126x radio driver implementation
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

#include <string.h>  // memcpy
#include "sx126x.h"
#include "sx126x_hal.h"
#include "sx126x_regs.h"

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE MACROS-----------------------------------------------------------
 */

#define SX126X_FREQ_IN_HZ_TO_PLL_STEP( freq, steps )                                                      \
    do                                                                                                    \
    {                                                                                                     \
        uint32_t steps_int;                                                                               \
        uint32_t steps_frac;                                                                              \
        steps_int  = freq / SX126X_PLL_STEP_RSH14;                                                        \
        steps_frac = freq - ( steps_int * SX126X_PLL_STEP_RSH14 );                                        \
        steps      = ( steps_int << RSH14 ) +                                                             \
                ( ( ( steps_frac << RSH14 ) + ( SX126X_PLL_STEP_RSH14 >> 1 ) ) / SX126X_PLL_STEP_RSH14 ); \
    } while( 0 )

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE CONSTANTS -------------------------------------------------------
 */

/**
 * @brief Internal frequency of the radio
 */
#define SX126X_XTAL_FREQ 32000000UL

/**
 * @brief PLL step - right shifted of 14 bits
 */
#define RSH14 ( 14 )
#define SX126X_PLL_STEP_RSH14 ( SX126X_XTAL_FREQ >> ( 25 - RSH14 ) )

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE TYPES -----------------------------------------------------------
 */

typedef struct
{
    uint32_t bw;
    uint8_t  param;
} gfsk_bw_t;

gfsk_bw_t gfsk_bw[] = {
    { 4800, SX126X_GFSK_BW_4800 },     { 5800, SX126X_GFSK_BW_5800 },     { 7300, SX126X_GFSK_BW_7300 },
    { 9700, SX126X_GFSK_BW_9700 },     { 11700, SX126X_GFSK_BW_11700 },   { 14600, SX126X_GFSK_BW_14600 },
    { 19500, SX126X_GFSK_BW_19500 },   { 23400, SX126X_GFSK_BW_23400 },   { 29300, SX126X_GFSK_BW_29300 },
    { 39000, SX126X_GFSK_BW_39000 },   { 46900, SX126X_GFSK_BW_46900 },   { 58600, SX126X_GFSK_BW_58600 },
    { 78200, SX126X_GFSK_BW_78200 },   { 93800, SX126X_GFSK_BW_93800 },   { 117300, SX126X_GFSK_BW_117300 },
    { 156200, SX126X_GFSK_BW_156200 }, { 187200, SX126X_GFSK_BW_187200 }, { 234300, SX126X_GFSK_BW_234300 },
    { 312000, SX126X_GFSK_BW_312000 }, { 373600, SX126X_GFSK_BW_373600 }, { 467000, SX126X_GFSK_BW_467000 },
};

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE VARIABLES -------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS DECLARATION -------------------------------------------
 */

/**
 * @brief 15.1.2 Workaround
 *
 * @remark Before any packet transmission, bit #2 of SX126X_REG_TX_MODULATION shall be set to:
 * 0 if the LoRa BW = 500 kHz
 * 1 for any other LoRa BW
 * 1 for any (G)FSK configuration
 *
 * @param [in] context Chip implementation context.
 * @param [in] pkt_type The modulation type (G)FSK/LoRa
 * @param [in] bw In case of LoRa modulation the bandwith must be specified
 *
 * @returns Operation status
 */
static sx126x_status_t sx126x_tx_modulation_workaround( const void* context, sx126x_pkt_type_t pkt_type,
                                                        sx126x_lora_bw_t bw );

static inline uint32_t sx126x_get_gfsk_crc_len_in_bytes( sx126x_gfsk_crc_types_t crc_type );

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC FUNCTIONS DEFINITION ---------------------------------------------
 */

sx126x_status_t sx126x_set_sleep( const void* context, const sx126x_sleep_cfgs_t cfg )
{
    uint8_t buf[SX126X_SIZE_SET_SLEEP] = { 0 };

    buf[0] = SX126X_SET_SLEEP;

    buf[1] = ( uint8_t ) cfg;

    return ( sx126x_status_t ) sx126x_hal_write( context, buf, SX126X_SIZE_SET_SLEEP, 0, 0 );
}

sx126x_status_t sx126x_set_standby( const void* context, const sx126x_standby_cfg_t cfg )
{
    uint8_t buf[SX126X_SIZE_SET_STANDBY] = { 0 };

    buf[0] = SX126X_SET_STANDBY;

    buf[1] = ( uint8_t ) cfg;

    return ( sx126x_status_t ) sx126x_hal_write( context, buf, SX126X_SIZE_SET_STANDBY, 0, 0 );
}

sx126x_status_t sx126x_set_fs( const void* context )
{
    uint8_t buf[SX126X_SIZE_SET_FS] = { 0 };

    buf[0] = SX126X_SET_FS;

    return ( sx126x_status_t ) sx126x_hal_write( context, buf, SX126X_SIZE_SET_FS, 0, 0 );
}

sx126x_status_t sx126x_set_tx( const void* context, const uint32_t timeout )
{
    uint8_t buf[SX126X_SIZE_SET_TX] = { 0 };

    buf[0] = SX126X_SET_TX;

    buf[1] = ( uint8_t )( timeout >> 16 );
    buf[2] = ( uint8_t )( timeout >> 8 );
    buf[3] = ( uint8_t )( timeout >> 0 );

    return ( sx126x_status_t ) sx126x_hal_write( context, buf, SX126X_SIZE_SET_TX, 0, 0 );
}

sx126x_status_t sx126x_set_rx( const void* context, const uint32_t timeout )
{
    uint8_t buf[SX126X_SIZE_SET_RX] = { 0 };

    buf[0] = SX126X_SET_RX;

    buf[1] = ( uint8_t )( timeout >> 16 );
    buf[2] = ( uint8_t )( timeout >> 8 );
    buf[3] = ( uint8_t )( timeout >> 0 );

    return ( sx126x_status_t ) sx126x_hal_write( context, buf, SX126X_SIZE_SET_RX, 0, 0 );
}

sx126x_status_t sx126x_stop_tmr_on_pbl( const void* context, const bool enable )
{
    uint8_t buf[SX126X_SIZE_SET_STOPTIMERONPREAMBLE] = { 0 };

    buf[0] = SX126X_SET_STOPTIMERONPREAMBLE;

    buf[1] = ( enable == true ) ? 1 : 0;

    return ( sx126x_status_t ) sx126x_hal_write( context, buf, SX126X_SIZE_SET_STOPTIMERONPREAMBLE, 0, 0 );
}

sx126x_status_t sx126x_set_rx_duty_cycle( const void* context, const uint32_t rx_time, const uint32_t sleep_time )
{
    uint8_t buf[SX126X_SIZE_SET_RXDUTYCYCLE] = { 0 };

    buf[0] = SX126X_SET_RXDUTYCYCLE;

    buf[1] = ( uint8_t )( rx_time >> 16 );
    buf[2] = ( uint8_t )( rx_time >> 8 );
    buf[3] = ( uint8_t )( rx_time >> 0 );

    buf[4] = ( uint8_t )( sleep_time >> 16 );
    buf[5] = ( uint8_t )( sleep_time >> 8 );
    buf[6] = ( uint8_t )( sleep_time >> 0 );

    return ( sx126x_status_t ) sx126x_hal_write( context, buf, SX126X_SIZE_SET_RXDUTYCYCLE, 0, 0 );
}

sx126x_status_t sx126x_set_cad( const void* context )
{
    uint8_t buf[SX126X_SIZE_SET_CAD] = { 0 };

    buf[0] = SX126X_SET_CAD;

    return ( sx126x_status_t ) sx126x_hal_write( context, buf, SX126X_SIZE_SET_CAD, 0, 0 );
}

sx126x_status_t sx126x_set_tx_cw( const void* context )
{
    uint8_t buf[SX126X_SIZE_SET_TXCONTINUOUSWAVE] = { 0 };

    buf[0] = SX126X_SET_TXCONTINUOUSWAVE;

    return ( sx126x_status_t ) sx126x_hal_write( context, buf, SX126X_SIZE_SET_TXCONTINUOUSWAVE, 0, 0 );
}

sx126x_status_t sx126x_set_tx_cpbl( const void* context )
{
    uint8_t buf[SX126X_SIZE_SET_TXCONTINUOUSPREAMBLE] = { 0 };

    buf[0] = SX126X_SET_TXCONTINUOUSPREAMBLE;

    return ( sx126x_status_t ) sx126x_hal_write( context, buf, SX126X_SIZE_SET_TXCONTINUOUSPREAMBLE, 0, 0 );
}

sx126x_status_t sx126x_set_reg_mode( const void* context, const sx126x_reg_mod_t mode )
{
    uint8_t buf[SX126X_SIZE_SET_REGULATORMODE] = { 0 };

    buf[0] = SX126X_SET_REGULATORMODE;

    buf[1] = ( uint8_t ) mode;

    return ( sx126x_status_t ) sx126x_hal_write( context, buf, SX126X_SIZE_SET_REGULATORMODE, 0, 0 );
}

sx126x_status_t sx126x_set_xtal_trim( const void* context, uint8_t xta, uint8_t xtb )
{
    uint8_t buf[2];

    buf[0] = xta;
    buf[1] = xtb;

    return ( sx126x_status_t ) sx126x_write_register(context, SX126X_REG_XTATRIM, buf, 2 );
}

sx126x_status_t sx126x_cal( const void* context, const sx126x_cal_mask_t param )
{
    uint8_t buf[SX126X_SIZE_CALIBRATE] = { 0 };

    buf[0] = SX126X_CALIBRATE;

    buf[1] = param;

    return ( sx126x_status_t ) sx126x_hal_write( context, buf, SX126X_SIZE_CALIBRATE, 0, 0 );
}

sx126x_status_t sx126x_cal_img( const void* context, const uint32_t freq_in_hz )
{
    uint8_t buf[SX126X_SIZE_CALIBRATEIMAGE] = { 0 };

    buf[0] = SX126X_CALIBRATEIMAGE;

    if( freq_in_hz > 900000000 )
    {
        buf[1] = 0xE1;
        buf[2] = 0xE9;
    }
    else if( freq_in_hz > 850000000 )
    {
        buf[1] = 0xD7;
        buf[2] = 0xDB;
    }
    else if( freq_in_hz > 770000000 )
    {
        buf[1] = 0xC1;
        buf[2] = 0xC5;
    }
    else if( freq_in_hz > 460000000 )
    {
        buf[1] = 0x75;
        buf[2] = 0x81;
    }
    else
    {
        buf[1] = 0x6B;
        buf[2] = 0x6F;
    }

    return ( sx126x_status_t ) sx126x_hal_write( context, buf, SX126X_SIZE_CALIBRATEIMAGE, 0, 0 );
}

sx126x_status_t sx126x_set_pa_cfg( const void* context, const sx126x_pa_cfg_params_t* params )
{
    uint8_t buf[SX126X_SIZE_SET_PACONFIG] = { 0 };

    buf[0] = SX126X_SET_PACONFIG;

    buf[1] = params->pa_duty_cycle;
    buf[2] = params->hp_max;
    buf[3] = params->device_sel;
    buf[4] = params->pa_lut;

    return ( sx126x_status_t ) sx126x_hal_write( context, buf, SX126X_SIZE_SET_PACONFIG, 0, 0 );
}

sx126x_status_t sx126x_set_rx_tx_fallback_mode( const void* context, const sx126x_fallback_modes_t fallback_mode )
{
    uint8_t buf[SX126X_SIZE_SET_RXTXFALLBACKMODE] = { 0 };

    buf[0] = SX126X_SET_RXTXFALLBACKMODE;

    buf[1] = ( uint8_t ) fallback_mode;

    return ( sx126x_status_t ) sx126x_hal_write( context, buf, SX126X_SIZE_SET_RXTXFALLBACKMODE, 0, 0 );
}

//
// Registers and buffer Access
//

sx126x_status_t sx126x_write_register( const void* context, const uint16_t addr, const uint8_t* buffer,
                                       const uint8_t size )
{
    uint8_t buf[SX126X_SIZE_WRITE_REGISTER] = { 0 };

    buf[0] = SX126X_WRITE_REGISTER;

    buf[1] = ( uint8_t )( addr >> 8 );
    buf[2] = ( uint8_t )( addr >> 0 );

    return ( sx126x_status_t ) sx126x_hal_write( context, buf, SX126X_SIZE_WRITE_REGISTER, buffer, size );
}

sx126x_status_t sx126x_read_register( const void* context, const uint16_t addr, uint8_t* buffer, const uint8_t size )
{
    uint8_t         buf[SX126X_SIZE_READ_REGISTER] = { 0 };
    sx126x_status_t status                         = SX126X_STATUS_ERROR;

    buf[0] = SX126X_READ_REGISTER;

    buf[1] = ( uint8_t )( addr >> 8 );
    buf[2] = ( uint8_t )( addr >> 0 );

    status = ( sx126x_status_t ) sx126x_hal_read( context, buf, SX126X_SIZE_READ_REGISTER, buffer, size );

    return status;
}

sx126x_status_t sx126x_write_buffer( const void* context, const uint8_t offset, const uint8_t* buffer,
                                     const uint8_t size )
{
    uint8_t buf[SX126X_SIZE_WRITE_BUFFER] = { 0 };

    buf[0] = SX126X_WRITE_BUFFER;

    buf[1] = offset;

    return ( sx126x_status_t ) sx126x_hal_write( context, buf, SX126X_SIZE_WRITE_BUFFER, buffer, size );
}

sx126x_status_t sx126x_read_buffer( const void* context, const uint8_t offset, uint8_t* buffer, const uint8_t size )
{
    uint8_t         buf[SX126X_SIZE_READ_BUFFER] = { 0 };
    sx126x_status_t status                       = SX126X_STATUS_ERROR;

    buf[0] = SX126X_READ_BUFFER;

    buf[1] = offset;

    status = ( sx126x_status_t ) sx126x_hal_read( context, buf, SX126X_SIZE_READ_BUFFER, buffer, size );

    return status;
}

//
// DIO and IRQ Control Functions
//
sx126x_status_t sx126x_set_dio_irq_params( const void* context, const uint16_t irq_mask, const uint16_t dio1_mask,
                                           const uint16_t dio2_mask, const uint16_t dio3_mask )
{
    uint8_t buf[SX126X_SIZE_SET_DIOIRQPARAMS] = { 0 };

    buf[0] = SX126X_SET_DIOIRQPARAMS;

    buf[1] = ( uint8_t )( irq_mask >> 8 );
    buf[2] = ( uint8_t )( irq_mask >> 0 );

    buf[3] = ( uint8_t )( dio1_mask >> 8 );
    buf[4] = ( uint8_t )( dio1_mask >> 0 );

    buf[5] = ( uint8_t )( dio2_mask >> 8 );
    buf[6] = ( uint8_t )( dio2_mask >> 0 );

    buf[7] = ( uint8_t )( dio3_mask >> 8 );
    buf[8] = ( uint8_t )( dio3_mask >> 0 );

    return ( sx126x_status_t ) sx126x_hal_write( context, buf, SX126X_SIZE_SET_DIOIRQPARAMS, 0, 0 );
}

sx126x_status_t sx126x_get_irq_status( const void* context, sx126x_irq_mask_t* irq )
{
    uint8_t         buf[SX126X_SIZE_GET_IRQSTATUS]         = { 0 };
    uint8_t         irq_local[sizeof( sx126x_irq_mask_t )] = { 0x00 };
    sx126x_status_t status                                 = SX126X_STATUS_ERROR;

    buf[0] = SX126X_GET_IRQSTATUS;

    status = ( sx126x_status_t ) sx126x_hal_read( context, buf, SX126X_SIZE_GET_IRQSTATUS, irq_local,
                                                  sizeof( sx126x_irq_mask_t ) );

    if( status == SX126X_STATUS_OK )
    {
        *irq = ( ( sx126x_irq_mask_t ) irq_local[0] << 8 ) + ( ( sx126x_irq_mask_t ) irq_local[1] << 0 );
    }

    return status;
}

sx126x_status_t sx126x_clear_irq_status( const void* context, const sx126x_irq_mask_t irq_mask )
{
    uint8_t buf[SX126X_SIZE_CLR_IRQSTATUS] = { 0 };

    buf[0] = SX126X_CLR_IRQSTATUS;

    buf[1] = ( uint8_t )( irq_mask >> 8 );
    buf[2] = ( uint8_t )( irq_mask >> 0 );

    return ( sx126x_status_t ) sx126x_hal_write( context, buf, SX126X_SIZE_CLR_IRQSTATUS, 0, 0 );
}

sx126x_status_t sx126x_get_and_clear_irq_status( const void* context, sx126x_irq_mask_t* irq )
{
    sx126x_irq_mask_t sx126x_irq_mask = SX126X_IRQ_NONE;

    sx126x_status_t status = sx126x_get_irq_status( context, &sx126x_irq_mask );

    if( ( status == SX126X_STATUS_OK ) && ( sx126x_irq_mask != 0 ) )
    {
        status = sx126x_clear_irq_status( context, sx126x_irq_mask );
    }
    if( ( status == SX126X_STATUS_OK ) && ( irq != NULL ) )
    {
        *irq = sx126x_irq_mask;
    }
    return status;
}

sx126x_status_t sx126x_set_dio2_as_rf_sw_ctrl( const void* context, const bool enable )
{
    uint8_t buf[SX126X_SIZE_SET_DIO2ASRFSWITCHCTRL] = { 0 };

    buf[0] = SX126X_SET_DIO2ASRFSWITCHCTRL;

    buf[1] = ( enable == true ) ? 1 : 0;

    return ( sx126x_status_t ) sx126x_hal_write( context, buf, SX126X_SIZE_SET_DIO2ASRFSWITCHCTRL, 0, 0 );
}

sx126x_status_t sx126x_set_dio3_as_tcxo_ctrl( const void* context, const sx126x_tcxo_ctrl_voltages_t tcxo_voltage,
                                              const uint32_t timeout )
{
    uint8_t buf[SX126X_SIZE_SET_DIO3ASTCXOCTRL] = { 0 };

    buf[0] = SX126X_SET_DIO3ASTCXOCTRL;

    buf[1] = ( uint8_t ) tcxo_voltage;

    buf[2] = ( uint8_t )( timeout >> 16 );
    buf[3] = ( uint8_t )( timeout >> 8 );
    buf[4] = ( uint8_t )( timeout >> 0 );

    return ( sx126x_status_t ) sx126x_hal_write( context, buf, SX126X_SIZE_SET_DIO3ASTCXOCTRL, 0, 0 );
}

//
// RF Modulation and Packet-Related Functions
//

sx126x_status_t sx126x_set_rf_freq( const void* context, const uint32_t freq_in_hz )
{
    uint8_t  buf[SX126X_SIZE_SET_RFFREQUENCY] = { 0 };
    uint32_t freq;

    SX126X_FREQ_IN_HZ_TO_PLL_STEP( freq_in_hz, freq );

    buf[0] = SX126X_SET_RFFREQUENCY;

    buf[1] = ( uint8_t )( freq >> 24 );
    buf[2] = ( uint8_t )( freq >> 16 );
    buf[3] = ( uint8_t )( freq >> 8 );
    buf[4] = ( uint8_t )( freq >> 0 );

    return ( sx126x_status_t ) sx126x_hal_write( context, buf, SX126X_SIZE_SET_RFFREQUENCY, 0, 0 );
}

sx126x_status_t sx126x_set_pkt_type( const void* context, const sx126x_pkt_type_t pkt_type )
{
    uint8_t buf[SX126X_SIZE_SET_PACKETTYPE] = { 0 };

    buf[0] = SX126X_SET_PACKETTYPE;

    buf[1] = ( uint8_t ) pkt_type;

    return ( sx126x_status_t ) sx126x_hal_write( context, buf, SX126X_SIZE_SET_PACKETTYPE, 0, 0 );
}

sx126x_status_t sx126x_get_pkt_type( const void* context, sx126x_pkt_type_t* pkt_type )
{
    uint8_t         buf[SX126X_SIZE_GET_PACKETTYPE] = { 0 };
    sx126x_status_t status                          = SX126X_STATUS_ERROR;

    buf[0] = SX126X_GET_PACKETTYPE;

    status = ( sx126x_status_t ) sx126x_hal_read( context, buf, SX126X_SIZE_GET_PACKETTYPE, ( uint8_t* ) pkt_type, 1 );

    return status;
}

sx126x_status_t sx126x_set_tx_params( const void* context, const int8_t pwr_in_dbm, const sx126x_ramp_time_t ramp_time )
{
    uint8_t buf[SX126X_SIZE_SET_TXPARAMS] = { 0 };

    buf[0] = SX126X_SET_TXPARAMS;

    buf[1] = ( uint8_t ) pwr_in_dbm;
    buf[2] = ( uint8_t ) ramp_time;

    return ( sx126x_status_t ) sx126x_hal_write( context, buf, SX126X_SIZE_SET_TXPARAMS, 0, 0 );
}

sx126x_status_t sx126x_set_gfsk_mod_params( const void* context, const sx126x_mod_params_gfsk_t* params )
{
    sx126x_status_t status = SX126X_STATUS_ERROR;

    uint8_t  buf[SX126X_SIZE_SET_MODULATIONPARAMS_GFSK] = { 0 };
    uint32_t bitrate                                    = ( uint32_t )( 32 * SX126X_XTAL_FREQ / params->br_in_bps );
    uint32_t fdev;

    SX126X_FREQ_IN_HZ_TO_PLL_STEP( params->fdev_in_hz, fdev );

    buf[0] = SX126X_SET_MODULATIONPARAMS;

    buf[1] = ( uint8_t )( bitrate >> 16 );
    buf[2] = ( uint8_t )( bitrate >> 8 );
    buf[3] = ( uint8_t )( bitrate >> 0 );

    buf[4] = ( uint8_t )( params->mod_shape );

    buf[5] = params->bw_dsb_param;

    buf[6] = ( uint8_t )( fdev >> 16 );
    buf[7] = ( uint8_t )( fdev >> 8 );
    buf[8] = ( uint8_t )( fdev >> 0 );

    status = ( sx126x_status_t ) sx126x_hal_write( context, buf, SX126X_SIZE_SET_MODULATIONPARAMS_GFSK, 0, 0 );

    if( status == SX126X_STATUS_OK )
    {
        // WORKAROUND - Modulation Quality with 500 kHz LoRa Bandwidth, see DS_SX1261-2_V1.2 datasheet chapter 15.1
        status = sx126x_tx_modulation_workaround( context, SX126X_PKT_TYPE_GFSK, 0 );
        // WORKAROUND END
    }
    return status;
}

sx126x_status_t sx126x_set_lora_mod_params( const void* context, const sx126x_mod_params_lora_t* params )
{
    sx126x_status_t status = SX126X_STATUS_ERROR;

    uint8_t buf[SX126X_SIZE_SET_MODULATIONPARAMS_LORA] = { 0 };

    buf[0] = SX126X_SET_MODULATIONPARAMS;

    buf[1] = ( uint8_t )( params->sf );
    buf[2] = ( uint8_t )( params->bw );
    buf[3] = ( uint8_t )( params->cr );
    buf[4] = params->ldro & 0x01;

    status = ( sx126x_status_t ) sx126x_hal_write( context, buf, SX126X_SIZE_SET_MODULATIONPARAMS_LORA, 0, 0 );

    if( status == SX126X_STATUS_OK )
    {
        // WORKAROUND - Modulation Quality with 500 kHz LoRa Bandwidth, see datasheet DS_SX1261-2_V1.2 §15.1
        status = sx126x_tx_modulation_workaround( context, SX126X_PKT_TYPE_LORA, params->bw );
        // WORKAROUND END
    }

    return status;
}

sx126x_status_t sx126x_set_gfsk_pkt_params( const void* context, const sx126x_pkt_params_gfsk_t* params )
{
    uint8_t buf[SX126X_SIZE_SET_PACKETPARAMS_GFSK] = { 0 };

    buf[0] = SX126X_SET_PACKETPARAMS;

    buf[1] = ( uint8_t )( params->pbl_len_in_bits >> 8 );
    buf[2] = ( uint8_t )( params->pbl_len_in_bits >> 0 );
    buf[3] = ( uint8_t )( params->pbl_min_det );
    buf[4] = params->sync_word_len_in_bits;
    buf[5] = ( uint8_t )( params->addr_cmp );
    buf[6] = ( uint8_t )( params->hdr_type );
    buf[7] = params->pld_len_in_bytes;
    buf[8] = ( uint8_t )( params->crc_type );
    buf[9] = ( uint8_t )( params->dc_free );

    return ( sx126x_status_t ) sx126x_hal_write( context, buf, SX126X_SIZE_SET_PACKETPARAMS_GFSK, 0, 0 );
}

sx126x_status_t sx126x_set_lora_pkt_params( const void* context, const sx126x_pkt_params_lora_t* params )
{
    sx126x_status_t status = SX126X_STATUS_ERROR;

    uint8_t buf[SX126X_SIZE_SET_PACKETPARAMS_LORA] = { 0 };

    buf[0] = SX126X_SET_PACKETPARAMS;

    buf[1] = ( uint8_t )( params->pbl_len_in_symb >> 8 );
    buf[2] = ( uint8_t )( params->pbl_len_in_symb >> 0 );
    buf[3] = ( uint8_t )( params->hdr_type );
    buf[4] = params->pld_len_in_bytes;
    buf[5] = ( uint8_t )( params->crc_is_on ? 1 : 0 );
    buf[6] = ( uint8_t )( params->invert_iq_is_on ? 1 : 0 );

    status = ( sx126x_status_t ) sx126x_hal_write( context, buf, SX126X_SIZE_SET_PACKETPARAMS_LORA, 0, 0 );

    // WORKAROUND - Optimizing the Inverted IQ Operation, see datasheet DS_SX1261-2_V1.2 §15.4
    if( status == SX126X_STATUS_OK )
    {
        uint8_t reg_value = 0;

        status = sx126x_read_register( context, SX126X_REG_IRQ_POLARITY, &reg_value, 1 );
        if( status == SX126X_STATUS_OK )
        {
            if( params->invert_iq_is_on == true )
            {
                reg_value &= ~( 1 << 2 );  // Bit 2 set to 0 when using inverted IQ polarity
            }
            else
            {
                reg_value |= ( 1 << 2 );  // Bit 2 set to 1 when using standard IQ polarity
            }
            status = sx126x_write_register( context, SX126X_REG_IRQ_POLARITY, &reg_value, 1 );
        }
    }
    // WORKAROUND END

    return status;
}

sx126x_status_t sx126x_set_cad_params( const void* context, const sx126x_lora_cad_params_t* params )
{
    uint8_t buf[SX126X_SIZE_SET_CADPARAMS] = { 0 };

    buf[0] = SX126X_SET_CADPARAMS;

    buf[1] = ( uint8_t ) params->cad_symb_nb;
    buf[2] = params->cad_det_peak;
    buf[3] = params->cad_det_min;
    buf[4] = ( uint8_t ) params->cad_exit_mode;
    buf[5] = ( uint8_t )( params->cad_timeout >> 16 );
    buf[6] = ( uint8_t )( params->cad_timeout >> 8 );
    buf[7] = ( uint8_t )( params->cad_timeout >> 0 );

    return ( sx126x_status_t ) sx126x_hal_write( context, buf, SX126X_SIZE_SET_CADPARAMS, 0, 0 );
}

sx126x_status_t sx126x_set_buffer_base_addr( const void* context, const uint8_t tx_base_addr,
                                             const uint8_t rx_base_addr )
{
    uint8_t buf[SX126X_SIZE_SET_BUFFERBASEADDRESS] = { 0 };

    buf[0] = SX126X_SET_BUFFERBASEADDRESS;

    buf[1] = tx_base_addr;
    buf[2] = rx_base_addr;

    return ( sx126x_status_t ) sx126x_hal_write( context, buf, SX126X_SIZE_SET_BUFFERBASEADDRESS, 0, 0 );
}

sx126x_status_t sx126x_set_lora_symb_nb_timeout( const void* context, const uint8_t nb_of_symbs )
{
    uint8_t         buf[SX126X_SIZE_SET_LORASYMBNUMTIMEOUT] = { 0 };
    sx126x_status_t status                                  = SX126X_STATUS_ERROR;

    buf[0] = SX126X_SET_LORASYMBNUMTIMEOUT;

    buf[1] = nb_of_symbs;

    status = ( sx126x_status_t ) sx126x_hal_write( context, buf, SX126X_SIZE_SET_LORASYMBNUMTIMEOUT, 0, 0 );

    if( ( status == SX126X_STATUS_OK ) && ( nb_of_symbs >= 64 ) )
    {
        uint8_t mant = nb_of_symbs >> 1;
        uint8_t exp  = 0;
        uint8_t reg  = 0;

        while( mant > 31 )
        {
            mant >>= 2;
            exp++;
        }

        reg    = exp + ( mant << 3 );
        status = sx126x_write_register( context, SX126X_REG_LR_SYNCH_TIMEOUT, &reg, 1 );
    }
    return status;
}

//
// Communication Status Information
//

sx126x_status_t sx126x_get_status( const void* context, sx126x_chip_status_t* radio_status )
{
    uint8_t         buf[SX126X_SIZE_GET_STATUS] = { 0 };
    uint8_t         status_local                = 0;
    sx126x_status_t status                      = SX126X_STATUS_ERROR;

    buf[0] = SX126X_GET_STATUS;

    status = ( sx126x_status_t ) sx126x_hal_read( context, buf, SX126X_SIZE_GET_STATUS, &status_local, 1 );

    if( status == SX126X_STATUS_OK )
    {
        radio_status->cmd_status = ( status_local & SX126X_CMD_STATUS_MASK ) >> SX126X_CMD_STATUS_POS;
        radio_status->chip_mode  = ( status_local & SX126X_CHIP_MODES_MASK ) >> SX126X_CHIP_MODES_POS;
    }

    return status;
}

sx126x_status_t sx126x_get_rx_buffer_status( const void* context, sx126x_rx_buffer_status_t* rx_buffer_status )
{
    uint8_t         buf[SX126X_SIZE_GET_RXBUFFERSTATUS]               = { 0 };
    uint8_t         status_local[sizeof( sx126x_rx_buffer_status_t )] = { 0x00 };
    sx126x_status_t status                                            = SX126X_STATUS_ERROR;

    buf[0] = SX126X_GET_RXBUFFERSTATUS;

    status = ( sx126x_status_t ) sx126x_hal_read( context, buf, SX126X_SIZE_GET_RXBUFFERSTATUS, status_local,
                                                  sizeof( sx126x_rx_buffer_status_t ) );

    if( status == SX126X_STATUS_OK )
    {
        rx_buffer_status->pld_len_in_bytes     = status_local[0];
        rx_buffer_status->buffer_start_pointer = status_local[1];
    }

    return status;
}

sx126x_status_t sx126x_get_gfsk_pkt_status( const void* context, sx126x_pkt_status_gfsk_t* pkt_status )
{
    uint8_t         buf[SX126X_SIZE_GET_PACKETSTATUS]                    = { 0 };
    uint8_t         pkt_status_local[sizeof( sx126x_pkt_status_gfsk_t )] = { 0x00 };
    sx126x_status_t status                                               = SX126X_STATUS_ERROR;

    buf[0] = SX126X_GET_PACKETSTATUS;

    status = ( sx126x_status_t ) sx126x_hal_read( context, buf, SX126X_SIZE_GET_PACKETSTATUS, pkt_status_local,
                                                  sizeof( sx126x_pkt_status_gfsk_t ) );

    if( status == SX126X_STATUS_OK )
    {
        pkt_status->rx_status = pkt_status_local[0];
        pkt_status->rssi_sync = ( int8_t )( -pkt_status_local[1] >> 1 );
        pkt_status->rssi_avg  = ( int8_t )( -pkt_status_local[2] >> 1 );
    }

    return status;
}

sx126x_status_t sx126x_get_lora_pkt_status( const void* context, sx126x_pkt_status_lora_t* pkt_status )
{
    uint8_t         buf[SX126X_SIZE_GET_PACKETSTATUS]                    = { 0 };
    uint8_t         pkt_status_local[sizeof( sx126x_pkt_status_lora_t )] = { 0x00 };
    sx126x_status_t status                                               = SX126X_STATUS_ERROR;

    buf[0] = SX126X_GET_PACKETSTATUS;

    status = ( sx126x_status_t ) sx126x_hal_read( context, buf, SX126X_SIZE_GET_PACKETSTATUS, pkt_status_local,
                                                  sizeof( sx126x_pkt_status_lora_t ) );

    if( status == SX126X_STATUS_OK )
    {
        pkt_status->rssi_pkt_in_dbm        = ( int8_t )( -pkt_status_local[0] >> 1 );
        pkt_status->snr_pkt_in_db          = ( ( ( int8_t ) pkt_status_local[1] ) + 2 ) >> 2;
        pkt_status->signal_rssi_pkt_in_dbm = ( int8_t )( -pkt_status_local[2] >> 1 );
    }

    return status;
}

sx126x_status_t sx126x_get_rssi_inst( const void* context, int16_t* rssi_in_dbm )
{
    uint8_t         buf[SX126X_SIZE_GET_RSSIINST] = { 0 };
    uint8_t         rssi_local                    = 0x00;
    sx126x_status_t status                        = SX126X_STATUS_ERROR;

    buf[0] = SX126X_GET_RSSIINST;

    status = ( sx126x_status_t ) sx126x_hal_read( context, buf, SX126X_SIZE_GET_RSSIINST, &rssi_local, 1 );

    if( status == SX126X_STATUS_OK )
    {
        *rssi_in_dbm = ( int8_t )( -rssi_local >> 1 );
    }

    return status;
}

sx126x_status_t sx126x_get_gfsk_stats( const void* context, sx126x_stats_gfsk_t* stats )
{
    uint8_t         buf[SX126X_SIZE_GET_STATS]                 = { 0 };
    uint8_t         stats_local[sizeof( sx126x_stats_gfsk_t )] = { 0 };
    sx126x_status_t status                                     = SX126X_STATUS_ERROR;

    buf[0] = SX126X_GET_STATS;

    status = ( sx126x_status_t ) sx126x_hal_read( context, buf, SX126X_SIZE_GET_STATS, stats_local,
                                                  sizeof( sx126x_stats_gfsk_t ) );

    if( status == SX126X_STATUS_OK )
    {
        stats->nb_pkt_received  = ( ( uint16_t ) stats_local[0] << 8 ) + ( uint16_t ) stats_local[1];
        stats->nb_pkt_crc_error = ( ( uint16_t ) stats_local[2] << 8 ) + ( uint16_t ) stats_local[3];
        stats->nb_pkt_len_error = ( ( uint16_t ) stats_local[4] << 8 ) + ( uint16_t ) stats_local[5];
    }

    return status;
}

sx126x_status_t sx126x_get_lora_stats( const void* context, sx126x_stats_lora_t* stats )
{
    uint8_t         buf[SX126X_SIZE_GET_STATS]                 = { 0 };
    uint8_t         stats_local[sizeof( sx126x_stats_lora_t )] = { 0 };
    sx126x_status_t status                                     = SX126X_STATUS_ERROR;

    buf[0] = SX126X_GET_STATS;

    status = ( sx126x_status_t ) sx126x_hal_read( context, buf, SX126X_SIZE_GET_STATS, stats_local,
                                                  sizeof( sx126x_stats_lora_t ) );

    if( status == SX126X_STATUS_OK )
    {
        stats->nb_pkt_received  = ( ( uint16_t ) stats_local[0] << 8 ) + ( uint16_t ) stats_local[1];
        stats->nb_pkt_crc_error = ( ( uint16_t ) stats_local[2] << 8 ) + ( uint16_t ) stats_local[3];
        stats->nb_pkt_hdr_error = ( ( uint16_t ) stats_local[4] << 8 ) + ( uint16_t ) stats_local[5];
    }
    return status;
}

sx126x_status_t sx126x_reset_stats( const void* context )
{
    uint8_t buf[SX126X_SIZE_RESET_STATS] = { 0 };

    buf[0] = SX126X_RESET_STATS;

    return ( sx126x_status_t ) sx126x_hal_write( context, buf, SX126X_SIZE_RESET_STATS, 0, 0 );
}

//
// Miscellaneous
//

sx126x_status_t sx126x_reset( const void* context )
{
    return ( sx126x_status_t ) sx126x_hal_reset( context );
}

sx126x_status_t sx126x_wakeup( const void* context )
{
    return ( sx126x_status_t ) sx126x_hal_wakeup( context );
}

sx126x_status_t sx126x_get_device_errors( const void* context, sx126x_errors_mask_t* errors )
{
    uint8_t         buf[SX126X_SIZE_GET_DEVICEERRORS]            = { 0 };
    uint8_t         errors_local[sizeof( sx126x_errors_mask_t )] = { 0x00 };
    sx126x_status_t status                                       = SX126X_STATUS_ERROR;

    buf[0] = SX126X_GET_DEVICEERRORS;

    status = ( sx126x_status_t ) sx126x_hal_read( context, buf, SX126X_SIZE_GET_DEVICEERRORS, errors_local,
                                                  sizeof( sx126x_errors_mask_t ) );

    if( status == SX126X_STATUS_OK )
    {
        *errors = ( ( sx126x_errors_mask_t ) errors_local[0] << 8 ) + ( ( sx126x_errors_mask_t ) errors_local[1] << 0 );
    }

    return status;
}

sx126x_status_t sx126x_clear_device_errors( const void* context )
{
    uint8_t buf[SX126X_SIZE_CLR_DEVICEERRORS] = { 0 };

    buf[0] = SX126X_CLR_DEVICEERRORS;

    return ( sx126x_status_t ) sx126x_hal_write( context, buf, SX126X_SIZE_CLR_DEVICEERRORS, 0, 0 );
}

sx126x_status_t sx126x_get_gfsk_bw_param( const uint32_t bw, uint8_t* param )
{
    sx126x_status_t status = SX126X_STATUS_ERROR;

    if( bw != 0 )
    {
        status = SX126X_STATUS_UNKNOWN_VALUE;
        for( uint8_t i = 0; i < ( sizeof( gfsk_bw ) / sizeof( gfsk_bw_t ) ); i++ )
        {
            if( bw <= gfsk_bw[i].bw )
            {
                *param = gfsk_bw[i].param;
                status = SX126X_STATUS_OK;
                break;
            }
        }
    }

    return status;
}

uint32_t sx126x_get_lora_bw_in_hz( sx126x_lora_bw_t bw )
{
    uint32_t bw_in_hz = 0;

    switch( bw )
    {
    case SX126X_LORA_BW_007:
        bw_in_hz = 7812UL;
        break;
    case SX126X_LORA_BW_010:
        bw_in_hz = 10417UL;
        break;
    case SX126X_LORA_BW_015:
        bw_in_hz = 15625UL;
        break;
    case SX126X_LORA_BW_020:
        bw_in_hz = 20833UL;
        break;
    case SX126X_LORA_BW_031:
        bw_in_hz = 31250UL;
        break;
    case SX126X_LORA_BW_041:
        bw_in_hz = 41667UL;
        break;
    case SX126X_LORA_BW_062:
        bw_in_hz = 62500UL;
        break;
    case SX126X_LORA_BW_125:
        bw_in_hz = 125000UL;
        break;
    case SX126X_LORA_BW_250:
        bw_in_hz = 250000UL;
        break;
    case SX126X_LORA_BW_500:
        bw_in_hz = 500000UL;
        break;
    }

    return bw_in_hz;
}

uint32_t sx126x_get_lora_time_on_air_numerator( const sx126x_pkt_params_lora_t* pkt_p,
                                                const sx126x_mod_params_lora_t* mod_p )
{
    const int32_t pld_len_in_bytes = pkt_p->pld_len_in_bytes;
    const int32_t sf               = mod_p->sf;
    const bool    pld_is_fix       = pkt_p->hdr_type == SX126X_LORA_PKT_IMPLICIT;

    int32_t fine_synch        = ( sf <= 6 ) ? 1 : 0;
    bool     long_interleaving = ( mod_p->cr > 4 );

    int32_t total_bytes_nb = pld_len_in_bytes + ( pkt_p->crc_is_on ? 2 : 0 );
    int32_t tx_bits_symbol = sf - 2 * ( mod_p->ldro != 0 ? 1 : 0 );

    int32_t ceil_numerator;
    int32_t ceil_denominator;

    int32_t intermed;

    int32_t symbols_nb_data;
    int32_t  tx_infobits_header;
    int32_t  tx_infobits_payload;

    if( long_interleaving )
    {
        const int32_t fec_rate_numerator   = 4;
        const int32_t fec_rate_denominator = ( mod_p->cr + ( mod_p->cr == 7 ? 1 : 0 ) );

        if( pld_is_fix )
        {
            int32_t tx_bits_symbol_start = sf - 2 + 2 * fine_synch;
            if( 8 * total_bytes_nb * fec_rate_denominator <= 7 * fec_rate_numerator * tx_bits_symbol_start )
            {
                ceil_numerator   = 8 * total_bytes_nb * fec_rate_denominator;
                ceil_denominator = fec_rate_numerator * tx_bits_symbol_start;
            }
            else
            {
                int32_t tx_codedbits_header = tx_bits_symbol_start * 8;
                ceil_numerator = 8 * fec_rate_numerator * tx_bits_symbol + 8 * total_bytes_nb * fec_rate_denominator -
                                 fec_rate_numerator * tx_codedbits_header;
                ceil_denominator = fec_rate_numerator * tx_bits_symbol;
            }
        }
        else
        {
            tx_infobits_header = ( sf * 4 + fine_synch * 8 - 28 ) & ~0x07;
            if( tx_infobits_header < 8 * total_bytes_nb )
            {
                if( tx_infobits_header > 8 * pld_len_in_bytes )
                {
                    tx_infobits_header = 8 * pld_len_in_bytes;
                }
            }
            tx_infobits_payload = 8 * total_bytes_nb - tx_infobits_header;
            if( tx_infobits_payload < 0 )
            {
                tx_infobits_payload = 0;
            }

            ceil_numerator   = tx_infobits_payload * fec_rate_denominator + 8 * fec_rate_numerator * tx_bits_symbol;
            ceil_denominator = fec_rate_numerator * tx_bits_symbol;
        }
    }
    else
    {
        tx_infobits_header = sf * 4 + fine_synch * 8 - 8;

        if( !pld_is_fix )
        {
            tx_infobits_header -= 20;
        }

        tx_infobits_payload = 8 * total_bytes_nb - tx_infobits_header;

        if( tx_infobits_payload < 0 )
            tx_infobits_payload = 0;

        ceil_numerator   = tx_infobits_payload;
        ceil_denominator = 4 * tx_bits_symbol;
    }

    symbols_nb_data = ( ( ceil_numerator + ceil_denominator - 1 ) / ceil_denominator );
    if( !long_interleaving )
    {
        symbols_nb_data = symbols_nb_data * ( mod_p->cr + 4 ) + 8;
    }
    intermed = pkt_p->pbl_len_in_symb + 4 + 2 * fine_synch + symbols_nb_data;

    return ( uint32_t )( ( 4 * intermed + 1 ) * ( 1 << ( sf - 2 ) ) ) - 1;
}

uint32_t sx126x_get_lora_cad_duration_microsecs(uint8_t symbol
                                        ,const sx126x_mod_params_lora_t* mod_p )
{
    return 2000;
}

uint32_t sx126x_get_lora_time_on_air_in_ms( const sx126x_pkt_params_lora_t* pkt_p,
                                            const sx126x_mod_params_lora_t* mod_p )
{
    uint32_t numerator   = 1000U * sx126x_get_lora_time_on_air_numerator( pkt_p, mod_p );
    uint32_t denominator = sx126x_get_lora_bw_in_hz( mod_p->bw );
    // Perform integral ceil()
    return ( numerator + denominator - 1 ) / denominator;
}

uint32_t sx126x_get_gfsk_time_on_air_numerator( const sx126x_pkt_params_gfsk_t* pkt_p )
{
    return pkt_p->pbl_len_in_bits + ( pkt_p->hdr_type == SX126X_GFSK_PKT_VAR_LEN ? 8 : 0 ) +
           pkt_p->sync_word_len_in_bits +
           ( ( pkt_p->pld_len_in_bytes + ( pkt_p->addr_cmp == SX126X_GFSK_ADDR_CMP_FILT_OFF ? 0 : 1 ) +
               sx126x_get_gfsk_crc_len_in_bytes( pkt_p->crc_type ) )
             << 3 );
}

uint32_t sx126x_get_gfsk_time_on_air_in_ms( const sx126x_pkt_params_gfsk_t* pkt_p,
                                            const sx126x_mod_params_gfsk_t* mod_p )
{
    uint32_t numerator   = 1000U * sx126x_get_gfsk_time_on_air_numerator( pkt_p );
    uint32_t denominator = mod_p->br_in_bps;

    // Perform integral ceil()
    return ( numerator + denominator - 1 ) / denominator;
}

sx126x_status_t sx126x_get_random_numbers( const void* context, uint32_t* numbers, unsigned int n )
{
    sx126x_status_t status;

    uint8_t tmp_ana_lna   = 0x00;
    uint8_t tmp_ana_mixer = 0x00;
    uint8_t tmp           = 0x00;

    // Configure for random number generation
    status = sx126x_read_register( context, SX126X_REG_ANA_LNA, &tmp_ana_lna, 1 );
    if( status != SX126X_STATUS_OK )
    {
        return status;
    }
    tmp    = tmp_ana_lna & ~( 1 << 0 );
    status = sx126x_write_register( context, SX126X_REG_ANA_LNA, &tmp, 1 );
    if( status != SX126X_STATUS_OK )
    {
        return status;
    }

    status = sx126x_read_register( context, SX126X_REG_ANA_MIXER, &tmp_ana_mixer, 1 );
    if( status != SX126X_STATUS_OK )
    {
        return status;
    }
    tmp    = tmp_ana_mixer & ~( 1 << 7 );
    status = sx126x_write_register( context, SX126X_REG_ANA_MIXER, &tmp, 1 );
    if( status != SX126X_STATUS_OK )
    {
        return status;
    }

    // Start RX continuous
    status = sx126x_set_rx( context, 0xFFFFFF );
    if( status != SX126X_STATUS_OK )
    {
        return status;
    }

    // Store values
    for( unsigned int i = 0; i < n; i++ )
    {
        status = sx126x_read_register( context, SX126X_REG_RNGBASEADDR, ( uint8_t* ) &numbers[i], 4 );
        if( status != SX126X_STATUS_OK )
        {
            return status;
        }
    }

    status = sx126x_set_standby( context, SX126X_STANDBY_CFG_RC );
    if( status != SX126X_STATUS_OK )
    {
        return status;
    }

    // Restore registers
    status = sx126x_write_register( context, SX126X_REG_ANA_LNA, &tmp_ana_lna, 1 );
    if( status != SX126X_STATUS_OK )
    {
        return status;
    }
    status = sx126x_write_register( context, SX126X_REG_ANA_MIXER, &tmp_ana_mixer, 1 );

    return status;
}

//
// Registers access
//

sx126x_status_t sx126x_cfg_rx_boosted( const void* context, const bool state )
{
    if( state == true )
    {
        return sx126x_write_register( context, SX126X_REG_RXGAIN, ( uint8_t[] ){ 0x96 }, 1 );
    }
    else
    {
        return sx126x_write_register( context, SX126X_REG_RXGAIN, ( uint8_t[] ){ 0x94 }, 1 );
    }
}

sx126x_status_t sx126x_set_gfsk_sync_word( const void* context, const uint8_t* sync_word, const uint8_t sync_word_len )
{
    sx126x_status_t status = SX126X_STATUS_ERROR;
    uint8_t         buf[8] = { 0 };

    if( sync_word_len <= 8 )
    {
        memcpy( buf, sync_word, sync_word_len );
        status = sx126x_write_register( context, SX126X_REG_SYNCWORDBASEADDRESS, buf, 8 );
    }

    return status;
}

sx126x_status_t sx126x_set_lora_sync_word( const void* context, const uint8_t sync_word )
{
    sx126x_status_t status    = SX126X_STATUS_ERROR;
    uint8_t         buffer[2] = { 0x00 };

    status = sx126x_read_register( context, SX126X_REG_LR_SYNCWORD, buffer, 2 );

    if( status == SX126X_STATUS_OK )
    {
        buffer[0] = ( buffer[0] & ~0xF0 ) + ( sync_word & 0xF0 );
        buffer[1] = ( buffer[1] & ~0xF0 ) + ( ( sync_word & 0x0F ) << 4 );

        status = sx126x_write_register( context, SX126X_REG_LR_SYNCWORD, buffer, 2 );
    }

    return status;
}

sx126x_status_t sx126x_set_gfsk_crc_seed( const void* context, uint16_t seed )
{
    uint8_t s[] = { ( uint8_t )( seed >> 8 ), ( uint8_t ) seed };

    return sx126x_write_register( context, SX126X_REG_CRCSEEDBASEADDR, s, sizeof( s ) );
}

sx126x_status_t sx126x_set_gfsk_crc_polynomial( const void* context, const uint16_t polynomial )
{
    uint8_t poly[] = { ( uint8_t )( polynomial >> 8 ), ( uint8_t ) polynomial };

    return sx126x_write_register( context, SX126X_REG_CRCPOLYBASEADDR, poly, sizeof( poly ) );
}

sx126x_status_t sx126x_set_gfsk_whitening_seed( const void* context, const uint16_t seed )
{
    sx126x_status_t status    = SX126X_STATUS_ERROR;
    uint8_t         reg_value = 0;

    // The SX126X_REG_WHITSEEDBASEADDR @ref LSBit is used for the seed value. The 7 MSBits must not be modified.
    // Thus, we first need to read the current value and then change the LSB according to the provided seed @ref value.
    status = sx126x_read_register( context, SX126X_REG_WHITSEEDBASEADDR, &reg_value, 1 );
    if( status == SX126X_STATUS_OK )
    {
        reg_value = ( reg_value & 0xFE ) | ( ( uint8_t )( seed >> 8 ) & 0x01 );
        status    = sx126x_write_register( context, SX126X_REG_WHITSEEDBASEADDR, &reg_value, 1 );
        if( status == SX126X_STATUS_OK )
        {
            reg_value = ( uint8_t ) seed;
            status    = sx126x_write_register( context, SX126X_REG_WHITSEEDBASEADDR + 1, &reg_value, 1 );
        }
    }

    return status;
}

sx126x_status_t sx126x_config_tx_clamp( const void* context )
{
    sx126x_status_t status    = SX126X_STATUS_ERROR;
    uint8_t         reg_value = 0x00;

    status = sx126x_read_register( context, SX126X_REG_TX_CLAMP_CONFIG, &reg_value, 1 );

    if( status == SX126X_STATUS_OK )
    {
        reg_value |= SX126X_REG_TX_CLAMP_CONFIG_MASK;
        status = sx126x_write_register( context, SX126X_REG_TX_CLAMP_CONFIG, &reg_value, 1 );
    }

    return status;
}

sx126x_status_t sx126x_stop_rtc( const void* context )
{
    sx126x_status_t status    = SX126X_STATUS_ERROR;
    uint8_t         reg_value = 0;

    reg_value = 0;
    status    = sx126x_write_register( context, SX126X_REG_RTC_CTRL, &reg_value, 1 );

    if( status == SX126X_STATUS_OK )
    {
        status = sx126x_read_register( context, SX126X_REG_EVT_CLR, &reg_value, 1 );

        if( status == SX126X_STATUS_OK )
        {
            reg_value |= SX126X_REG_EVT_CLR_TIMEOUT_MASK;
            status = sx126x_write_register( context, SX126X_REG_EVT_CLR, &reg_value, 1 );
        }
    }

    return status;
}

sx126x_status_t sx126x_get_lora_params_from_header( const void* context, sx126x_lora_cr_t* cr, bool* crc_is_on )
{
    sx126x_status_t status = SX126X_STATUS_ERROR;
    uint8_t         buffer_cr;
    uint8_t         buffer_crc;

    status = sx126x_read_register( context, SX126X_REG_LR_HEADER_CR, &buffer_cr, 1 );

    if( status == SX126X_STATUS_OK )
    {
        status = sx126x_read_register( context, SX126X_REG_LR_HEADER_CRC, &buffer_crc, 1 );

        if( status == SX126X_STATUS_OK )
        {
            *cr = ( sx126x_lora_cr_t )( ( buffer_cr & SX126X_REG_LR_HEADER_CR_MASK ) >> SX126X_REG_LR_HEADER_CR_POS );
            *crc_is_on = ( ( buffer_crc & SX126X_REG_LR_HEADER_CRC_MASK ) != 0 ) ? true : false;
        }
    }

    return status;
}

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS DEFINITION --------------------------------------------
 */

static sx126x_status_t sx126x_tx_modulation_workaround( const void* context, sx126x_pkt_type_t pkt_type,
                                                        sx126x_lora_bw_t bw )
{
    sx126x_status_t status    = SX126X_STATUS_ERROR;
    uint8_t         reg_value = 0;

    status = sx126x_read_register( context, SX126X_REG_TX_MODULATION, &reg_value, 1 );

    if( status == SX126X_STATUS_OK )
    {
        if( pkt_type == SX126X_PKT_TYPE_LORA )
        {
            if( bw == SX126X_LORA_BW_500 )
            {
                reg_value &= ~( 1 << 2 );  // Bit 2 set to 0 if the LoRa BW = 500 kHz
            }
            else
            {
                reg_value |= ( 1 << 2 );  // Bit 2 set to 1 for any other LoRa BW
            }
        }
        else
        {
            reg_value |= ( 1 << 2 );  // Bit 2 set to 1 for any (G)FSK configuration
        }

        status = sx126x_write_register( context, SX126X_REG_TX_MODULATION, &reg_value, 1 );
    }
    return status;
}

static inline uint32_t sx126x_get_gfsk_crc_len_in_bytes( sx126x_gfsk_crc_types_t crc_type )
{
    switch( crc_type )
    {
    case SX126X_GFSK_CRC_OFF:
        return 0;
    case SX126X_GFSK_CRC_1_BYTE:
        return 1;
    case SX126X_GFSK_CRC_2_BYTES:
        return 2;
    case SX126X_GFSK_CRC_1_BYTE_INV:
        return 1;
    case SX126X_GFSK_CRC_2_BYTES_INV:
        return 2;
    }

    return 0;
}

/* --- EOF ------------------------------------------------------------------ */
