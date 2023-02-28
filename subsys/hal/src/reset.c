/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

/** @file reset.c
 *  @brief HAL reboot (reset) implementation.
 */

#include <sid_hal_reset_ifc.h>
#include <zephyr/sys/reboot.h>
#include <zephyr/kernel.h>

sid_error_t sid_hal_reset(sid_hal_reset_type_t type)
{
	if (SID_HAL_RESET_NORMAL == type) {
		sys_reboot(SYS_REBOOT_WARM);
	} else {
		return SID_ERROR_NOSUPPORT;
	}

	return SID_ERROR_NONE;
}
