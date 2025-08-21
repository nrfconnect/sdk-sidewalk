/*
 * Copyright (c) 2019-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * This file is exposed to each procuct configuration to customize the driver behavior
 */

#ifndef LR1110_CONFIG_H
#define LR1110_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lr1110_gnss.h"
#include "lr1110_radio_types.h"
#include "lr1110_system_types.h"
#include "lr1110_wifi.h"

#include <sid_time_types.h>
#include <semtech_radio_ifc.h>
#include <sid_pal_radio_ifc.h>
#include <sid_pal_serial_bus_ifc.h>

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <time.h>

#define HALO_GPIO_NOT_CONNECTED                 128

#define RADIO_REGION_NA SID_PAL_RADIO_RC_NA
#define RADIO_REGION_EU SID_PAL_RADIO_RC_EU
#define RADIO_REGION_NONE SID_PAL_RADIO_RC_NONE

typedef struct radio_lr1110_pa_cfg {
    lr1110_radio_pa_cfg_t    pa_cfg;
    lr1110_radio_ramp_time_t ramp_time;
    int8_t                   tx_power_in_dbm;
    bool                     enable_ext_pa;
} radio_lr1110_pa_cfg_t;

typedef struct radio_lr1110_regional_param {
    uint8_t param_region;
    int8_t max_tx_power[SID_PAL_RADIO_DATA_RATE_MAX_NUM];
    int8_t cca_level_adjust[SID_PAL_RADIO_DATA_RATE_MAX_NUM];
    int16_t ant_dbi;
} radio_lr1110_regional_param_t;

typedef struct radio_lr1110_regional_config {
   uint8_t radio_region;
   uint8_t reg_param_table_size;
   const radio_lr1110_regional_param_t *reg_param_table;
} radio_lr1110_regional_config_t;

typedef enum {
    LR1110_TCXO_CTRL_NONE = 0,
    LR1110_TCXO_CTRL_VDD  = 1,
    LR1110_TCXO_CTRL_DIO3 = 2
} lr1110_tcxo_ctrl_t;

typedef int32_t (*radio_lr1110_get_pa_cfg_t)(int8_t tx_power, radio_lr1110_pa_cfg_t *pa_cfg);

typedef struct {
    lr1110_system_reg_mode_t                regulator_mode;
    bool                                    rx_boost;
    int8_t                                  lna_gain;
    const radio_lr1110_get_pa_cfg_t         pa_cfg_callback;
    const struct sid_pal_serial_bus_factory *bus_factory;

    struct {
        uint32_t                            power;
        uint32_t                            int1;
        uint32_t                            radio_busy;
        uint32_t                            rf_sw_ena;
        uint32_t                            tx_bypass;
        uint32_t                            txrx;
        bool (*power_cb)(bool set_value, void *arg);
        void *arg;
    }                                       gpios;

    uint16_t                                wakeup_delay_us;

    lr1110_system_lfclk_cfg_t               lfclock_cfg;
    struct {
        lr1110_tcxo_ctrl_t                  ctrl;
        lr1110_system_tcxo_supply_voltage_t tune;
        uint32_t                            timeout;
    }                                       tcxo_config;

    lr1110_system_rfswitch_cfg_t            rfswitch;

    int8_t                                  rssi_no_signal_offset;

    struct {
        void                                (*pre_hook)(void*arg);
        void                                (*post_hook)(void*arg);
        void                                *arg;
    }                                       wifi_scan;

    struct {
        void                                (*pre_hook)(void*arg);
        void                                (*post_hook)(void*arg);
        void                                *arg;
    }                                       gnss_scan;

    struct sid_pal_serial_bus_client        bus_selector;

    struct {
        /* It was found that during sleep IRQ line can change it's state. This options masks
           interrupts during sleep */
        bool                                irq_noise_during_sleep;
        /* It was found that after wakeup from sleep LBD IRQ is active. This prevents further
           normal operations. This option will enable clear of this IRQ after wakeup */
        bool                                lbd_clear_on_wakeup;
    }                                       mitigations;

    struct {
        uint8_t *p;
        size_t   size;
    } internal_buffer;

    sid_pal_radio_state_transition_timings_t state_timings;
    radio_lr1110_regional_config_t          regional_config;
} radio_lr1110_device_config_t;

/**
 * Configuration for GPS scan function
 */
typedef struct {
    lr1110_gnss_search_mode_t effort_mode;
    uint8_t                   gnss_input_paramaters;
    lr1110_gnss_scan_mode_t   mode;
    sid_time_t                timeout;
    lr1110_gnss_date_t        date;
} lr1110_gnss_scan_config_t;

typedef struct {
    lr1110_wifi_signal_type_scan_t type;
    lr1110_wifi_mode_t             mode;
    lr1110_wifi_channel_mask_t     mask;
    uint32_t                       timeout_ms;     /* Total scan time*/
    uint8_t                        channel_factor; /* Nunber of scan attempts per channel */
} lr1110_wifi_scan_config_t;

/**
 * Callback to notify when radio is about to switch to sleep state
 * The callback is called in software irq context. The user has to
 * switch context from software irq in this callback.
 */
typedef void (*sid_pal_radio_sleep_start_notify_handler_t)(struct sid_timespec * const wakeup_time);

/** @brief Set Semtech radio config parameters
 *
 *  @param pointer to lr1110 radio device config
 */
void set_radio_lr1110_device_config(const radio_lr1110_device_config_t *cfg);

/** @brief Set callback to trigger external action when radio is going to sleep.
 *
 *  @param[in]   sid_pal_radio_action_in_sleep_handler_t callback to handle
 *  entering radio sleep mode. Accepts NULL if no action is required.
 *
 *  @retval  On success RADIO_ERROR_NONE.
 */
int32_t sid_hal_set_sleep_start_notify_cb(sid_pal_radio_sleep_start_notify_handler_t callback);

/** @brief Get tx power range the semtech chip supports
 *  This API is used by diagnostics firmware only.
 *  This is used to determine the max tx power the chip supports so that the diag
 *  firmware will not exceed the max tx power setting the chip supports
 *
 *  @param  max tx power to be populated.
 *  @param  min tx power to be populated.
 *  @return On success RADIO_ERROR_NONE, on error a negative number is returned
 */
int32_t get_lr1110_tx_power_range(int8_t *max_tx_power, int8_t *min_tx_power);

/**
 * @brief Start GNSS scan and get back results
 * This API is used for diagnostic only.
 *
 *  @param  config    Config for GNSS scan.
 *  @param  results   Results to populate.
 *  @param  num       Max number of results \ returned amount of results
 *  @param  error     Error description (if any)
 *  @return On success RADIO_ERROR_NONE, on error a negative number is returned
 */
int32_t lr1110_gnss_scan(const lr1110_gnss_scan_config_t *config, lr1110_gnss_detected_satellite_t *results, uint8_t *num, const char **error);

#ifdef __cplusplus
}
#endif

#endif /* HALO_DEV_RADIO_LR1110_INCLUDE_EXPORT_DEV_RADIO_LR1110_CONFIG_H */
