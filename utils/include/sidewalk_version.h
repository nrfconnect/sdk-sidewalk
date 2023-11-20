/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <stdint.h>
#include <stddef.h>

extern const char *const sidewalk_version_common_commit;
extern const size_t sidewalk_version_component_count;
extern const char *const build_time_stamp;

extern const char *const sidewalk_version_component_name[];
extern const char *const sidewalk_version_component[];

#define NAME_COLUMN_WIDTH "20"

#if defined(CONFIG_LOG)
#define PRINT_SIDEWALK_VERSION()                                                                   \
	LOG_PRINTK("----------------------------------------------------------------\n");          \
	for (int i = 0; i < sidewalk_version_component_count; i++) {                               \
		LOG_PRINTK("%-" NAME_COLUMN_WIDTH "s %s\n", sidewalk_version_component_name[i],    \
			   sidewalk_version_component[i]);                                         \
	}                                                                                          \
	LOG_PRINTK("----------------------------------------------------------------\n");          \
	LOG_PRINTK("sidewalk_fork_point = %s\n", sidewalk_version_common_commit);                  \
	LOG_PRINTK("build time          = %s\n", build_time_stamp);                                \
	LOG_PRINTK("board               = %s\n", CONFIG_BOARD);                                    \
	LOG_PRINTK("----------------------------------------------------------------\n")
#else
#define PRINT_SIDEWALK_VERSION()
#endif /* CONFIG_LOG */
