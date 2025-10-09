/*
 * Copyright (c) 2019-2025 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * This file is exposed to each procuct configuration to customize the driver behavior
 */

#ifndef SX126X_CONFIG_H
#define SX126X_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <semtech_radio_ifc.h>
#include <sid_pal_radio_ifc.h>
#include <sid_pal_serial_bus_ifc.h>

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#define SEMTECH_ID_SX1261                       0x1
#define SEMTECH_ID_SX1262                       0x2

#define RADIO_SX126X_REGULATOR_LDO              0x00
#define RADIO_SX126X_REGULATOR_DCDC             0x01

#define RADIO_SX126X_RAMP_10_US                 0x00
#define RADIO_SX126X_RAMP_20_US                 0x01
#define RADIO_SX126X_RAMP_40_US                 0x02
#define RADIO_SX126X_RAMP_80_US                 0x03
#define RADIO_SX126X_RAMP_200_US                0x04
#define RADIO_SX126X_RAMP_800_US                0x05
#define RADIO_SX126X_RAMP_1700_US               0x06
#define RADIO_SX126X_RAMP_3400_US               0x07

#define RADIO_SX126X_TCXO_CTRL_1_6V             0x00
#define RADIO_SX126X_TCXO_CTRL_1_7V             0x01
#define RADIO_SX126X_TCXO_CTRL_1_8V             0x02
#define RADIO_SX126X_TCXO_CTRL_2_2V             0x03
#define RADIO_SX126X_TCXO_CTRL_2_4V             0x04
#define RADIO_SX126X_TCXO_CTRL_2_7V             0x05
#define RADIO_SX126X_TCXO_CTRL_3_0V             0x06
#define RADIO_SX126X_TCXO_CTRL_3_3V             0x07

#define HALO_GPIO_NOT_CONNECTED                 128

#define RADIO_REGION_NA SID_PAL_RADIO_RC_NA
#define RADIO_REGION_EU SID_PAL_RADIO_RC_EU
#define RADIO_REGION_NONE SID_PAL_RADIO_RC_NONE

typedef enum {
    SX126X_TCXO_CTRL_NONE = 0,
    SX126X_TCXO_CTRL_VDD  = 1,
    SX126X_TCXO_CTRL_DIO3 = 2
} sx126x_tcxo_ctrl_t;

typedef struct radio_sx126x_pa_cfg {
    uint8_t pa_duty_cycle;
    uint8_t hp_max;
    uint8_t device_sel;
    uint8_t pa_lut;
    int8_t tx_power;
    uint8_t ramp_time;
    bool enable_ext_pa;
} radio_sx126x_pa_cfg_t;

typedef struct radio_sx126x_tcxo {
    sx126x_tcxo_ctrl_t ctrl;
    uint8_t dio3_to_mcu_pin;
    uint8_t voltage;
    uint32_t timeout; // Delay duration = timeout * 15.625 μs
} radio_sx126x_tcxo_t;

typedef struct radio_sx126x_regional_param {
    uint8_t param_region;
    int8_t max_tx_power[SID_PAL_RADIO_DATA_RATE_MAX_NUM];
    int8_t cca_level_adjust[SID_PAL_RADIO_DATA_RATE_MAX_NUM];
    int16_t ant_dbi;
} radio_sx126x_regional_param_t;

typedef struct radio_sx126x_regional_config {
   uint8_t radio_region;
   uint8_t reg_param_table_size;
   const radio_sx126x_regional_param_t *reg_param_table;
} radio_sx126x_regional_config_t;

typedef int32_t (*radio_sx126x_get_pa_cfg_t)(int8_t tx_power, radio_sx126x_pa_cfg_t *pa_cfg);

// Get Trim Capacitor value read from the mfg page
typedef int32_t (*radio_sx126x_get_mfg_trim_val_t)(uint16_t *trim);

// DIO3 out voltage to supply antenna switch power.
typedef int32_t (*radio_sx126x_get_dio3_cfg_t)(uint8_t radio_state);

// Band select logical control to support RF TRX band switch.
typedef int32_t (*radio_sx126x_band_select_logical_cfg_t)(bool tx_en);

typedef struct {
    uint8_t id;
    uint8_t regulator_mode;
    bool    rx_boost;
    int8_t  lna_gain;
    const radio_sx126x_get_pa_cfg_t pa_cfg_callback;
    const radio_sx126x_get_mfg_trim_val_t trim_cap_val_callback;
    const radio_sx126x_get_dio3_cfg_t dio3_cfg_callback;
    const radio_sx126x_band_select_logical_cfg_t band_sel_cfg_callback;
    const struct sid_pal_serial_bus_factory *bus_factory;

    uint32_t gpio_power;
    uint32_t gpio_int1;
    uint32_t gpio_radio_busy;
    uint32_t gpio_rf_sw_ena;
    uint32_t gpio_tx_bypass;

    struct sid_pal_serial_bus_client bus_selector;
    radio_sx126x_tcxo_t tcxo;
    sid_pal_radio_state_transition_timings_t state_timings;
    struct {
        uint8_t *p;
        size_t   size;
    } internal_buffer;

    radio_sx126x_regional_config_t          regional_config;
} radio_sx126x_device_config_t;

/** @brief Set Semtech radio config parameters
 *
 *  @param pointer to sx126x radio device config
 */
void set_radio_sx126x_device_config(const radio_sx126x_device_config_t *cfg);

#ifdef __cplusplus
}
#endif

#endif /* HALO_DEV_RADIO_SX126X_INCLUDE_EXPORT_DEV_RADIO_SX126X_CONFIG_H */
