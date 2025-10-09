#ifndef SID_PAL_RADIO_MUTEX
/*
 * Copyright (c) 2019-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * This file defines interface used by Semtech driver to perform platform specific operations
 * This code was modified by Semtech
 */

/*
 * -----------------------------------------------------------------------------
 * --- DEPENDENCIES ------------------------------------------------------------
 */

#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "sid_pal_radio_ifc.h"
#include "sid_pal_delay_ifc.h"

#include "halo_lr11xx_radio.h"
#include "lr11xx_radio.h"
#include "lr11xx_hal.h"

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE MACROS-----------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE CONSTANTS -------------------------------------------------------
 */

#define SEMTECH_BOOTUP_DELAY_US 40000
#define STATUS_FIELD_OFFSET_BITS 1
#define STATUS_OK_MASK ( LR11XX_SYSTEM_CMD_STATUS_OK << STATUS_FIELD_OFFSET_BITS )
#define DEFAULT_WAKEUP_DELAY 98
// Delay time to allow for any external PA/FEM turn ON/OFF
#define SEMTECH_STDBY_STATE_DELAY_US 10
#define SEMTECH_MAX_WAIT_ON_BUSY_CNT_US 40000
#define SEMTECH_BOOTLOADER_MAX_WAIT_ON_BUSY_CNT_US 400000

#define SEM_TICKS_TO_WAIT   40

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

static sid_error_t lr11xx_wait_on_busy( const halo_drv_semtech_ctx_t* drv_ctx, unsigned max_wait_us );

//static inline bool cmd_allowed_during_scan( const uint8_t* command );

static inline bool is_scan_start_command( const uint8_t* command );

static lr11xx_hal_status_t lr11xx_hal_rdwr( const halo_drv_semtech_ctx_t* context, const uint8_t* command,
                                            const uint16_t command_length, uint8_t* data, const uint16_t data_length,
                                            bool read );

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC FUNCTIONS DEFINITION ---------------------------------------------
 */

lr11xx_hal_status_t lr11xx_hal_abort_blocking_cmd( const void* context )
{
  halo_drv_semtech_ctx_t* drv_ctx = ( halo_drv_semtech_ctx_t* ) context;
  const radio_lr11xx_device_config_t *config = drv_ctx->config;
  uint8_t command[4] = { 0 };
  uint8_t unused[4] = { 0 };

  int err = drv_ctx->bus_iface->xfer(drv_ctx->bus_iface, &config->bus_selector, command, unused, sizeof(command));
  if (err != SID_ERROR_NONE) {
    SL_SID_LOG_APP_ERROR("abort_blocking xfer");
    return LR11XX_HAL_STATUS_ERROR;
  }

  if (lr11xx_wait_on_busy(drv_ctx, 1000000) != SID_ERROR_NONE) {// microseconds 1,000,000
    SL_SID_LOG_APP_ERROR("abort_blocking wait on busy");
    return LR11XX_HAL_STATUS_ERROR;
  } else
	drv_ctx->aborted = true;

  return LR11XX_HAL_STATUS_OK;
}

lr11xx_hal_status_t lr11xx_hal_direct_read( const void* context, uint8_t* data, const uint16_t data_length, const char *caller )
{
    halo_drv_semtech_ctx_t* drv_ctx = ( halo_drv_semtech_ctx_t* ) context;
    assert( drv_ctx );
    assert( data );
    assert( data_length != 0 );
    assert( data_length <= drv_ctx->config->internal_buffer.size );

    const radio_lr11xx_device_config_t* config = drv_ctx->config;
    uint8_t*                            empty  = config->internal_buffer.p;
    memset( empty, 0, data_length );

    if (drv_ctx->sleeping) {
      lr11xx_hal_status_t err;
      SL_SID_LOG_APP_WARNING("direct_read self-wakeup");
      if ((err = lr11xx_hal_wakeup(drv_ctx)) != LR11XX_HAL_STATUS_OK) {
        SL_SID_LOG_APP_ERROR(BRED"fail wakeup %d"COLOR_RESET, err);
        return LR11XX_HAL_STATUS_ERROR;
      }
    }

    if( lr11xx_wait_on_busy( drv_ctx, SEMTECH_MAX_WAIT_ON_BUSY_CNT_US) != SID_ERROR_NONE )
    {
        SL_SID_LOG_APP_ERROR(BRED"direct_read %s wait_on_busy, last %04x"COLOR_RESET, caller, drv_ctx->last.command);
        return LR11XX_HAL_STATUS_ERROR;
    }

    int err = drv_ctx->bus_iface->xfer( drv_ctx->bus_iface, &config->bus_selector, empty, data, data_length );
    if (err != SID_ERROR_NONE) {
        SL_SID_LOG_APP_ERROR(BRED"direct_read xfer fail %d"COLOR_RESET, err);
    }

    if( !( drv_ctx->last.stat1 & STATUS_OK_MASK ) && drv_ctx->last.command )
    {
        SL_SID_LOG_APP_WARNING( "LR11xx: Command 0x%.4X failed; Stat1 0x%.2X Stat2 0x%.2X", drv_ctx->last.command, drv_ctx->last.stat1, drv_ctx->last.stat2);
    }
    drv_ctx->last.command = 0;  // No command  - only read. Chip will complain about it

#ifdef LOCAL_DEBUG
    SID_HAL_LOG_INFO( "-----------------------------" );
    SID_HAL_LOG_INFO( "Direct read" );
    SID_HAL_LOG_HEXDUMP_INFO( data, data_length );
#endif

    return ( err == SID_ERROR_NONE ) ? LR11XX_HAL_STATUS_OK : LR11XX_HAL_STATUS_ERROR;
}

lr11xx_hal_status_t lr11xx_hal_reset( const void* context )
{
    if( NULL == context )
    {
        return LR11XX_HAL_STATUS_ERROR;
    }

    const halo_drv_semtech_ctx_t* drv_ctx = ( halo_drv_semtech_ctx_t* ) context;

    if (!drv_ctx->config->gpios.power_cb) {
        if( sid_pal_gpio_set_direction( drv_ctx->config->gpios.power, SID_PAL_GPIO_DIRECTION_OUTPUT ) != SID_ERROR_NONE )
        {
            return LR11XX_HAL_STATUS_ERROR;
        }

        if( sid_pal_gpio_output_mode( drv_ctx->config->gpios.power, SID_PAL_GPIO_OUTPUT_PUSH_PULL ) != SID_ERROR_NONE )
        {
            return LR11XX_HAL_STATUS_ERROR;
        }
    }

    bool success = drv_ctx->config->gpios.power_cb
                       ? drv_ctx->config->gpios.power_cb(false, drv_ctx->config->gpios.arg)
                       : sid_pal_gpio_write(drv_ctx->config->gpios.power, 0) == SID_ERROR_NONE;
    if (!success) {
        return LR11XX_HAL_STATUS_ERROR;
    }

    sid_pal_delay_us( SEMTECH_BOOTUP_DELAY_US );

    success = drv_ctx->config->gpios.power_cb ? drv_ctx->config->gpios.power_cb(true, drv_ctx->config->gpios.arg)
                                              : sid_pal_gpio_write(drv_ctx->config->gpios.power, 1) == SID_ERROR_NONE;
    if (!success) {
        return LR11XX_HAL_STATUS_ERROR;
    }

    sid_pal_delay_us( SEMTECH_BOOTUP_DELAY_US );

    return LR11XX_HAL_STATUS_OK;
}

uint8_t _read_busy(const halo_drv_semtech_ctx_t* drv_ctx)
{
  uint8_t     is_radio_busy = 0;
  sid_error_t err = sid_pal_gpio_read( drv_ctx->config->gpios.radio_busy, &is_radio_busy );
  if (err != SID_ERROR_NONE)
    SL_SID_LOG_APP_ERROR(BRED"read busy pin fail"COLOR_RESET);

  return is_radio_busy;
}

uint8_t _read_int1(const halo_drv_semtech_ctx_t* drv_ctx)
{
  uint8_t     is_int1 = 0;
  sid_error_t err = sid_pal_gpio_read( drv_ctx->config->gpios.int1, &is_int1 );
  if (err != SID_ERROR_NONE)
    SL_SID_LOG_APP_ERROR(BRED"read busy pin fail"COLOR_RESET);

  return is_int1;
}

lr11xx_hal_status_t lr11xx_hal_wakeup( const void* context )
{
    halo_drv_semtech_ctx_t* drv_ctx = ( halo_drv_semtech_ctx_t* ) context;

    if( drv_ctx == NULL )
    {
        return LR11XX_HAL_STATUS_ERROR;
    }

    if (drv_ctx->radio_state != SID_PAL_RADIO_SLEEP && !drv_ctx->sleeping) {
      SL_SID_LOG_APP_WARNING(BYEL "!SID_PAL_RADIO_SLEEP and !sleeping"COLOR_RESET);
      return LR11XX_HAL_STATUS_OK;
    }

    if (!_read_busy(drv_ctx)) {
      SL_SID_LOG_APP_WARNING(BRED "hal_wakeup busy low"COLOR_RESET);
    }

    if (!drv_ctx->config->bus_selector.client_selector_cb) {
        if (sid_pal_gpio_set_direction(drv_ctx->config->bus_selector.client_selector, SID_PAL_GPIO_DIRECTION_OUTPUT)
            != SID_ERROR_NONE) {
            return LR11XX_HAL_STATUS_ERROR;
        }
    }

    if (drv_ctx->config->wakeup_delay_us != 0) {
      if (drv_ctx->config->wakeup_delay_us < 95) {
        SL_SID_LOG_APP_ERROR( "wakeup_delay_us too low, %u", drv_ctx->config->wakeup_delay_us);
      } else {
        SL_SID_LOG_APP_INFO( "wakeup_delay_us %u", drv_ctx->config->wakeup_delay_us);
      }
    }

    bool success = drv_ctx->config->bus_selector.client_selector_cb
                       ? drv_ctx->config->bus_selector.client_selector_cb(
                             &drv_ctx->config->bus_selector, SID_PAL_SERIAL_BUS_CLIENT_SELECT,
                             drv_ctx->config->bus_selector.client_selector_context)
                       : sid_pal_gpio_write(drv_ctx->config->bus_selector.client_selector, 0) == SID_ERROR_NONE;
    if (!success) {
        return LR11XX_HAL_STATUS_ERROR;
    }

    sid_pal_delay_us( drv_ctx->config->wakeup_delay_us ? drv_ctx->config->wakeup_delay_us : DEFAULT_WAKEUP_DELAY );

    /* pull up NSS pin again to allow transactions */
    success = drv_ctx->config->bus_selector.client_selector_cb
                  ? drv_ctx->config->bus_selector.client_selector_cb(
                        &drv_ctx->config->bus_selector, SID_PAL_SERIAL_BUS_CLIENT_DESELECT,
                        drv_ctx->config->bus_selector.client_selector_context)
                  : sid_pal_gpio_write(drv_ctx->config->bus_selector.client_selector, 1) == SID_ERROR_NONE;
    if (!success) {
        return LR11XX_HAL_STATUS_ERROR;
    }

    /* Wait for chip to be ready */
    if( lr11xx_wait_on_busy( drv_ctx, SEMTECH_MAX_WAIT_ON_BUSY_CNT_US ) != SID_ERROR_NONE )
    {
        return LR11XX_HAL_STATUS_ERROR;
    }

    drv_ctx->sleeping = false;

    return LR11XX_HAL_STATUS_OK;
}

lr11xx_hal_status_t lr11xx_hal_read( const void* context, const uint8_t* command, const uint16_t command_length,
                                     uint8_t* data, const uint16_t data_length )
{
    lr11xx_hal_status_t ret;
    if( context == NULL || command == NULL || data == NULL || command_length == 0 || data_length == 0 )
    {
        return LR11XX_HAL_STATUS_ERROR;
    }

    ret = lr11xx_hal_rdwr( context, command, command_length, data, data_length, true );
    return ret;
}

lr11xx_hal_status_t lr11xx_hal_write( const void* context, const uint8_t* command, const uint16_t command_length,
                                      const uint8_t* data, const uint16_t data_length )
{
    lr11xx_hal_status_t ret;
    /* For write data can be null and data length 0 */
    if( context == NULL || command == NULL || command_length == 0 )
    {
        SL_SID_LOG_APP_ERROR( "hal_write dropped ctx %p, cmd %p, cmd_len %u", context, command, command_length);
        return LR11XX_HAL_STATUS_ERROR;
    }

    ret = lr11xx_hal_rdwr( context, command, command_length, ( void* ) data, data_length, false );
    if (ret == LR11XX_HAL_STATUS_OK && is_scan_start_command(command)) {
      const halo_drv_semtech_ctx_t *cctx = context;
      if (cctx->radio_state != SID_PAL_RADIO_SCAN) {
        /* sid_pal_scan_mode() should have been called */
        SL_SID_LOG_APP_ERROR("scan start cmd while not in scan state");
      }
    }
    return ret;
}

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS DEFINITION --------------------------------------------
 */

static sid_error_t lr11xx_wait_on_busy( const halo_drv_semtech_ctx_t* drv_ctx, unsigned max_wait_us )
{
    assert( drv_ctx );

    uint8_t     is_radio_busy = 0;
    uint32_t    cnt           = 0;
    sid_error_t err           = SID_ERROR_NONE;

    while( cnt++ < max_wait_us )
    {
        err = sid_pal_gpio_read( drv_ctx->config->gpios.radio_busy, &is_radio_busy );
        if( ( err == SID_ERROR_NONE ) && !is_radio_busy )
        {
            break;
        }
        sid_pal_delay_us( SEMTECH_STDBY_STATE_DELAY_US );
    }

    if( cnt >= max_wait_us )
    {
        return SID_ERROR_BUSY;
    }
    return SID_ERROR_NONE;
}

static inline bool is_scan_start_command( const uint8_t* command )
{
    if( command[0] == 0x04 )
    {                             // gnss
        if( command[1] == 0x09 )  // autonomous start
            return true;
        else if( command[1] == 0x0a )  // assisted start
            return true;
        else
            return false;
    }
    else if( command[0] == 0x03 )
    {                             // wifi
        if( command[1] == 0x00 )  // scan start
            return true;
        else
            return false;
    }
    else
        return false;
}

#ifdef BUFFER_USAGE_CHECK
static inline unsigned get_buf_max_usage( halo_drv_semtech_ctx_t* drv_ctx )
{
    unsigned n;
    for( n = drv_ctx->config->internal_buffer.size; n > 0; )
    {
        if( drv_ctx->config->internal_buffer.p[--n] != 0xaa ) return n;
    }
    return n;
}
#endif /* BUFFER_USAGE_CHECK */

static lr11xx_hal_status_t lr11xx_hal_rdwr( const halo_drv_semtech_ctx_t* context, const uint8_t* command,
                                            const uint16_t command_length, uint8_t* data, const uint16_t data_length,
                                            bool read )
{
    unsigned                max_wait_us;
    halo_drv_semtech_ctx_t* drv_ctx = ( halo_drv_semtech_ctx_t* ) context;
    assert( drv_ctx );
	lr11xx_hal_status_t ret = LR11XX_HAL_STATUS_OK;

    if (context->sleeping) {
      lr11xx_hal_status_t err;
      if ((err = lr11xx_hal_wakeup(context)) != LR11XX_HAL_STATUS_OK) {
        SL_SID_LOG_APP_ERROR(BRED"fail wakeup %d"COLOR_RESET, err);
      }
    }

    max_wait_us = SEMTECH_MAX_WAIT_ON_BUSY_CNT_US;
    if( command[0] == 0x80 ) /* if flash write in bootloader */
        max_wait_us = SEMTECH_BOOTLOADER_MAX_WAIT_ON_BUSY_CNT_US;
    if( lr11xx_wait_on_busy( drv_ctx, max_wait_us ) != SID_ERROR_NONE )
    {
        SL_SID_LOG_APP_ERROR( BYEL "rdwr start wait_on_busy 0x%02X%02X, waited %u"COLOR_RESET, command[0], command[1], max_wait_us );
        return LR11XX_HAL_STATUS_ERROR;
    }

#ifdef LOCAL_DEBUG
    SID_HAL_LOG_INFO( "-----------------------------" );
    SID_HAL_LOG_INFO( ( read ) ? "Command (read):" : "Command (write):" );
    SID_HAL_LOG_HEXDUMP_INFO( command, command_length );
    if( !read && data_length > 0 )
    {
        SID_HAL_LOG_INFO( "Data:" );
        SID_HAL_LOG_HEXDUMP_INFO( data, data_length );
    }
#endif

    size_t   size = command_length;
    uint8_t* buff = drv_ctx->config->internal_buffer.p;
    memcpy( buff, command, command_length );
    if( !read && data_length > 0 )
    {
        size += data_length;
        memcpy( &buff[command_length], data, data_length );
    }

    int err = drv_ctx->bus_iface->xfer( drv_ctx->bus_iface, &drv_ctx->config->bus_selector, buff, buff, size );
    if( err != SID_ERROR_NONE )
    {
        SL_SID_LOG_APP_ERROR( "rdwr write xfer fail" );
        return LR11XX_HAL_STATUS_ERROR;
    }

    drv_ctx->last.stat1 = buff[0];
    if( !( drv_ctx->last.stat1 & STATUS_OK_MASK ) && drv_ctx->last.command )
    {
		bool ok = false;
		if (drv_ctx->aborted) {
			if ((drv_ctx->last.stat1 & 0x0e) == (LR11XX_SYSTEM_CMD_STATUS_CMD_ABORT << STATUS_FIELD_OFFSET_BITS)) {
				SL_SID_LOG_APP_INFO("abort response: CMD_ABORT");
				ok = true;
			} else {
				SL_SID_LOG_APP_INFO("abort response, stat1 %x", drv_ctx->last.stat1);
			}
			drv_ctx->aborted = false;
		}
		if (!ok) {
			/* section 3.4.2: bit0 = interrupt status */
			SL_SID_LOG_APP_WARNING( "during 0x%02X%02X, Command 0x%.4X failed; Stat1 0x%.2X int1:%d Stat2 0x%.2X ", command[0], command[1],
							 drv_ctx->last.command, drv_ctx->last.stat1, _read_int1(drv_ctx), drv_ctx->last.stat2);
			drv_ctx->last.failedCommand = drv_ctx->last.command;
			ret = LR11XX_HAL_STATUS_ERROR;
		}
    } else if (drv_ctx->aborted) {
		SL_SID_LOG_APP_INFO("abort response: OK");
		drv_ctx->aborted = false;
	}

    drv_ctx->last.stat2 = buff[1];
    drv_ctx->last.command = ( command[0] << 8 ) | command[1];
    if (drv_ctx->last.command == 0x11b) {
      drv_ctx->sleeping = true;
    }

#ifdef LOCAL_DEBUG
    SID_HAL_LOG_INFO( "Read back" );
    SID_HAL_LOG_HEXDUMP_INFO( empty, size );
#endif

    if( !read )
    {
        return ret;
    }

    if( lr11xx_wait_on_busy( drv_ctx, SEMTECH_MAX_WAIT_ON_BUSY_CNT_US ) != SID_ERROR_NONE )
    {
        SL_SID_LOG_APP_ERROR( "rdwr read busy" );
        return LR11XX_HAL_STATUS_ERROR;
    }

    size = data_length + 1;
    buff = drv_ctx->config->internal_buffer.p;
    memset( buff, 0, size );
    err = drv_ctx->bus_iface->xfer( drv_ctx->bus_iface, &drv_ctx->config->bus_selector, buff, buff, size );
    if( err != SID_ERROR_NONE )
    {
        SL_SID_LOG_APP_ERROR( "rdwr read xfer fail" );
        return LR11XX_HAL_STATUS_ERROR;
    }

    drv_ctx->last.stat1 = buff[0];
    if( !( drv_ctx->last.stat1 & STATUS_OK_MASK ) && drv_ctx->last.command )
    {
        SL_SID_LOG_APP_WARNING( "Command rsp 0x%.4X failed; Stat1 0x%.2X Stat2 0x%.2X", drv_ctx->last.command, drv_ctx->last.stat1, drv_ctx->last.stat2);
    }

#ifdef LOCAL_DEBUG
    SID_HAL_LOG_INFO( "Data" );
    SID_HAL_LOG_HEXDUMP_INFO( buff, size );
#endif

    memcpy( data, &buff[1], data_length );

    return ret;
}

#endif /* !SID_PAL_RADIO_MUTEX */
/* --- EOF ------------------------------------------------------------------ */
