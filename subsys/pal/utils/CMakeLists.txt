#
# Copyright (c) 2023 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
#

zephyr_include_directories(.)

zephyr_library_sources_ifdef(CONFIG_SIDEWALK_UPTIME zephyr_time.c)
zephyr_library_sources_ifdef(CONFIG_SIDEWALK_ACE_OSAL_ZEPHYR osal_alloc.c)
