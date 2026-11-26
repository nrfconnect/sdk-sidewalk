/*
 * Copyright (c) 2025 Semtech Corporation. All rights reserved.
 * Copyright (c) 2026 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * LR20xx HAL for the Sidewalk PAL. Adapted from usp_zephyr
 * drivers/usp/lr20xx/lr20xx_hal.c (Clear BSD, Semtech): the original targets the
 * Zephyr device-driver model (dev->config / dev->data); this version reads the
 * cfg/data from the sidewalk-owned halo_drv_lr20xx_ctx_t instead. Zephyr
 * spi_dt_spec / gpio_dt_spec / k_uptime are kept as-is.
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/kernel.h>
#include <zephyr/types.h>

#include "lr20xx_hal.h"
#include "lr20xx_hal_context.h"
#include "halo_lr20xx_radio.h"
#include "sl_sidewalk_log_app.h"

#define LR20XX_HAL_WAIT_ON_BUSY_TIMEOUT_MSEC 2000
#define LR20XX_HAL_LR20_DUMMY_BYTE_LENGTH    2

static const struct lr20xx_hal_context_cfg_t* hal_cfg( const void* context )
{
    return ( ( const halo_drv_lr20xx_ctx_t* ) context )->cfg;
}

static struct lr20xx_hal_context_data_t* hal_data( const void* context )
{
    return &( ( halo_drv_lr20xx_ctx_t* ) context )->data;
}

static lr20xx_hal_status_t lr20xx_hal_wait_on_busy( const void* context )
{
    const struct lr20xx_hal_context_cfg_t* config = hal_cfg( context );
    uint32_t end       = k_uptime_get_32() + LR20XX_HAL_WAIT_ON_BUSY_TIMEOUT_MSEC;
    bool     timed_out = false;

    while( k_uptime_get_32() <= end )
    {
        if( gpio_pin_get_dt( &config->busy ) == 0 )
        {
            timed_out = true;
            break;
        }
    }

    if( !timed_out )
    {
        SL_SID_LOG_APP_ERROR( "lr20xx hal: busy timeout %dms", LR20XX_HAL_WAIT_ON_BUSY_TIMEOUT_MSEC );
        return LR20XX_HAL_STATUS_ERROR;
    }
    return LR20XX_HAL_STATUS_OK;
}

static lr20xx_hal_status_t lr20xx_hal_check_device_ready( const void* context )
{
    struct lr20xx_hal_context_data_t* data = hal_data( context );

    if( data->radio_status != RADIO_SLEEP )
    {
        return lr20xx_hal_wait_on_busy( context );
    }

    /* Busy is HIGH in sleep mode; wake the device with a small glitch on NSS. */
    const struct gpio_dt_spec* cs = &( hal_cfg( context )->spi.config.cs.gpio );
    gpio_pin_set_dt( cs, 1 );
    gpio_pin_set_dt( cs, 0 );
    if( lr20xx_hal_wait_on_busy( context ) != LR20XX_HAL_STATUS_OK )
    {
        return LR20XX_HAL_STATUS_ERROR;
    }
    data->radio_status = RADIO_AWAKE;
    return LR20XX_HAL_STATUS_OK;
}

lr20xx_hal_status_t lr20xx_hal_reset( const void* context )
{
    const struct lr20xx_hal_context_cfg_t* config = hal_cfg( context );
    struct lr20xx_hal_context_data_t*      data   = hal_data( context );

    /* TCXO variant: use a longer reset pulse (1 ms) than the usp XTAL default
     * (100 us) — the TCXO / power rail may need more time to stabilize. */
    gpio_pin_set_dt( &config->reset, 1 );
    k_sleep( K_MSEC( 1 ) );
    gpio_pin_set_dt( &config->reset, 0 );
    k_sleep( K_MSEC( 5 ) );

    data->radio_status = RADIO_AWAKE;
    return LR20XX_HAL_STATUS_OK;
}

lr20xx_hal_status_t lr20xx_hal_wakeup( const void* context )
{
    return lr20xx_hal_check_device_ready( context );
}

lr20xx_hal_status_t lr20xx_hal_write( const void* context, const uint8_t* command, const uint16_t command_length,
                                      const uint8_t* data, const uint16_t data_length )
{
    const struct lr20xx_hal_context_cfg_t* config   = hal_cfg( context );
    struct lr20xx_hal_context_data_t*      dev_data = hal_data( context );

    const struct spi_buf tx_buf[] = {
        { .buf = ( uint8_t* ) command, .len = command_length },
        { .buf = ( uint8_t* ) data,    .len = data_length },
    };
    const struct spi_buf_set tx = { .buffers = tx_buf, .count = ARRAY_SIZE( tx_buf ) };

    if( lr20xx_hal_check_device_ready( context ) != LR20XX_HAL_STATUS_OK )
    {
        return LR20XX_HAL_STATUS_ERROR;
    }
    if( spi_write_dt( &config->spi, &tx ) != 0 )
    {
        return LR20XX_HAL_STATUS_ERROR;
    }

    /* LR20XX_SYSTEM_SET_SLEEP_OC opcode 0x0127: in sleep mode busy is held high. */
    if( ( command[0] == 0x01 ) && ( command_length > 1 ) && ( command[1] == 0x27 ) )
    {
        dev_data->radio_status = RADIO_SLEEP;
        k_sleep( K_USEC( 500 ) );
    }
    return LR20XX_HAL_STATUS_OK;
}

lr20xx_hal_status_t lr20xx_hal_read( const void* context, const uint8_t* command, const uint16_t command_length,
                                     uint8_t* data, const uint16_t data_length )
{
    const struct lr20xx_hal_context_cfg_t* config = hal_cfg( context );

    const struct spi_buf tx_buf[] = {
        { .buf = ( uint8_t* ) command, .len = command_length },
    };
    const struct spi_buf_set tx = { .buffers = tx_buf, .count = ARRAY_SIZE( tx_buf ) };

    if( lr20xx_hal_check_device_ready( context ) != LR20XX_HAL_STATUS_OK )
    {
        return LR20XX_HAL_STATUS_ERROR;
    }
    if( spi_write_dt( &config->spi, &tx ) != 0 )
    {
        return LR20XX_HAL_STATUS_ERROR;
    }

    if( data_length > 0 )
    {
        uint8_t dummy[LR20XX_HAL_LR20_DUMMY_BYTE_LENGTH];
        const struct spi_buf rx_buf[] = {
            { .buf = dummy, .len = LR20XX_HAL_LR20_DUMMY_BYTE_LENGTH },
            { .buf = data,  .len = data_length },
        };
        const struct spi_buf_set rx = { .buffers = rx_buf, .count = ARRAY_SIZE( rx_buf ) };

        if( lr20xx_hal_check_device_ready( context ) != LR20XX_HAL_STATUS_OK )
        {
            return LR20XX_HAL_STATUS_ERROR;
        }
        if( spi_read_dt( &config->spi, &rx ) != 0 )
        {
            return LR20XX_HAL_STATUS_ERROR;
        }
    }
    return LR20XX_HAL_STATUS_OK;
}

lr20xx_hal_status_t lr20xx_hal_direct_read( const void* context, uint8_t* data, const uint16_t data_length )
{
    const struct lr20xx_hal_context_cfg_t* config = hal_cfg( context );

    if( lr20xx_hal_check_device_ready( context ) != LR20XX_HAL_STATUS_OK )
    {
        return LR20XX_HAL_STATUS_ERROR;
    }

    const struct spi_buf rx_buf[] = {
        { .buf = data, .len = data_length },
    };
    const struct spi_buf_set rx = { .buffers = rx_buf, .count = ARRAY_SIZE( rx_buf ) };

    return ( spi_read_dt( &config->spi, &rx ) == 0 ) ? LR20XX_HAL_STATUS_OK : LR20XX_HAL_STATUS_ERROR;
}

lr20xx_hal_status_t lr20xx_hal_direct_read_fifo( const void* context, const uint8_t* command,
                                                 const uint16_t command_length, uint8_t* data,
                                                 const uint16_t data_length )
{
    const struct lr20xx_hal_context_cfg_t* config = hal_cfg( context );
    uint8_t dummy[LR20XX_HAL_LR20_DUMMY_BYTE_LENGTH];

    const struct spi_buf tx_bufs[] = {
        { .buf = ( uint8_t* ) command, .len = command_length },
        { .buf = data,                 .len = data_length },
    };
    const struct spi_buf rx_bufs[] = {
        { .buf = dummy, .len = LR20XX_HAL_LR20_DUMMY_BYTE_LENGTH },
        { .buf = data,  .len = data_length },
    };
    const struct spi_buf_set tx_set = { .buffers = tx_bufs, .count = ARRAY_SIZE( tx_bufs ) };
    const struct spi_buf_set rx_set = { .buffers = rx_bufs, .count = ARRAY_SIZE( rx_bufs ) };

    if( lr20xx_hal_check_device_ready( context ) != LR20XX_HAL_STATUS_OK )
    {
        return LR20XX_HAL_STATUS_ERROR;
    }
    return ( spi_transceive_dt( &config->spi, &tx_set, &rx_set ) == 0 ) ? LR20XX_HAL_STATUS_OK
                                                                        : LR20XX_HAL_STATUS_ERROR;
}

/* --- EOF ------------------------------------------------------------------ */
