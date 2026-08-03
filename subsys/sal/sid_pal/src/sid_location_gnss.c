/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/* GNSS NOSUPPORT defaults for the location platform abstraction layer.
 *
 * The Sidewalk library references these entry points unconditionally, also on
 * builds that provide no GNSS location back end at all.
 *
 * Compiled only when nothing implements them, see CMakeLists.txt, so exactly
 * one definition of each symbol exists in any configuration.
 */

#include <zephyr/toolchain.h>

#include <sid_pal_gnss_ifc.h>

sid_error_t sid_pal_gnss_init(struct sid_pal_gnss_config *config)
{
	ARG_UNUSED(config);
	return SID_ERROR_NOSUPPORT;
}

sid_error_t sid_pal_gnss_process_event(uint8_t event_id)
{
	ARG_UNUSED(event_id);
	return SID_ERROR_NOSUPPORT;
}

sid_error_t sid_pal_gnss_schedule_scan(uint32_t scan_delay_s)
{
	ARG_UNUSED(scan_delay_s);
	return SID_ERROR_NOSUPPORT;
}

sid_error_t sid_pal_gnss_cancel_scan()
{
	return SID_ERROR_NOSUPPORT;
}

sid_error_t sid_pal_gnss_get_scan_payload(struct sid_pal_gnss_payload *gnss_scan_group)
{
	ARG_UNUSED(gnss_scan_group);
	return SID_ERROR_NOSUPPORT;
}

sid_error_t sid_pal_gnss_alm_demod_start()
{
	return SID_ERROR_NOSUPPORT;
}

sid_error_t sid_pal_gnss_deinit()
{
	return SID_ERROR_NOSUPPORT;
}
