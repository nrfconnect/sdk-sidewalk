/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <stddef.h>

void z_log_minimal_printk(const char *fmt, ...)
{
	(void)fmt;
}

void z_log_minimal_hexdump_print(int level, const char *data, size_t length)
{
	(void)level;
	(void)data;
	(void)length;
}
