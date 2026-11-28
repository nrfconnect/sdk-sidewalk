/*
 * Copyright (c) 2026 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Private driver context for the LR20xx PAL. Embeds the usp_zephyr HAL
 * cfg pointer + data inline so the HAL reads ctx->cfg / &ctx->data, plus
 * sidewalk callbacks/state.
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef HALO_LR20XX_RADIO_H
#define HALO_LR20XX_RADIO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lr20xx_config.h"
#include "lr20xx_hal_context.h"
#include "lr20xx_system.h"
#include "ral.h"
#include "ral_lr20xx.h"
#include "sl_sidewalk_log_app.h"

#include <sid_pal_radio_ifc.h>
#include <zephyr/drivers/gpio.h>
#include <stdint.h>
#include <stdbool.h>

/* The HAL reads cfg from ctx->cfg and data from &ctx->data (see lr20xx_hal.c). */
typedef struct halo_drv_lr20xx_ctx_s
{
    const radio_lr20xx_device_config_t*     config;   /* sidewalk config (wraps hal_cfg) */
    const struct lr20xx_hal_context_cfg_t*  cfg;      /* = &config->hal_cfg, for HAL */
    struct lr20xx_hal_context_data_t        data;     /* HAL runtime state (radio_status) */

    sid_pal_radio_modem_mode_t   modem;
    sid_pal_radio_rx_packet_t*   radio_rx_packet;
    sid_pal_radio_event_notify_t report_radio_event;
    uint8_t                      radio_state;
    sid_pal_radio_irq_handler_t  irq_handler;

    bool      sleeping;
    uint32_t  radio_freq_hz;
    sid_pal_radio_irq_mask_t irq_mask_cache;

    /* Runtime regional params (populated by sid_pal_radio_set_region during
     * init, copied from config->regional_config.reg_param_table). get_max_tx_power
     * / get_ant_dbi / get_cca_level_adjust read from this, mirroring lr11xx. */
    radio_lr20xx_regional_param_t regional_radio_param;

    lr20xx_system_version_t ver;

    /* RAL instance used by the ral_* adapters. */
    ral_t ral;

    /* nRF-side GPIO interrupt callback for the LR2021 IRQ DIO. The chip
     * asserts DIO8 on IRQ; this fires the ISR → drv_ctx.irq_handler (stack
     * bottom-half) → sid_pal_radio_irq_process from thread context. */
    struct gpio_callback dio_cb;
    const struct gpio_dt_spec* dio_irq_gpio;  /* points into cfg->dios_config[] */

    struct
    {
        sid_pal_radio_lora_packet_params_t     lora_pkt_params;
        sid_pal_radio_lora_modulation_params_t lora_mod_params;
        sid_pal_radio_fsk_packet_params_t      fsk_pkt_params;
        sid_pal_radio_fsk_modulation_params_t  fsk_mod_params;
        sid_pal_radio_fsk_cad_params_t         fsk_cad_params;
    } settings_cache;
} halo_drv_lr20xx_ctx_t;

/* LR2021 expected system version (lr20xx_system.h). */
#define LR20XX_LR2021_VERSION_MAJOR 0x01
#define LR20XX_LR2021_VERSION_MINOR 0x18

halo_drv_lr20xx_ctx_t* lr20xx_get_drv_ctx( void );

#ifdef __cplusplus
}
#endif

#endif /* HALO_LR20XX_RADIO_H */
