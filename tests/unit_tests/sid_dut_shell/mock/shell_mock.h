/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/fff.h>
#include <zephyr/shell/shell.h>

DEFINE_FFF_GLOBALS;

FAKE_VOID_FUNC_VARARG(shell_fprintf, const struct shell *, enum shell_vt100_color, const char *,
		      ...);
