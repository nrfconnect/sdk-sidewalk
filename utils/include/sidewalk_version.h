/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <stdint.h>
#include <stddef.h>
#include <sid_sdk_version.h>
#include <app_version.h>

#ifdef CONFIG_DT_HAS_SEMTECH_LR1110_ENABLED
#define SHIELD_NAME "Semtech lr1110"
#elif defined(CONFIG_DT_HAS_SEMTECH_SX1262_ENABLED) | defined(CONFIG_DT_HAS_SEMTECH_SX1262_NEW_ENABLED)
#define SHIELD_NAME "Semtech sx1262"
#endif

#ifndef SHIELD_NAME
#define SHIELD_NAME "none"
#endif

#if defined(CONFIG_LOG)
#define PRINT_SIDEWALK_VERSION()                                                                   \
	LOG_PRINTK("----------------------------------------------------------------\n");          \
	LOG_PRINTK("Sidewalk SDK        = %d.%d.%d.%d\n", SID_SDK_MAJOR_VERSION,                   \
		   SID_SDK_MINOR_VERSION, SID_SDK_PATCH_VERSION, SID_SDK_BUILD_VERSION);           \
	LOG_PRINTK("APP_BUILD_VERSION   = %s\n", STRINGIFY(APP_BUILD_VERSION));                    \
	LOG_PRINTK("APP_NAME            = %s\n", CONFIG_SIDEWALK_APPLICATION_NAME);                \
	LOG_PRINTK("build time          = %s %s\n", __DATE__, __TIME__);                           \
	LOG_PRINTK("board               = %s\n", CONFIG_BOARD_TARGET);                             \
	LOG_PRINTK("shield              = %s\n", SHIELD_NAME);                                     \
	LOG_PRINTK("----------------------------------------------------------------\n")
#else
#define PRINT_SIDEWALK_VERSION()
#endif /* CONFIG_LOG */
