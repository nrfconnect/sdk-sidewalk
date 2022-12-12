#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
#

zephyr_library()
zephyr_library_sources(
    memory.c
    reset.c
)

if(CONFIG_SIDEWALK_HAL_BUTTONS)
	zephyr_library_sources(buttons.c)
	zephyr_compile_definitions(HAL_BUTTONS_LOG_LEVEL=${CONFIG_SIDEWALK_LOG_LEVEL})
endif()

if (CONFIG_SIDEWALK_CLI)
    zephyr_library_sources(shell.c)
    zephyr_include_directories(${ZEPHYR_BASE}/../sidewalk/lib/include/)
    zephyr_include_directories(${ZEPHYR_BASE}/../sidewalk/samples/template/include)
endif()
