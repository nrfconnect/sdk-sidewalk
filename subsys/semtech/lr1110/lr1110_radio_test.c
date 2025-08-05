/*
 * Copyright (c) 2019-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * This file supports diagnostic functionality
 */

#include "lr1110_radio.h"
#include "halo_lr1110_radio.h"

#include <sid_pal_radio_ifc.h>
#include <semtech_radio_ifc.h>

#include <sid_utils.h>
#include <assert.h>

int32_t semtech_radio_get_tx_power_range(int8_t *max_tx_power, int8_t *min_tx_power)
{
   halo_drv_semtech_ctx_t *drv_ctx = lr1110_get_drv_ctx();

   switch (drv_ctx->pa_cfg.pa_cfg.pa_sel) {
    case LR1110_RADIO_PA_SEL_LP:
        *max_tx_power = LP_MAX_POWER;
        *min_tx_power = LP_MIN_POWER;
        return RADIO_ERROR_NONE;
    case LR1110_RADIO_PA_SEL_HP:
    case LR1110_RADIO_PA_SEL_HF:
        *max_tx_power = HP_MAX_POWER;
        *min_tx_power = HP_MIN_POWER;
        return RADIO_ERROR_NONE;
    default:
        assert(0);
        break;
    }

    return RADIO_ERROR_INVALID_PARAMS;
}

int32_t semtech_radio_set_trim_cap_val(uint16_t trim)
{
    return RADIO_ERROR_NOT_SUPPORTED;
}

uint16_t semtech_radio_get_trim_cap_val(void)
{
    return RADIO_ERROR_NOT_SUPPORTED;
}

int32_t semtech_radio_get_pa_config(semtech_radio_pa_cfg_t *cfg)
{
    if (!cfg) {
        return RADIO_ERROR_INVALID_PARAMS;
    }

    radio_lr1110_pa_cfg_t cur_cfg;
    get_radio_lr1110_pa_config(&cur_cfg);

    cfg->pa_duty_cycle = cur_cfg.pa_cfg.pa_duty_cycle;
    cfg->hp_max = cur_cfg.pa_cfg.pa_hp_sel;
    cfg->device_sel = (uint8_t)cur_cfg.pa_cfg.pa_sel;
    cfg->pa_lut = (uint8_t)cur_cfg.pa_cfg.pa_reg_supply;
    cfg->tx_power = cur_cfg.tx_power_in_dbm;
    cfg->ramp_time = (uint8_t)cur_cfg.ramp_time;
    cfg->enable_ext_pa = cur_cfg.enable_ext_pa;

    return RADIO_ERROR_NONE;
}

static const lr1110_radio_lora_sf_t lora_sf[] = {
    LR1110_RADIO_LORA_SF5,  LR1110_RADIO_LORA_SF6,  LR1110_RADIO_LORA_SF7,
    LR1110_RADIO_LORA_SF8,  LR1110_RADIO_LORA_SF9,  LR1110_RADIO_LORA_SF10,
    LR1110_RADIO_LORA_SF11, LR1110_RADIO_LORA_SF12
};

static const lr1110_radio_lora_bw_t lora_bw[] = {
    LR1110_RADIO_LORA_BW_500, LR1110_RADIO_LORA_BW_250, LR1110_RADIO_LORA_BW_125,
    LR1110_RADIO_LORA_BW_62, LR1110_RADIO_LORA_BW_41, LR1110_RADIO_LORA_BW_31,
    LR1110_RADIO_LORA_BW_20, LR1110_RADIO_LORA_BW_15, LR1110_RADIO_LORA_BW_10
};

static const lr1110_radio_lora_cr_t lora_cr[] = {
    LR1110_RADIO_LORA_CR_4_5, LR1110_RADIO_LORA_CR_4_6, LR1110_RADIO_LORA_CR_4_7,
    LR1110_RADIO_LORA_CR_4_8, LR1110_RADIO_LORA_CR_LI_4_5, LR1110_RADIO_LORA_CR_LI_4_6,
    LR1110_RADIO_LORA_CR_LI_4_8
};

static const uint8_t lora_cad_symbol[] = { 1, 2, 4, 8, 16};

static const lr1110_radio_cad_exit_mode_t lora_cad_exit_mode[] = {
    LR1110_RADIO_CAD_EXIT_MODE_STANDBYRC, LR1110_RADIO_CAD_EXIT_MODE_RX,
    LR1110_RADIO_CAD_EXIT_MODE_TX
};

#define LORA_SF_NUM countof(lora_sf)
#define LORA_BW_NUM countof(lora_bw)
#define LORA_CR_NUM countof(lora_cr)
#define LORA_CAD_SYMBOL_NUM countof(lora_cad_symbol)
#define LORA_CAD_EXIT_MODE_NUM countof(lora_cad_exit_mode)

int32_t semtech_radio_get_lora_sf(uint8_t idx, uint8_t *sf)
{
    if (idx >= LORA_SF_NUM) {
        return RADIO_ERROR_INVALID_PARAMS;
    }
    *sf = lora_sf[idx];
    return RADIO_ERROR_NONE;
}

int16_t semtech_radio_get_lora_sf_idx(uint8_t sf)
{
    for (int i = 0; i < LORA_SF_NUM; i++) {
        if (lora_sf[i] == sf)
            return i;
    }
    return -1;
}


int32_t semtech_radio_get_lora_bw(uint8_t idx, uint8_t *bw)
{
    if (idx >= LORA_BW_NUM) {
        return RADIO_ERROR_INVALID_PARAMS;
    }
    *bw = lora_bw[idx];
    return RADIO_ERROR_NONE;
}

int16_t semtech_radio_get_lora_bw_idx(uint8_t bw)
{
    for (int i = 0; i < LORA_BW_NUM; i++) {
        if (lora_bw[i] == bw) {
            return i;
        }
    }
    return -1;
}

int32_t semtech_radio_get_lora_cr(uint8_t idx, uint8_t *cr)
{
    if (idx >= LORA_CR_NUM) {
        return RADIO_ERROR_INVALID_PARAMS;
    }
    *cr = lora_cr[idx];
    return RADIO_ERROR_NONE;
}

int16_t semtech_radio_get_lora_cr_idx(uint8_t cr)
{
    for (int i = 0; i < LORA_CR_NUM; i++) {
        if (lora_cr[i] == cr) {
            return i;
        }
    }
    return -1;
}

int32_t semtech_radio_get_lora_cad_symbol(uint8_t idx, uint8_t *sym)
{
    if (idx >= LORA_CAD_SYMBOL_NUM) {
        return RADIO_ERROR_INVALID_PARAMS;
    }
    *sym = lora_cad_symbol[idx];
    return RADIO_ERROR_NONE;
}

int16_t semtech_radio_get_lora_cad_symbol_idx(uint8_t sym)
{
    for (int i = 0; i < LORA_CAD_SYMBOL_NUM; i++) {
        if (lora_cad_symbol[i] == sym) {
            return i;
        }
    }
    return -1;
}

int32_t semtech_radio_get_lora_cad_exit_mode(uint8_t idx, uint8_t *em)
{
    if (idx >= LORA_CAD_EXIT_MODE_NUM) {
        return RADIO_ERROR_INVALID_PARAMS;
    }
    *em = lora_cad_exit_mode[idx];
    return RADIO_ERROR_NONE;
}

int16_t semtech_radio_get_lora_cad_exit_mode_idx(uint8_t em)
{
    for (int i = 0; i < LORA_CAD_EXIT_MODE_NUM; i++) {
        if (lora_cad_exit_mode[i] == em) {
            return i;
        }
    }
    return -1;
}



static const lr1110_radio_gfsk_pulse_shape_t fsk_mod_shaping[] = {
    LR1110_RADIO_GFSK_PULSE_SHAPE_OFF,
    LR1110_RADIO_GFSK_PULSE_SHAPE_BT_03,
    LR1110_RADIO_GFSK_PULSE_SHAPE_BT_05,
    LR1110_RADIO_GFSK_PULSE_SHAPE_BT_07,
    LR1110_RADIO_GFSK_PULSE_SHAPE_BT_1
};

static const lr1110_radio_gfsk_bw_t fsk_bw[] = {
    LR1110_RADIO_GFSK_BW_4800,   LR1110_RADIO_GFSK_BW_5800,   LR1110_RADIO_GFSK_BW_7300,
    LR1110_RADIO_GFSK_BW_9700,   LR1110_RADIO_GFSK_BW_11700,  LR1110_RADIO_GFSK_BW_14600,
    LR1110_RADIO_GFSK_BW_19500,  LR1110_RADIO_GFSK_BW_23400,  LR1110_RADIO_GFSK_BW_29300,
    LR1110_RADIO_GFSK_BW_39000,  LR1110_RADIO_GFSK_BW_46900,  LR1110_RADIO_GFSK_BW_58600,
    LR1110_RADIO_GFSK_BW_78200,  LR1110_RADIO_GFSK_BW_93800,  LR1110_RADIO_GFSK_BW_117300,
    LR1110_RADIO_GFSK_BW_156200, LR1110_RADIO_GFSK_BW_187200, LR1110_RADIO_GFSK_BW_234300,
    LR1110_RADIO_GFSK_BW_312000, LR1110_RADIO_GFSK_BW_373600, LR1110_RADIO_GFSK_BW_467000
};

static const lr1110_radio_gfsk_address_filtering_t fsk_addr_comp[] = {
    LR1110_RADIO_GFSK_ADDRESS_FILTERING_DISABLE, LR1110_RADIO_GFSK_ADDRESS_FILTERING_NODE_ADDRESS,
    LR1110_RADIO_GFSK_ADDRESS_FILTERING_NODE_AND_BROADCAST_ADDRESSES
};

static const lr1110_radio_gfsk_preamble_detector_t fsk_preamble_detect[] = {
    LR1110_RADIO_GFSK_PREAMBLE_DETECTOR_OFF, LR1110_RADIO_GFSK_PREAMBLE_DETECTOR_MIN_8BITS,
    LR1110_RADIO_GFSK_PREAMBLE_DETECTOR_MIN_16BITS, LR1110_RADIO_GFSK_PREAMBLE_DETECTOR_MIN_24BITS,
    LR1110_RADIO_GFSK_PREAMBLE_DETECTOR_MIN_32BITS
};

static const lr1110_radio_gfsk_crc_type_t fsk_crc_type[] = {
    LR1110_RADIO_GFSK_CRC_OFF, LR1110_RADIO_GFSK_CRC_1_BYTE, LR1110_RADIO_GFSK_CRC_2_BYTES,
    LR1110_RADIO_GFSK_CRC_1_BYTE_INV, LR1110_RADIO_GFSK_CRC_2_BYTES_INV
};

#define FSK_MOD_SHAPING_PARAMS_NUM     countof(fsk_mod_shaping)
#define FSK_BW_NUM                     countof(fsk_bw)
#define FSK_ADDR_COMP_NUM              countof(fsk_addr_comp)
#define FSK_PREAMBLE_DETECT_NUM        countof(fsk_preamble_detect)
#define FSK_CRC_TYPES_NUM              countof(fsk_crc_type)


int32_t semtech_radio_get_fsk_mod_shaping(uint8_t idx, uint8_t *ms)
{
    if (idx >= FSK_MOD_SHAPING_PARAMS_NUM) {
        return RADIO_ERROR_INVALID_PARAMS;
    }
    *ms = fsk_mod_shaping[idx];
    return RADIO_ERROR_NONE;
}

int16_t semtech_radio_get_fsk_mod_shaping_idx(uint8_t ms)
{
    for (int i = 0; i < FSK_MOD_SHAPING_PARAMS_NUM; i++) {
        if (fsk_mod_shaping[i] == ms) {
            return i;
        }
    }
    return -1;
}

int32_t semtech_radio_get_fsk_bw(uint8_t idx, uint8_t *bw)
{
    if (idx >= FSK_BW_NUM) {
        return RADIO_ERROR_INVALID_PARAMS;
    }
    *bw = fsk_bw[idx];
    return RADIO_ERROR_NONE;
}

int16_t semtech_radio_get_fsk_bw_idx(uint8_t bw)
{
    for (int i = 0; i < FSK_BW_NUM; i++) {
        if (fsk_bw[i] == bw) {
            return i;
        }
    }
    return -1;
}

int32_t semtech_radio_get_fsk_addr_comp(uint8_t idx, uint8_t *ac)
{
    if (idx >= FSK_ADDR_COMP_NUM) {
        return RADIO_ERROR_INVALID_PARAMS;
    }
    *ac = fsk_addr_comp[idx];
    return RADIO_ERROR_NONE;
}


int16_t semtech_radio_get_fsk_addr_comp_idx(uint8_t ac)
{
    for (int i = 0; i < FSK_ADDR_COMP_NUM; i++) {
        if (fsk_addr_comp[i] == ac) {
            return i;
        }
    }
    return -1;
}

int32_t semtech_radio_get_fsk_preamble_detect(uint8_t idx, uint8_t *pd)
{
    if (idx >= FSK_PREAMBLE_DETECT_NUM) {
        return RADIO_ERROR_INVALID_PARAMS;
    }
    *pd = fsk_preamble_detect[idx];
    return RADIO_ERROR_NONE;
}

int16_t semtech_radio_get_fsk_preamble_detect_idx(uint8_t pd)
{
    for (int i = 0; i < FSK_PREAMBLE_DETECT_NUM; i++) {
        if (fsk_preamble_detect[i] == pd) {
            return i;
        }
    }
    return -1;
}

int32_t semtech_radio_get_fsk_crc_type(uint8_t idx, uint8_t *crc)
{
    if (idx >= FSK_CRC_TYPES_NUM) {
        return RADIO_ERROR_INVALID_PARAMS;
    }
    *crc = fsk_crc_type[idx];
    return RADIO_ERROR_NONE;
}


int16_t semtech_radio_get_fsk_crc_type_idx(uint8_t crc)
{
    for (int i = 0; i < FSK_CRC_TYPES_NUM; i++) {
        if (fsk_crc_type[i] == crc) {
            return i;
        }
    }
    return -1;
}

