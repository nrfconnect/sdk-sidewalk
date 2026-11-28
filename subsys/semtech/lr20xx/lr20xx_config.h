/*
 * Copyright (c) 2026 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * LR20xx device config for the Sidewalk PAL. The HAL cfg/data come from
 * usp_zephyr's lr20xx_hal_context.h (spi_dt_spec / gpio_dt_spec form). The
 * sidewalk PAL owns the context (halo_drv_lr20xx_ctx_t) embedding cfg/data plus
 * sidewalk callbacks.
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef LR20XX_CONFIG_H
#define LR20XX_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lr20xx_hal_context.h"   /* lr20xx_hal_context_cfg_t / _data_t */
#include "lr20xx_system_types.h"

#include <sid_pal_radio_ifc.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#define HALO_GPIO_NOT_CONNECTED 128

#define RADIO_REGION_NA   SID_PAL_RADIO_RC_NA
#define RADIO_REGION_EU   SID_PAL_RADIO_RC_EU
#define RADIO_REGION_NONE SID_PAL_RADIO_RC_NONE

/* Regional parameters. max_tx_power / cca_level_adjust carry board-measured
 * values (defaults pending). */
typedef struct radio_lr20xx_regional_param
{
    uint8_t param_region;
    int8_t  max_tx_power[SID_PAL_RADIO_DATA_RATE_MAX_NUM];
    int8_t  cca_level_adjust[SID_PAL_RADIO_DATA_RATE_MAX_NUM];
    int16_t ant_dbi;
} radio_lr20xx_regional_param_t;

typedef struct radio_lr20xx_regional_config
{
    uint8_t                                radio_region;
    uint8_t                                reg_param_table_size;
    const radio_lr20xx_regional_param_t*   reg_param_table;
} radio_lr20xx_regional_config_t;

/*
 * Sidewalk-side device config. Wraps the usp_zephyr HAL context cfg plus the
 * regional/state data the PAL needs. Built from devicetree by
 * app_subGHz_config_lr20xx.c.
 */
typedef struct radio_lr20xx_device_config
{
    struct lr20xx_hal_context_cfg_t   hal_cfg;       /* SPI/reset/busy/reg-mode/lf-clk/DIOs/PA tables */
    sid_pal_radio_state_transition_timings_t state_timings;
    radio_lr20xx_regional_config_t    regional_config;
} radio_lr20xx_device_config_t;

/** @brief Set Semtech LR20xx radio config parameters.
 *  @param pointer to lr20xx radio device config
 */
void set_radio_lr20xx_device_config( const radio_lr20xx_device_config_t* cfg );

#ifdef __cplusplus
}
#endif

#endif /* LR20XX_CONFIG_H */
