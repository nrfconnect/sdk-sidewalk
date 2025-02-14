#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

if (CONFIG_SIDEWALK_USE_PREBUILTS)
    zephyr_include_directories(sid_ifc)
    zephyr_include_directories(sid_pal_ifc)
    zephyr_include_directories(sid_time_ops)
endif()

zephyr_include_directories(sid_pal_types)

add_subdirectory_ifdef(CONFIG_SIDEWALK_ON_DEV_CERT sid_on_dev_cert)
