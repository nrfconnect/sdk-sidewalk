/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#include <zephyr/logging/log.h>

#if CONFIG_SIDEWALK_DFU_SERVICE_USB
#include <zephyr/usb/usb_device.h>
#endif

#if CONFIG_BOOTLOADER_MCUBOOT
#include <zephyr/dfu/mcuboot.h>
#endif

#include <sidewalk_version.h>

#include <sid_error.h>
#include <sid_dut_shell.h>
#include <sid_thread.h>

LOG_MODULE_REGISTER(sid_main, LOG_LEVEL_INF);

void main(void)
{
	PRINT_SIDEWALK_VERSION();

	initialize_sidewalk_shell(sid_thread_get_config(), sid_thread_get_app_context());

	sid_error_t e = sid_thread_init();

	if (e != SID_ERROR_NONE) {
		LOG_ERR("sidwalk failed to initialize with error (sid_error_t) %d", e);
	}

	#if CONFIG_BOOTLOADER_MCUBOOT
	if (!boot_is_img_confirmed()) {
		int ret = boot_write_img_confirmed();

		if (ret) {
			LOG_ERR("Couldn't confirm image: %d", ret);
		} else {
			LOG_INF("Marked image as OK");
		}
	}
	#endif
}
