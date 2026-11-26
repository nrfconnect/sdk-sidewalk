/*
 * Copyright (c) 2026 Seeed Technology Co., Ltd.
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef ZEPHYR_INCLUDE_DT_BINDINGS_LORA_LBM_LR20XX_H_
#define ZEPHYR_INCLUDE_DT_BINDINGS_LORA_LBM_LR20XX_H_

/* Regulator mode (lr20xx_system_types.h: lr20xx_system_reg_mode_t) */
#define LR20XX_REG_MODE_LDO  0x00
#define LR20XX_REG_MODE_DCDC 0x02

/* Low-frequency clock source (lr20xx_system_lfclk_cfg_t) */
#define LR20XX_LFCLK_RC  0x00
#define LR20XX_LFCLK_EXT 0x02

/* TCXO supply voltage (lr20xx_system_tcxo_supply_voltage_t).
 * Used with the tcxo-voltage property; only meaningful when tcxo-wakeup-time > 0. */
#define LR20XX_SYSTEM_TCXO_CTRL_1_6V 0x00
#define LR20XX_SYSTEM_TCXO_CTRL_1_7V 0x01
#define LR20XX_SYSTEM_TCXO_CTRL_1_8V 0x02
#define LR20XX_SYSTEM_TCXO_CTRL_2_2V 0x03
#define LR20XX_SYSTEM_TCXO_CTRL_2_4V 0x04
#define LR20XX_SYSTEM_TCXO_CTRL_2_7V 0x05
#define LR20XX_SYSTEM_TCXO_CTRL_3_0V 0x06
#define LR20XX_SYSTEM_TCXO_CTRL_3_3V 0x07

#endif /* ZEPHYR_INCLUDE_DT_BINDINGS_LORA_LBM_LR20XX_H_ */
