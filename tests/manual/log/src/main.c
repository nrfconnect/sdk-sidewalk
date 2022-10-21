/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#include <zephyr.h>
#include <sys/printk.h>

#include "sid_pal_log_ifc.h"

#include <logging/log.h>
LOG_MODULE_REGISTER(log_test, LOG_LEVEL_DBG);

void main(void)
{
	LOG_INF("> Test application log.\n");

	LOG_INF("> Test Sidewalk log levels.");
	SID_PAL_LOG_DEBUG("Sidewalk log Debug");
	SID_PAL_LOG_INFO("Sidewalk log Info");
	SID_PAL_LOG_WARNING("Sidewalk log Warning");
	SID_PAL_LOG_ERROR("Sidewalk log Error");
	sid_pal_log(SID_PAL_LOG_SEVERITY_DEBUG + 1, 0, "Sidewalk log level invalid");
	LOG_INF("Test end.\n");

	LOG_INF("> Test Sidewalk log paramteres types.");
	SID_PAL_LOG_INFO("Decimal %d", 42);
	SID_PAL_LOG_INFO("Hexadecimal 0x%x", 0xdeedbeef);
	SID_PAL_LOG_INFO("Float %f", 3.14159265359);
	SID_PAL_LOG_INFO("String %s", "Text");
	LOG_INF("Test end.\n");

	LOG_INF("> Test Sidewalk log with many paramters.");
	SID_PAL_LOG_INFO("One %d", 1);
	SID_PAL_LOG_INFO("One %d, Two: %d", 1, 2);
	SID_PAL_LOG_INFO("One %d, Two: %d, Three: %d", 1, 2, 3);
	SID_PAL_LOG_INFO("Ten: %d, %d, %d, %d, %d, %d, %d, %d, %d, %d", 1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
	LOG_INF("Test end.\n");

	LOG_INF("> Test Sidewalk log long messages.");
	SID_PAL_LOG_INFO(
		"Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.");
	LOG_INF("Test end.\n");

	LOG_INF("> Test Sidewalk log flush.");
	SID_PAL_LOG_FLUSH();
	LOG_INF("Test end.\n");

	LOG_INF("> Test Sidewalk log hexdump.");
	uint8_t data[] = { 1, 2, 3, 4, 5, 6, 7, 8 };
	SID_PAL_HEXDUMP(SID_PAL_LOG_SEVERITY_INFO, data, sizeof(data));
	LOG_INF("Test end.\n");

	LOG_INF("Sidewalk log %s, level %d\n", SID_PAL_LOG_ENABLED ? "Enabled" : "Disabled", SID_PAL_LOG_LEVEL);
}
