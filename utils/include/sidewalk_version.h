/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <stdint.h>
#include <stddef.h>
#include <sid_sdk_version.h>

extern const size_t sidewalk_version_component_count;
extern const char *const build_time_stamp;

extern const char *const sidewalk_version_component_name[];
extern const char *const sidewalk_version_component[];

#define NAME_COLUMN_WIDTH "20"

#if defined(CONFIG_LOG)
#define PRINT_SIDEWALK_VERSION()                                                                   \
	LOG_PRINTK("----------------------------------------------------------------\n");          \
	LOG_PRINTK("Sidewalk SDK        = %d.%d.%d.%d\n", SID_SDK_MAJOR_VERSION,                   \
		   SID_SDK_MINOR_VERSION, SID_SDK_PATCH_VERSION, SID_SDK_BUILD_VERSION);           \
	LOG_PRINTK("build time          = %s\n", build_time_stamp);                                \
	LOG_PRINTK("board               = %s\n", CONFIG_BOARD_TARGET);                             \
	LOG_PRINTK("----------------------------------------------------------------\n")
#else
#define PRINT_SIDEWALK_VERSION()
#endif /* CONFIG_LOG */
