/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

// clang-format off

#ifndef _APP_VERSION_H_
#define _APP_VERSION_H_

/*  values come from cmake/version.cmake
 * BUILD_VERSION related  values will be 'git describe',
 * alternatively user defined BUILD_VERSION.
 */

/* #undef ZEPHYR_VERSION_CODE */
/* #undef ZEPHYR_VERSION */

#define APPVERSION                   0x2076300
#define APP_VERSION_NUMBER           0x20763
#define APP_VERSION_MAJOR            2
#define APP_VERSION_MINOR            7
#define APP_PATCHLEVEL               99
#define APP_TWEAK                    0
#define APP_VERSION_STRING           "2.7.99"
#define APP_VERSION_EXTENDED_STRING  "2.7.99+0"
#define APP_VERSION_TWEAK_STRING     "2.7.99+0"

#define APP_BUILD_VERSION v2.7.0-32-g9ea02813749e


#endif /* _APP_VERSION_H_ */

// clang-format on
