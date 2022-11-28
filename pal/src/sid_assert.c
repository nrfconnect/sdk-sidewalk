/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

/** @file sid_assert.c
 *  @brief Assert implementation.
 */

#include <sid_pal_assert_ifc.h>
#include <zephyr/sys/__assert.h>

void sid_pal_assert(int line, const char *file)
{
#ifdef CONFIG_ASSERT_NO_FILE_INFO
	ARG_UNUSED(line);
	ARG_UNUSED(file);
	assert_post_action();
#else
	assert_post_action(file, line);
#endif
	for (;;);
}
