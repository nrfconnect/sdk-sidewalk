/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/** @file sid_assert.c
 *  @brief Assert implementation.
 */

#include <sid_pal_assert_ifc.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(sid_assert, LOG_LEVEL_ERR);

void sid_pal_assert(int line, const char *file)
{
	LOG_ERR("PAL_ASSERT failed at line: %d, file: %s, return address is %p", line, file,
		(void *)__builtin_return_address(0));
#if defined(CONFIG_ASSERT)
#if defined(CONFIG_ASSERT_NO_FILE_INFO)
	ARG_UNUSED(line);
	ARG_UNUSED(file);
	assert_post_action();
#else
	assert_post_action(file, line);
#endif /* CONFIG_ASSERT_NO_FILE_INFO */
#endif /* CONFIG_ASSERT */
	for (;;)
		;
}
