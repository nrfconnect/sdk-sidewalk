/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/* Wi-Fi NOSUPPORT defaults for the location platform abstraction layer.
 *
 * The Sidewalk library references these entry points unconditionally, also on
 * builds that provide no Wi-Fi location back end at all.
 *
 * Compiled only when nothing implements them, see CMakeLists.txt, so exactly
 * one definition of each symbol exists in any configuration.
 */

#include <zephyr/toolchain.h>

#include <sid_pal_wifi_ifc.h>

sid_error_t sid_pal_wifi_init(struct sid_pal_wifi_config *config)
{
	ARG_UNUSED(config);
	return SID_ERROR_NOSUPPORT;
}

sid_error_t sid_pal_wifi_process_event(uint8_t event_id)
{
	ARG_UNUSED(event_id);
	return SID_ERROR_NOSUPPORT;
}

sid_error_t sid_pal_wifi_deinit()
{
	return SID_ERROR_NOSUPPORT;
}

sid_error_t sid_pal_wifi_schedule_scan(uint32_t scan_delay_s)
{
	ARG_UNUSED(scan_delay_s);
	return SID_ERROR_NOSUPPORT;
}

sid_error_t sid_pal_wifi_cancel_scan()
{
	return SID_ERROR_NOSUPPORT;
}

sid_error_t sid_pal_wifi_get_scan_payload(struct sid_pal_wifi_payload *wifi_scan_result)
{
	ARG_UNUSED(wifi_scan_result);
	return SID_ERROR_NOSUPPORT;
}
