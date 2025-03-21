/*
 * Copyright (c) 2019-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * This file defines interface used by Semtech driver to perform platform specific
 * operations
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include <sid_pal_delay_ifc.h>
#include <sid_pal_critical_region_ifc.h>

#include <sx126x.h>
#include <sx126x_radio.h>

#ifdef MARS_SPI_BUS_WORKAROUND
#include "board_hal.h"
#ifndef BOARD_HAL_SPI_IAE_NSS_POLARITY
#define BOARD_HAL_SPI_IAE_NSS_POLARITY 0
#endif // BOARD_HAL_SPI_IAE_NSS_POLARITY
#endif // MARS_SPI_BUS_WORKAROUND

// Delay time when SX126x wakes up from sleep and goes to standby
#define SEMTECH_SLEEP_STATE_DELAY_US       550

static int32_t set_gpio_power(const halo_drv_semtech_ctx_t *drv_ctx,
                              sid_pal_gpio_direction_t dir)
{
    if (sid_pal_gpio_set_direction(drv_ctx->config->gpio_power, dir) != SID_ERROR_NONE) {
        return RADIO_ERROR_HARDWARE_ERROR;
    }

    return RADIO_ERROR_NONE;
}

static int32_t set_gpio_config(const halo_drv_semtech_ctx_t *drv_ctx,
                               sid_pal_gpio_input_t mode)
{
    if (sid_pal_gpio_input_mode(drv_ctx->config->gpio_int1, mode) != SID_ERROR_NONE) {
        return RADIO_ERROR_HARDWARE_ERROR;
    }

    if (sid_pal_gpio_input_mode(drv_ctx->config->gpio_radio_busy, mode) != SID_ERROR_NONE) {
        return RADIO_ERROR_HARDWARE_ERROR;
    }

    return RADIO_ERROR_NONE;
}

static int32_t set_radio_int(const halo_drv_semtech_ctx_t *drv_ctx, bool int_enable)
{
    if (int_enable == true) {
        sid_pal_gpio_irq_enable(drv_ctx->config->gpio_int1);
    } else {
        sid_pal_gpio_irq_disable(drv_ctx->config->gpio_int1);
    }

    return RADIO_ERROR_NONE;
}

static int32_t sx126x_wait_for_device_ready( const halo_drv_semtech_ctx_t *drv_ctx)
{
    int32_t err = RADIO_ERROR_NONE;

    if (drv_ctx != NULL) {
        if (drv_ctx->radio_state == SID_PAL_RADIO_SLEEP || drv_ctx->radio_state == SID_PAL_RADIO_RX_DC) {
            sx126x_wakeup(drv_ctx);
            if ((err = radio_sx126x_set_radio_mode(true, true)) == RADIO_ERROR_NONE) {
                err = sx126x_wait_on_busy();
            }
        }
    } else {
        err = RADIO_ERROR_INVALID_PARAMS;
    }
    return err;
}

static int32_t sx126x_hal_rdwr(const void* context, const uint8_t* command, const uint16_t command_length,
                                     uint8_t* data, const uint16_t data_length, bool read)
{
    int32_t err;

    do {
        if (context == NULL) {
            err = RADIO_ERROR_INVALID_PARAMS;
            break;
        }

        if ((err = sx126x_wait_on_busy()) != RADIO_ERROR_NONE) {
            break;
        }

        if ((err = sx126x_radio_bus_xfer(command, command_length, data, data_length,
              (read ? command_length : 0))) != RADIO_ERROR_NONE) {
            break;
        }

    } while (0);

    return err;
}

void set_gpio_cfg_awake(const halo_drv_semtech_ctx_t *drv_ctx)
{
    /*TODO: Is this needed ? */
    set_gpio_power(drv_ctx, SID_PAL_GPIO_DIRECTION_INPUT);
    set_gpio_config(drv_ctx, SID_PAL_GPIO_INPUT_CONNECT);
    set_radio_int(drv_ctx, true);
}

void set_gpio_cfg_sleep(const halo_drv_semtech_ctx_t *drv_ctx)
{
    set_radio_int(drv_ctx, false);
    set_gpio_config(drv_ctx, SID_PAL_GPIO_INPUT_DISCONNECT);
    /*TODO: Is this needed ?*/
    set_gpio_power(drv_ctx, SID_PAL_GPIO_DIRECTION_INPUT);
}

sx126x_hal_status_t sx126x_hal_reset(const void *ctx)
{
    const halo_drv_semtech_ctx_t *drv_ctx;
    int32_t err;

    do {
        if (NULL == ctx) {
            err = RADIO_ERROR_INVALID_PARAMS;
            break;
        }

        drv_ctx = (halo_drv_semtech_ctx_t *)ctx;

        sid_pal_delay_us(10*1000);
        err = RADIO_ERROR_HARDWARE_ERROR;
        if (sid_pal_gpio_set_direction(drv_ctx->config->gpio_power,
            SID_PAL_GPIO_DIRECTION_OUTPUT) != SID_ERROR_NONE) {
            break;
        }
        if (sid_pal_gpio_write(drv_ctx->config->gpio_power, 0) != SID_ERROR_NONE) {
            break;
        }

        sid_pal_delay_us(20*1000);
        if (sid_pal_gpio_pull_mode(drv_ctx->config->gpio_power,
            SID_PAL_GPIO_PULL_NONE) != SID_ERROR_NONE) {
            break;
        }
        sid_pal_delay_us(10*1000);
        err = RADIO_ERROR_NONE;
    } while(0);

    if (err != RADIO_ERROR_NONE) {
        return SX126X_HAL_STATUS_ERROR;
    }

    return SX126X_HAL_STATUS_OK;
}

sx126x_hal_status_t sx126x_hal_wakeup(const void *context)
{
    const halo_drv_semtech_ctx_t *drv_ctx = (halo_drv_semtech_ctx_t *)context;

    sid_pal_enter_critical_region();

    /* wake up the gpio driver */
    set_gpio_cfg_awake(drv_ctx);

    if (sid_pal_gpio_set_direction(drv_ctx->config->bus_selector.client_selector,
        SID_PAL_GPIO_DIRECTION_OUTPUT) != SID_ERROR_NONE) {
        return SX126X_HAL_STATUS_ERROR;
    }

#ifndef BOARD_HAL_SPI_IAE_NSS_POLARITY
    if (sid_pal_gpio_write(drv_ctx->config->bus_selector.client_selector, 0)
#else
    if (sid_pal_gpio_write(drv_ctx->config->bus_selector.client_selector, BOARD_HAL_SPI_IAE_NSS_POLARITY)
#endif
        != SID_ERROR_NONE) {
        return SX126X_HAL_STATUS_ERROR;
    }

    /* Wait for chip to be ready */
    if (sx126x_wait_on_busy() != RADIO_ERROR_NONE) {
        return SX126X_HAL_STATUS_ERROR;
    }

    /* pull up NSS pin again to allow transactions */
#ifndef BOARD_HAL_SPI_IAE_NSS_POLARITY
    if (sid_pal_gpio_write(drv_ctx->config->bus_selector.client_selector, 1)
#else
    if (sid_pal_gpio_write(drv_ctx->config->bus_selector.client_selector, !BOARD_HAL_SPI_IAE_NSS_POLARITY)
#endif
        != SID_ERROR_NONE) {
        return SX126X_HAL_STATUS_ERROR;
    }

    sid_pal_exit_critical_region();

    return SX126X_HAL_STATUS_OK;
}

sx126x_hal_status_t sx126x_hal_read(const void* context, const uint8_t* command, const uint16_t command_length,
                                     uint8_t* data, const uint16_t data_length)
{
    sx126x_status_t status = SX126X_STATUS_ERROR;

    do {
        if (context == NULL || command == NULL || data == NULL || command_length == 0 || data_length == 0) {
            break;
        }
        // If device is in sleep or rx dc state wake up
        const halo_drv_semtech_ctx_t *drv_ctx = (halo_drv_semtech_ctx_t *)context;
        if (sx126x_wait_for_device_ready(drv_ctx) != RADIO_ERROR_NONE) {
            break;
        }
        if (sx126x_hal_rdwr(context, command, command_length, data, data_length, true) != RADIO_ERROR_NONE) {
            break;
        }
        status = SX126X_STATUS_OK;
    } while(0);

    return status == SX126X_STATUS_OK ? SX126X_HAL_STATUS_OK : SX126X_HAL_STATUS_ERROR;
}

sx126x_hal_status_t sx126x_hal_write(const void* context, const uint8_t* command, const uint16_t command_length,
                                     const uint8_t* data, const uint16_t data_length)
{
    sx126x_status_t status = SX126X_STATUS_ERROR;

    do {
        //For write data can be null and data length 0
        if ( context == NULL || command == NULL || command_length == 0) {
            break;
        }

        // If device is in sleep or rx dc state wake up
        const halo_drv_semtech_ctx_t *drv_ctx = (halo_drv_semtech_ctx_t *)context;
        if (sx126x_wait_for_device_ready(drv_ctx) != RADIO_ERROR_NONE) {
            break;
        }

        if (sx126x_hal_rdwr(context, command, command_length, (uint8_t *)data, data_length, false) != RADIO_ERROR_NONE) {
            break;
        }
        status = SX126X_STATUS_OK;
    } while(0);

    return status == SX126X_STATUS_OK ? SX126X_HAL_STATUS_OK : SX126X_HAL_STATUS_ERROR;
}
