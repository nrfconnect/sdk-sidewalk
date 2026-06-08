/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <app.h>
#include <app_mfg_config.h>
#include <app_subGHz_config.h>
#include <lr11xx_firmware_update.h>
#include <lr1110_transceiver_0401.h>
#include <sid_api.h>
#include <sid_error.h>
#include <sid_pal_common_ifc.h>
#include <sid_pal_radio_ifc.h>

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(application, CONFIG_SIDEWALK_LOG_LEVEL);

static void radio_event_notifier(sid_pal_radio_events_t event)
{
	ARG_UNUSED(event);
}

static sid_pal_radio_rx_packet_t rx_packet;

static void radio_irq_handler(void)
{
}

void app_start(void)
{
	platform_parameters_t platform_parameters = {
		.mfg_store_region.addr_start = APP_MFG_CFG_FLASH_START,
		.mfg_store_region.addr_end = APP_MFG_CFG_FLASH_END,
#ifdef CONFIG_SIDEWALK_SUBGHZ_SUPPORT
		.platform_init_parameters.radio_cfg = get_radio_cfg(),
#else
#error "Sub-GHz support is required for LR11xx firmware update"
#endif
	};

	sid_error_t e = sid_platform_init(&platform_parameters);

	if (e != SID_ERROR_NONE) {
		LOG_ERR("Sidewalk platform init err: %d", e);
		return;
	}

	int32_t err = sid_pal_radio_init(radio_event_notifier, radio_irq_handler, &rx_packet);

	if (err != RADIO_ERROR_NONE) {
		LOG_ERR("sid_pal_radio_init() failed: %d", err);
		/* This might fail due to too old firmware. Continue anyway. */
	}

	const lr11xx_fw_update_status_t status =
		lr11xx_update_firmware(LR11XX_FIRMWARE_UPDATE_TO, LR11XX_FIRMWARE_VERSION,
				       lr11xx_firmware_image, LR11XX_FIRMWARE_IMAGE_SIZE);

	switch (status) {
	case LR11XX_FW_UPDATE_OK:
		LOG_INF("Expected firmware running");
		LOG_INF("Flash another application (for example sid_end_device) to continue");
		break;
	case LR11XX_FW_UPDATE_WRONG_CHIP_TYPE:
		LOG_ERR("Wrong chip type");
		break;
	case LR11XX_FW_UPDATE_ERROR:
		LOG_ERR("Firmware update failed - please retry");
		break;
	}
}
