/*
 * The Clear BSD License
 * Copyright Semtech Corporation 2021. All rights reserved.
 * Copyright (c) 2026 Nordic Semiconductor ASA.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted (subject to the limitations in the disclaimer
 * below) provided that the following conditions are met:
 *	 * Redistributions of source code must retain the above copyright
 *	   notice, this list of conditions and the following disclaimer.
 *	 * Redistributions in binary form must reproduce the above copyright
 *	   notice, this list of conditions and the following disclaimer in the
 *	   documentation and/or other materials provided with the distribution.
 *	 * Neither the name of the Semtech corporation nor the
 *	   names of its contributors may be used to endorse or promote products
 *	   derived from this software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY
 * THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT
 * NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SEMTECH CORPORATION BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <lr11xx_firmware_update.h>

#include <app_subGHz_config.h>
#include <halo_lr11xx_radio.h>
#include <lr11xx_bootloader.h>
#include <lr11xx_system.h>
#include <sid_pal_delay_ifc.h>
#include <sid_pal_gpio_ifc.h>

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(fwup, CONFIG_SIDEWALK_LOG_LEVEL);

#define LR11XX_TYPE_PRODUCTION_MODE 0xDF
#define RESET_DELAY_US              500000
#define BUSY_DIR_IN_DELAY_US        100000

static bool lr11xx_is_chip_in_production_mode(uint8_t type)
{
	return type == LR11XX_TYPE_PRODUCTION_MODE;
}

static bool lr11xx_is_fw_compatible_with_chip(lr11xx_fw_update_t update, uint16_t bootloader_version)
{
	if (((update == LR1110_FIRMWARE_UPDATE_TO_TRX) ||
	     (update == LR1110_FIRMWARE_UPDATE_TO_MODEM)) &&
	    (bootloader_version != 0x6500)) {
		return false;
	}

	if ((update == LR1120_FIRMWARE_UPDATE_TO_TRX) && (bootloader_version != 0x2000)) {
		return false;
	}

	if ((update == LR1121_FIRMWARE_UPDATE_TO_TRX) && (bootloader_version != 0x2100)) {
		return false;
	}

	return true;
}

lr11xx_fw_update_status_t lr11xx_update_firmware(lr11xx_fw_update_t fw_update_direction,
						 uint32_t fw_expected,
						 const uint32_t *buffer,
						 uint32_t length)
{
	lr11xx_bootloader_version_t version_bootloader;
	void *drv_ctx;
	const radio_lr11xx_device_config_t *cfg =
		(const radio_lr11xx_device_config_t *)get_radio_cfg();
	sid_error_t gpio_err;

	LOG_INF("Reset the chip");

	gpio_err = sid_pal_gpio_set_direction(cfg->gpios.radio_busy,
					      SID_PAL_GPIO_DIRECTION_OUTPUT);
	if (gpio_err != SID_ERROR_NONE) {
		LOG_ERR("Failed to set BUSY pin direction to output: %d", gpio_err);
		return LR11XX_FW_UPDATE_ERROR;
	}

	gpio_err = sid_pal_gpio_write(cfg->gpios.radio_busy, 0);
	if (gpio_err != SID_ERROR_NONE) {
		LOG_ERR("Failed to drive BUSY pin low: %d", gpio_err);
		return LR11XX_FW_UPDATE_ERROR;
	}

	drv_ctx = lr11xx_get_drv_ctx();
	if (lr11xx_system_reset(drv_ctx) != LR11XX_STATUS_OK) {
		LOG_ERR("lr11xx_system_reset failed");
		return LR11XX_FW_UPDATE_ERROR;
	}

	sid_pal_delay_us(RESET_DELAY_US);

	gpio_err = sid_pal_gpio_set_direction(cfg->gpios.radio_busy,
					      SID_PAL_GPIO_DIRECTION_INPUT);
	if (gpio_err != SID_ERROR_NONE) {
		LOG_ERR("Failed to set BUSY pin direction to input: %d", gpio_err);
		return LR11XX_FW_UPDATE_ERROR;
	}

	sid_pal_delay_us(BUSY_DIR_IN_DELAY_US);

	LOG_INF("Reset done");

	if (lr11xx_bootloader_get_version(drv_ctx, &version_bootloader) != LR11XX_STATUS_OK) {
		LOG_ERR("lr11xx_bootloader_get_version failed");
		return LR11XX_FW_UPDATE_ERROR;
	}

	LOG_INF("Chip in bootloader mode");
	LOG_INF(" - Chip type               = 0x%02X (0xDF for production)",
		version_bootloader.type);
	LOG_INF(" - Chip hardware version   = 0x%02X (0x22 for V2C)", version_bootloader.hw);
	LOG_INF(" - Chip bootloader version = 0x%04X", version_bootloader.fw);

	if (!lr11xx_is_chip_in_production_mode(version_bootloader.type)) {
		LOG_ERR("Chip is not in production mode");
		return LR11XX_FW_UPDATE_WRONG_CHIP_TYPE;
	}

	if (!lr11xx_is_fw_compatible_with_chip(fw_update_direction, version_bootloader.fw)) {
		LOG_ERR("Incompatible firmware for chip bootloader version");
		return LR11XX_FW_UPDATE_WRONG_CHIP_TYPE;
	}

	LOG_INF("Start flash erase");

	if (lr11xx_bootloader_erase_flash(drv_ctx) != LR11XX_STATUS_OK) {
		LOG_ERR("lr11xx_bootloader_erase_flash failed");
		return LR11XX_FW_UPDATE_ERROR;
	}

	LOG_INF("Flash erase done");

	LOG_INF("Start flashing firmware (%u bytes)", length);

	if (lr11xx_bootloader_write_flash_encrypted_full(drv_ctx, 0, buffer, length) !=
	    LR11XX_STATUS_OK) {
		LOG_ERR("lr11xx_bootloader_write_flash_encrypted_full failed");
		return LR11XX_FW_UPDATE_ERROR;
	}

	LOG_INF("Flashing done");

	LOG_INF("Rebooting");

	if (lr11xx_bootloader_reboot(drv_ctx, false) != LR11XX_STATUS_OK) {
		LOG_ERR("lr11xx_bootloader_reboot failed");
		return LR11XX_FW_UPDATE_ERROR;
	}

	LOG_INF("Reboot done");

	switch (fw_update_direction) {
	case LR1110_FIRMWARE_UPDATE_TO_TRX:
	case LR1120_FIRMWARE_UPDATE_TO_TRX:
	case LR1121_FIRMWARE_UPDATE_TO_TRX: {
		lr11xx_system_version_t version_trx;
		lr11xx_system_uid_t uid;

		if (lr11xx_system_get_version(drv_ctx, &version_trx) != LR11XX_STATUS_OK) {
			LOG_ERR("lr11xx_system_get_version failed");
			return LR11XX_FW_UPDATE_ERROR;
		}

		LOG_INF("Chip in transceiver mode");
		LOG_INF(" - Chip type               = 0x%02X", version_trx.type);
		LOG_INF(" - Chip hardware version   = 0x%02X", version_trx.hw);
		LOG_INF(" - Chip firmware version   = 0x%04X", version_trx.fw);

		if (lr11xx_system_read_uid(drv_ctx, uid) != LR11XX_STATUS_OK) {
			LOG_ERR("lr11xx_system_read_uid failed");
			return LR11XX_FW_UPDATE_ERROR;
		}

		if (version_trx.fw == fw_expected) {
			return LR11XX_FW_UPDATE_OK;
		}

		LOG_ERR("Unexpected firmware version 0x%04X (expected 0x%04X)", version_trx.fw,
			fw_expected);
		return LR11XX_FW_UPDATE_ERROR;
	}
	case LR1110_FIRMWARE_UPDATE_TO_MODEM:
		LOG_ERR("LR1110 modem firmware update is not supported");
		return LR11XX_FW_UPDATE_ERROR;
	default:
		LOG_ERR("Unsupported firmware update direction: %d", fw_update_direction);
		return LR11XX_FW_UPDATE_ERROR;
	}
}
