/*
 * Copyright (c) 2019-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * This file has private functions needed by the driver
 */

#ifndef SX126X_RADIO_H
#define SX126X_RADIO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#include <sx126x_config.h>
#include <sid_pal_radio_ifc.h>
#include <sid_pal_gpio_ifc.h>

#include <sx126x.h>
#include <sx126x_hal.h>
#include <sx126x_halo.h>
#include <sx126x_regs.h>
#include <sx126x_timings.h>

/*
 * @brief RADIO FSK status enumeration definition
 */
typedef enum {
    RADIO_FSK_RX_DONE_STATUS_OK                  = 0,
    RADIO_FSK_RX_DONE_STATUS_INVALID_PARAMETER   = 1,
    RADIO_FSK_RX_DONE_STATUS_INVALID_LENGTH      = 2,
    RADIO_FSK_RX_DONE_STATUS_BAD_CRC             = 3,
    RADIO_FSK_RX_DONE_STATUS_TIMEOUT             = 4,
    RADIO_FSK_RX_DONE_STATUS_UNKNOWN_ERROR       = 5,
    RADIO_FSK_RX_DONE_STATUS_SW_MARK_NOT_PRESENT = 6,
} radio_fsk_rx_done_status_t;

typedef struct {
    const radio_sx126x_device_config_t           *config;
    const struct sid_pal_serial_bus_iface        *bus_iface;

    sid_pal_radio_modem_mode_t                   modem;
    sid_pal_radio_rx_packet_t                    *radio_rx_packet;
    sid_pal_radio_event_notify_t                 report_radio_event;
    uint8_t                                      radio_state;
    sid_pal_radio_irq_handler_t                  irq_handler;

    sid_pal_radio_cad_param_exit_mode_t          cad_exit_mode;
    radio_sx126x_pa_cfg_t                        pa_cfg;
#if CONFIG_SIDEWALK_BUILD_DIAGNOSTICS
    bool                                         pa_cfg_configured;
#endif

    uint16_t                                     irq_mask;
    uint16_t                                     trim;
    uint32_t                                     radio_freq_hz;

    struct {
        sid_pal_radio_fsk_cad_params_t           fsk_cad_params;
    }                                            settings_cache;
    radio_sx126x_regional_param_t                regional_radio_param;
} halo_drv_semtech_ctx_t;

/* enum for calibration bands in semtech radio */
typedef enum {
    SX126X_BAND_900M,
    SX126X_BAND_850M,
    SX126X_BAND_770M,
    SX126X_BAND_460M,
    SX126X_BAND_430M,
    SX126X_BAND_INVALID
} sx126x_freq_cal_band;

#define SX1261_MIN_TX_POWER                         -17
#define SX1261_MAX_TX_POWER                         14

#define SX1262_MIN_TX_POWER                         -9
#define SX1262_MAX_TX_POWER                         22

#define SEMTECH_TUS_IN_SEC                          64000
#define SEMTECH_TUS_IN_MSEC                         64
#define US_IN_SEC                                   (1000000UL)
#define US_IN_MSEC                                  (1000UL)
#define US_TO_SEMTEC_TICKS(X)                       (((X) * SEMTECH_TUS_IN_MSEC)/US_IN_MSEC)
#define SX126X_US_TO_SYMBOLS(time_in_us, bit_rate)  ((uint64_t)((uint64_t)time_in_us * (uint64_t)bit_rate)/US_IN_SEC)
#define SX126X_TUS_TO_US(X)                         ((X) * (US_IN_SEC / SEMTECH_TUS_IN_SEC))

/*!
 * @brief return the calibration band of the given frequency
 * @param [in] freqquency in hz
 * @return calibration band for the frequency
 */
static inline sx126x_freq_cal_band sx126x_get_freq_band (uint32_t freq_hz)
{
    if (freq_hz > 900000000) {
        return SX126X_BAND_900M;
    } else if (freq_hz > 850000000) {
        return SX126X_BAND_850M;
    } else if (freq_hz > 770000000) {
        return SX126X_BAND_770M;
    } else if (freq_hz > 460000000) {
        return SX126X_BAND_460M;
    } else if (freq_hz > 430000000) {
        return SX126X_BAND_430M;
    } else {
        return SX126X_BAND_INVALID;
    }
}

const halo_drv_semtech_ctx_t* sx126x_get_drv_ctx(void);

int32_t sx126x_wait_on_busy(void);

void set_gpio_cfg_awake(const halo_drv_semtech_ctx_t *drv_ctx);

void set_gpio_cfg_sleep(const halo_drv_semtech_ctx_t *drv_ctx);

int32_t sx126x_radio_bus_xfer(const uint8_t *cmd_buffer, const uint16_t cmd_buffer_size, uint8_t *buffer,
                              const uint16_t size, uint8_t read_offset);

int32_t radio_sx126x_set_radio_mode(bool rf_en, bool tx_en);

int32_t radio_lora_process_rx_done(halo_drv_semtech_ctx_t *drv_ctx);

int32_t radio_fsk_process_sync_word_detected(halo_drv_semtech_ctx_t *drv_ctx);

int32_t radio_fsk_process_rx_done(halo_drv_semtech_ctx_t *drv_ctx, radio_fsk_rx_done_status_t *rx_done_status);

void set_lora_exit_mode(sid_pal_radio_cad_param_exit_mode_t cad_exit_mode);

int32_t set_radio_sx126x_trim_cap_val(uint16_t trim);

int32_t get_radio_sx126x_pa_config(radio_sx126x_pa_cfg_t *cfg);

#ifdef __cplusplus
}
#endif

#endif //SX126X_RADIO_H
