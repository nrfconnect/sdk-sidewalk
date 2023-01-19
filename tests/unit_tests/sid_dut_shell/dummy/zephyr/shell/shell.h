/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#ifndef SHELL_MOCK_H
#define SHELL_MOCK_H

#define SHELL_STATIC_SUBCMD_SET_CREATE(...)
#define SHELL_CMD_ARG(...)
#define SHELL_CMD_REGISTER(...)

struct shell {
	void *phantom_data;
};

static inline void dummy(const char *format, ...)
{
	(void) format;
}

#define shell_error(shell, ...) dummy(__VA_ARGS__)
#define shell_info(shell, ...) dummy(__VA_ARGS__)
#define shell_warning(shell, ...) dummy(__VA_ARGS__)

#endif
