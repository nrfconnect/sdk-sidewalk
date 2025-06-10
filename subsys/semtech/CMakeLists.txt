#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

zephyr_include_directories(include)

if(CONFIG_RADIO_SX126X)
    zephyr_include_directories(sx126x/include)
    add_subdirectory(sx126x)
endif()
