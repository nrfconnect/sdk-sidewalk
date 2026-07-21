/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <sid_pal_gnss_ifc.h>
#include <sid_error.h>
#include <zephyr/toolchain.h>

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

sid_error_t sid_pal_gnss_cancel_scan(void)
{
	return SID_ERROR_NOSUPPORT;
}

sid_error_t sid_pal_gnss_get_scan_payload(struct sid_pal_gnss_payload *gnss_scan_group)
{
	ARG_UNUSED(gnss_scan_group);
	return SID_ERROR_NOSUPPORT;
}

sid_error_t sid_pal_gnss_alm_demod_start(void)
{
	return SID_ERROR_NOSUPPORT;
}

sid_error_t sid_pal_gnss_deinit(void)
{
	return SID_ERROR_NOSUPPORT;
}
