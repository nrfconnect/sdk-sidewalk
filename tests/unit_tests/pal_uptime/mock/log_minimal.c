/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/sys/printk.h>

void z_log_minimal_printk(const char *fmt, ...)
{
	ARG_UNUSED(fmt);
}
