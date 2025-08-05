/*
 * Copyright (c) 2019-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * This file defines interface used by Semtech driver to perform platform specific
 * operations
 */

#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include <sid_pal_delay_ifc.h>
#include <sid_pal_serial_bus_ifc.h>

#include "halo_lr1110_radio.h"
#include "lr1110_radio.h"
#include "lr1110_hal.h"

// #define LOCAL_DEBUG 1
#include <sid_pal_log_ifc.h>

#define SEMTECH_BOOTUP_DELAY_US            185000
#define STATUS_FIELD_OFFSET_BITS           1
#define STATUS_OK_MASK                     (LR1110_SYSTEM_CMD_STATUS_OK << STATUS_FIELD_OFFSET_BITS)
#define DEFAULT_WAKEUP_DELAY               5
// Delay time to allow for any external PA/FEM turn ON/OFF
#define SEMTECH_STDBY_STATE_DELAY_US       10
#define SEMTECH_MAX_WAIT_ON_BUSY_CNT_US    40000

static sid_error_t lr1110_wait_on_busy(const halo_drv_semtech_ctx_t *drv_ctx)
{
    assert(drv_ctx);

    uint8_t is_radio_busy = 0;
    uint16_t cnt = 0;
    sid_error_t err = SID_ERROR_NONE;

    while (cnt++ < SEMTECH_MAX_WAIT_ON_BUSY_CNT_US) {
        err = sid_pal_gpio_read(drv_ctx->config->gpios.radio_busy, &is_radio_busy);
        if ((err == SID_ERROR_NONE) && !is_radio_busy) {
           break;
        }
        sid_pal_delay_us(SEMTECH_STDBY_STATE_DELAY_US);
    }

    if (cnt >= SEMTECH_MAX_WAIT_ON_BUSY_CNT_US) {
        return SID_ERROR_BUSY;
    }
    return SID_ERROR_NONE;
}

static lr1110_hal_status_t lr1110_hal_rdwr(const halo_drv_semtech_ctx_t* cctx,
                                           const uint8_t* command,
                                           const uint16_t command_length,
                                           uint8_t* data,
                                           const uint16_t data_length,
                                           bool read)
{
    halo_drv_semtech_ctx_t* ctx = (halo_drv_semtech_ctx_t*) cctx;
    assert(ctx);

    if (lr1110_wait_on_busy(ctx) != SID_ERROR_NONE) {
        return LR1110_HAL_STATUS_ERROR;
    }

#ifdef LOCAL_DEBUG
    SID_HAL_LOG_INFO("-----------------------------");
    SID_HAL_LOG_INFO((read)? "Command (read):" : "Command (write):");
    SID_HAL_LOG_HEXDUMP_INFO(command, command_length);
    if (!read && data_length > 0) {
        SID_HAL_LOG_INFO("Data:");
        SID_HAL_LOG_HEXDUMP_INFO(data, data_length);
    }
#endif

    size_t size = command_length;
    uint8_t *buff = (void*)command;
    if (!read && data_length > 0) {
        buff  = ctx->config->internal_buffer.p;
        size += data_length;
        memcpy(buff                 , command, command_length);
        memcpy(&buff[command_length], data,    data_length);
    }

    uint8_t *empty = &ctx->config->internal_buffer.p[size];
    int err = ctx->bus_iface->xfer(ctx->bus_iface, &ctx->config->bus_selector, buff, empty, size);
    if (err != SID_ERROR_NONE) {
        return LR1110_HAL_STATUS_ERROR;
    }

    ctx->last.stat1 = empty[0];
    ctx->last.stat2 = empty[1];
    if (!(ctx->last.stat1 & STATUS_OK_MASK) && ctx->last.command) {
        SID_HAL_LOG_WARNING("LR1110: Command 0x%.4X failed; Stat1 0x%.2X", ctx->last.command, ctx->last.stat1);
    }

    ctx->last.command = (command[0] << 8) | command[1];

#ifdef LOCAL_DEBUG
    SID_HAL_LOG_INFO("Read back");
    SID_HAL_LOG_HEXDUMP_INFO(empty, size);
#endif

    if (!read) {
        return LR1110_HAL_STATUS_OK;
    }

    if (lr1110_wait_on_busy(ctx) != SID_ERROR_NONE) {
        return LR1110_HAL_STATUS_ERROR;
    }

    size = data_length + 1;
    buff = ctx->config->internal_buffer.p;
    empty = &buff[size];
    memset(buff, 0, size*2);
    err = ctx->bus_iface->xfer(ctx->bus_iface, &ctx->config->bus_selector, empty, buff, size);
    if (err != SID_ERROR_NONE) {
        return LR1110_HAL_STATUS_ERROR;
    }

    ctx->last.stat1 = buff[0];
    if (!(ctx->last.stat1 & STATUS_OK_MASK) && ctx->last.command) {
        SID_HAL_LOG_WARNING("LR1110: Command rsp 0x%.4X failed; Stat1 0x%.2X", ctx->last.command, ctx->last.stat1);
    }

#ifdef LOCAL_DEBUG
    SID_HAL_LOG_INFO("Data");
    SID_HAL_LOG_HEXDUMP_INFO(buff, size);
#endif

    memcpy(data, &buff[1], data_length);

    return LR1110_HAL_STATUS_OK;
}

lr1110_hal_status_t lr1110_hal_direct_read( const void* context,
                                            uint8_t* data,
                                            const uint16_t data_length )
{
    halo_drv_semtech_ctx_t* ctx = (halo_drv_semtech_ctx_t*) context;
    assert(ctx);
    assert(data);
    assert(data_length != 0);
    assert(data_length <= ctx->config->internal_buffer.size);

    const radio_lr1110_device_config_t *config = ctx->config;
    uint8_t *empty = config->internal_buffer.p;
    memset(empty, 0, data_length);

    if (lr1110_wait_on_busy(ctx) != SID_ERROR_NONE) {
        return LR1110_HAL_STATUS_ERROR;
    }

    int err = ctx->bus_iface->xfer(ctx->bus_iface, &config->bus_selector, empty, data, data_length);

    if (!(ctx->last.stat1 & STATUS_OK_MASK) && ctx->last.command) {
        SID_HAL_LOG_WARNING("LR1110: Command 0x%.4X failed; Stat1 0x%.2X", ctx->last.command, ctx->last.stat1);
    }
    ctx->last.command = 0; // No command  - only read. Chip will complain about it

#ifdef LOCAL_DEBUG
    SID_HAL_LOG_INFO("-----------------------------");
    SID_HAL_LOG_INFO("Direct read");
    SID_HAL_LOG_HEXDUMP_INFO(data, data_length);
#endif

    return (err == SID_ERROR_NONE)? LR1110_HAL_STATUS_OK : LR1110_HAL_STATUS_ERROR;
}

lr1110_hal_status_t lr1110_hal_reset(const void *ctx)
{
    if (NULL == ctx) {
        return LR1110_HAL_STATUS_ERROR;
    }

    const halo_drv_semtech_ctx_t *drv_ctx = (halo_drv_semtech_ctx_t *)ctx;

    if (!drv_ctx->config->gpios.power_cb) {
        if (sid_pal_gpio_set_direction(drv_ctx->config->gpios.power, SID_PAL_GPIO_DIRECTION_OUTPUT) != SID_ERROR_NONE) {
            return LR1110_HAL_STATUS_ERROR;
        }

        if (sid_pal_gpio_output_mode(drv_ctx->config->gpios.power, SID_PAL_GPIO_OUTPUT_PUSH_PULL) != SID_ERROR_NONE) {
            return LR1110_HAL_STATUS_ERROR;
        }
    }

    bool success = drv_ctx->config->gpios.power_cb
                       ? drv_ctx->config->gpios.power_cb(false, drv_ctx->config->gpios.arg)
                       : sid_pal_gpio_write(drv_ctx->config->gpios.power, 0) == SID_ERROR_NONE;
    if (!success) {
        return LR1110_HAL_STATUS_ERROR;
    }

    sid_pal_delay_us(SEMTECH_BOOTUP_DELAY_US);

    success = drv_ctx->config->gpios.power_cb ? drv_ctx->config->gpios.power_cb(true, drv_ctx->config->gpios.arg)
                                              : sid_pal_gpio_write(drv_ctx->config->gpios.power, 1) == SID_ERROR_NONE;
    if (!success) {
        return LR1110_HAL_STATUS_ERROR;
    }

    sid_pal_delay_us(SEMTECH_BOOTUP_DELAY_US);

    return LR1110_HAL_STATUS_OK;
}

lr1110_hal_status_t lr1110_hal_wakeup(const void *context)
{
    halo_drv_semtech_ctx_t *drv_ctx = (halo_drv_semtech_ctx_t *)context;

    if (drv_ctx == NULL) {
        return LR1110_HAL_STATUS_ERROR;
    }

    if (drv_ctx->radio_state != SID_PAL_RADIO_SLEEP) {
        return LR1110_HAL_STATUS_OK;
    }

    if (!drv_ctx->config->bus_selector.client_selector_cb) {
        if (sid_pal_gpio_set_direction(drv_ctx->config->bus_selector.client_selector, SID_PAL_GPIO_DIRECTION_OUTPUT)
            != SID_ERROR_NONE) {
            return LR1110_HAL_STATUS_ERROR;
        }
    }

    bool success = drv_ctx->config->bus_selector.client_selector_cb
                       ? drv_ctx->config->bus_selector.client_selector_cb(
                             &drv_ctx->config->bus_selector, SID_PAL_SERIAL_BUS_CLIENT_SELECT,
                             drv_ctx->config->bus_selector.client_selector_context)
                       : sid_pal_gpio_write(drv_ctx->config->bus_selector.client_selector, 0) == SID_ERROR_NONE;
    if (!success) {
        return LR1110_HAL_STATUS_ERROR;
    }

    sid_pal_delay_us(drv_ctx->config->wakeup_delay_us? drv_ctx->config->wakeup_delay_us : DEFAULT_WAKEUP_DELAY);

    /* pull up NSS pin again to allow transactions */
    success = drv_ctx->config->bus_selector.client_selector_cb
                  ? drv_ctx->config->bus_selector.client_selector_cb(
                        &drv_ctx->config->bus_selector, SID_PAL_SERIAL_BUS_CLIENT_DESELECT,
                        drv_ctx->config->bus_selector.client_selector_context)
                  : sid_pal_gpio_write(drv_ctx->config->bus_selector.client_selector, 1) == SID_ERROR_NONE;
    if (!success) {
        return LR1110_HAL_STATUS_ERROR;
    }

    /* Wait for chip to be ready */
    if (lr1110_wait_on_busy(drv_ctx) != SID_ERROR_NONE) {
        return LR1110_HAL_STATUS_ERROR;
    }

    return LR1110_HAL_STATUS_OK;
}

lr1110_hal_status_t lr1110_hal_read(const void* context, const uint8_t* command, const uint16_t command_length,
                                     uint8_t* data, const uint16_t data_length)
{
    if ( context == NULL || command == NULL || data == NULL || command_length == 0 || data_length == 0) {
        return LR1110_HAL_STATUS_ERROR;
    }

    return lr1110_hal_rdwr(context, command, command_length, data, data_length, true);
}

lr1110_hal_status_t lr1110_hal_write(const void* context, const uint8_t* command, const uint16_t command_length,
                                     const uint8_t* data, const uint16_t data_length)
{
    /* For write data can be null and data length 0 */
    if ( context == NULL || command == NULL || command_length == 0) {
        return LR1110_HAL_STATUS_ERROR;
    }

    return lr1110_hal_rdwr(context, command, command_length, (void*)data, data_length, false);
}
