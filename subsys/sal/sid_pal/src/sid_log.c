/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/** @file sid_log.c
 *  @brief Log interface implementation.
 */

#include <sid_pal_log_ifc.h>
#include <stddef.h>

#include <zephyr/sys/printk.h>
#include <zephyr/logging/log.h>
#include <zephyr/logging/log_ctrl.h>

LOG_MODULE_REGISTER(sidewalk, CONFIG_SIDEWALK_LOG_LEVEL);

#define LOG_FLUSH_SLEEP_PERIOD K_MSEC(5)

#define MSG_LENGTH_MAX (CONFIG_SIDEWALK_LOG_MSG_LENGTH_MAX)

#if CONFIG_LOG
#define LOG_VALIST(_level, fmt, valist)                                                            \
	do {                                                                                       \
		LOG_RAW("%c: ", z_log_minimal_level_to_char(_level));                              \
		z_log_vprintk(fmt, valist);                                                        \
		LOG_RAW("\n");                                                                     \
	} while (false)
#else
#define LOG_VALIST(_level, fmt, valist)
#endif

void sid_pal_log(sid_pal_log_severity_t severity, uint32_t num_args, const char *fmt, ...)
{
	ARG_UNUSED(num_args);

	va_list args;
	va_start(args, fmt);

	switch (severity) {
	case SID_PAL_LOG_SEVERITY_ERROR:
		LOG_VALIST(LOG_LEVEL_ERR, fmt, args);
		break;
	case SID_PAL_LOG_SEVERITY_WARNING:
		LOG_VALIST(LOG_LEVEL_WRN, fmt, args);
		break;
	case SID_PAL_LOG_SEVERITY_INFO:
		LOG_VALIST(LOG_LEVEL_INF, fmt, args);
		break;
	case SID_PAL_LOG_SEVERITY_DEBUG:
		LOG_VALIST(LOG_LEVEL_DBG, fmt, args);
		break;
	default:
		LOG_VALIST(LOG_LEVEL_DBG, fmt, args);
		LOG_WRN("sid pal log unknow severity %d", severity);
		break;
	}

	va_end(args);
}

void sid_pal_hexdump(sid_pal_log_severity_t severity, const void *address, int length)
{
	switch (severity) {
	case SID_PAL_LOG_SEVERITY_ERROR:
		LOG_HEXDUMP_ERR(address, length, "");
		break;
	case SID_PAL_LOG_SEVERITY_WARNING:
		LOG_HEXDUMP_WRN(address, length, "");
		break;
	case SID_PAL_LOG_SEVERITY_INFO:
		LOG_HEXDUMP_INF(address, length, "");
		break;
	case SID_PAL_LOG_SEVERITY_DEBUG:
		LOG_HEXDUMP_DBG(address, length, "");
		break;
	default:
		LOG_WRN("sid pal log unknow severity %d", severity);
		LOG_HEXDUMP_DBG(address, length, "");
		break;
	}
}

void sid_pal_log_flush(void)
{
#ifndef CONFIG_LOG_MODE_MINIMAL
	/* Note: log_buffered_cnt is not supported in minimal log mode. */
	while (log_buffered_cnt()) {
		k_sleep(LOG_FLUSH_SLEEP_PERIOD);
	}
#endif
}

char const *sid_pal_log_push_str(char *string)
{
	return string;
}

bool sid_pal_log_get_log_buffer(struct sid_pal_log_buffer *const log_buffer)
{
	ARG_UNUSED(log_buffer);
	LOG_WRN("%s - not implemented (optional).", __func__);

	return false;
}

sid_pal_log_severity_t sid_log_control_get_current_log_level(void)
{
	return (sid_pal_log_severity_t)SID_PAL_LOG_LEVEL;
}
