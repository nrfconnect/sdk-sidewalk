/*
 * Copyright 2021 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.txt file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef SEMTECH_RADIO_IFC_H
#define SEMTECH_RADIO_IFC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

typedef struct semtech_radio_pa_cfg {
    uint8_t pa_duty_cycle;
    uint8_t hp_max;
    uint8_t device_sel;
    uint8_t pa_lut;
    int8_t tx_power;
    uint8_t ramp_time;
    bool enable_ext_pa;
} semtech_radio_pa_cfg_t;

/**
 * APIs for diagnostic support. Please add APIs for diagnostic support below this
*/

/** @brief Set Semtech radio crystal trim capacitor value
 *
 *  @param trim capacitor value
 *  @return On success RADIO_ERROR_NONE, on error a negative number is returned
 */
int32_t semtech_radio_set_trim_cap_val(uint16_t trim);

/** @brief Get Semtech radio crystal trim capacitor value
 *
 *  @return configured trim capacitors value
 */
uint16_t semtech_radio_get_trim_cap_val(void);

/** @brief Set pa config
 *  API used to set a different pa config if the defaults set
 *  through target config has to be modified
 *
 *  @param  pa config
 *  @return On success RADIO_ERROR_NONE, on error a negative number is returned
 */
int32_t semtech_radio_set_sx126x_pa_config(semtech_radio_pa_cfg_t *cfg);

/** @brief Get current pa config
 *  This will copy the current pa config to passed parameter
 *  @param  pa config
 *  @return On success RADIO_ERROR_NONE, on error a negative number is returned
 */
int32_t semtech_radio_get_pa_config(semtech_radio_pa_cfg_t *cfg);

/** @brief Get tx power range the semtech chip supports
 *  This API is used by diagnostics firmware only.
 *  This is used to determine the max tx power the chip supports so that the diag
 *  firmware will not exceed the max tx power setting the chip supports
 *
 *  @param  max tx power to be populated.
 *  @param  min tx power to be populated.
 *  @return On success RADIO_ERROR_NONE, on error a negative number is returned
 */
int32_t semtech_radio_get_tx_power_range(int8_t *max_tx_power, int8_t *min_tx_power);

/** @brief Get fsk mod shaping for index
 *
 *  @param[in]  idx index for mod shaping for fsk
 *  @param[out] ms pointer to the value of the mod shaping at that index.
 *
 *  @retval RADIO_ERROR_NONE if valid index, RADIO_ERROR_INVALID_PARAMS otherwise
 */
int32_t semtech_radio_get_fsk_mod_shaping(uint8_t idx, uint8_t *ms);

/** @brief Get bandwidth in fsk mode for index
 *
 *  @param[in]  idx index for bandwidth in fsk mode
 *  @param[out] bw pointer to the value of the bandwidth at that index.
 *
 *  @retval RADIO_ERROR_NONE if valid index, RADIO_ERROR_INVALID_PARAMS otherwise
 */
int32_t semtech_radio_get_fsk_bw(uint8_t idx, uint8_t *bw);

/** @brief Get addr comp for fsk mode for index
 *
 *  @param[in]  idx index for addr comp in fsk mode
 *  @param[out] ac pointer to the value of the addr comp at that index.
 *
 *  @retval RADIO_ERROR_NONE if valid index, RADIO_ERROR_INVALID_PARAMS otherwise
 */
int32_t semtech_radio_get_fsk_addr_comp(uint8_t idx, uint8_t *ac);

/** @brief Get preamble detect value for fsk mode for index
 *
 *  @param[in]  idx index for preamble detect in fsk mode
 *  @param[out] pd pointer to the value of the preamble detect at that index.
 *
 *  @retval RADIO_ERROR_NONE if valid index, RADIO_ERROR_INVALID_PARAMS otherwise
 */
int32_t semtech_radio_get_fsk_preamble_detect(uint8_t idx, uint8_t *pd);

/** @brief Get crc type value for fsk mode for index
 *
 *  @param[in]  idx index for crc type in fsk mode
 *  @param[out] crc pointer to the value of the crc type at that index.
 *
 *  @retval RADIO_ERROR_NONE if valid index, RADIO_ERROR_INVALID_PARAMS otherwise
 */
int32_t semtech_radio_get_fsk_crc_type(uint8_t idx, uint8_t *crc);

/** @brief Get index for mod shaping value
 *
 *  @param[in] ms mod shaping value
 *
 *  @retval index for mod shaping value
 */
int16_t semtech_radio_get_fsk_mod_shaping_idx(uint8_t ms);

/** @brief Get index for bandwidth value
 *
 *  @param[in] bw bandwidth value
 *
 *  @retval index for bandwidth value
 */
int16_t semtech_radio_get_fsk_bw_idx(uint8_t bw);

/** @brief Get index for addr comp value
 *
 *  @param[in] ac addr comp value
 *
 *  @retval index for addr comp value
 */
int16_t semtech_radio_get_fsk_addr_comp_idx(uint8_t ac);

/** @brief Get index for preamble detect value
 *
 *  @param[in] pd preamble detect value
 *
 *  @retval index for preamble detect value
 */
int16_t semtech_radio_get_fsk_preamble_detect_idx(uint8_t pd);

/** @brief Get index for crc type value
 *
 *  @param[in] crc crc value
 *
 *  @retval index for crc value
 */
int16_t semtech_radio_get_fsk_crc_type_idx(uint8_t crc);

/** @brief Get Spreading Factor value for LoRa mode for index
 *
 *  @param[in]  idx index for spreading factor in LoRa mode
 *  @param[out] sf pointer to the value of the spreading factor at that index.
 *
 *  @retval RADIO_ERROR_NONE if valid index, RADIO_ERROR_INVALID_PARAMS otherwise
 */
int32_t semtech_radio_get_lora_sf(uint8_t idx, uint8_t *sf);

/** @brief Get bandwidth value for LoRa mode for index
 *
 *  @param[in]  idx index for bandwidth in LoRa mode
 *  @param[out] bw pointer to the value of the bandwidth at that index.
 *
 *  @retval RADIO_ERROR_NONE if valid index, RADIO_ERROR_INVALID_PARAMS otherwise
 */
int32_t semtech_radio_get_lora_bw(uint8_t idx, uint8_t *bw);

/** @brief Get coding rate value for LoRa mode for index
 *
 *  @param[in]  idx index for coding rate in LoRa mode
 *  @param[out] cr pointer to the value of the coding rate at that index.
 *
 *  @retval RADIO_ERROR_NONE if valid index, RADIO_ERROR_INVALID_PARAMS otherwise
 */
int32_t semtech_radio_get_lora_cr(uint8_t idx, uint8_t *cr);

/** @brief Get cad symbol value for LoRa mode for index
 *
 *  @param idx index for cad symbol in LoRa mode
 *  @param sym pointer to the value of the cad symbol at that index.
 *
 *  @retval RADIO_ERROR_NONE if valid index, RADIO_ERROR_INVALID_PARAMS otherwise
 */
int32_t semtech_radio_get_lora_cad_symbol(uint8_t idx, uint8_t *sym);

/** @brief Get cad exit mode value for LoRa mode for index
 *
 *  @param[in]  idx index for cad exit mode in LoRa mode
 *  @param[out] em pointer to the value of the cad exit mode at that index.
 *
 *  @retval RADIO_ERROR_NONE if valid index, RADIO_ERROR_INVALID_PARAMS otherwise
 */
int32_t semtech_radio_get_lora_cad_exit_mode(uint8_t idx, uint8_t *em);

/** @brief Get index for given spreading factor in LoRa mode
 *
 *  @param[in] sf spreading factor value for the index that is requested
 *
 *  @retval index or -1 if invalid parameter passed
 */
int16_t semtech_radio_get_lora_sf_idx(uint8_t sf);

/** @brief Get index for given bandwidth in LoRa mode
 *
 *  @param[in] bw bandwidth value for the index that is requested
 *
 *  @retval index or -1 if invalid parameter passed
 */
int16_t semtech_radio_get_lora_bw_idx(uint8_t bw);

/** @brief Get index for given coding rate in LoRa mode
 *
 *  @param[in] cr coding rate value for the index that is requested
 *
 *  @retval index or -1 if invalid parameter passed
 */
int16_t semtech_radio_get_lora_cr_idx(uint8_t cr);

/** @brief Get index for given cad symbol in LoRa mode
 *
 *  @param[in] sym cad symbol value for the index that is requested
 *
 *  @retval index or -1 if invalid parameter passed
 */
int16_t semtech_radio_get_lora_cad_symbol_idx(uint8_t sym);

/** @brief Get index for cad exit mode in LoRa mode
 *
 *  @param[in] em cad exit mode value for the index that is requested
 *
 *  @retval index or -1 if invalid parameter passed
 */
int16_t semtech_radio_get_lora_cad_exit_mode_idx(uint8_t em);

#ifdef __cplusplus
}
#endif

/** @} */

#endif
