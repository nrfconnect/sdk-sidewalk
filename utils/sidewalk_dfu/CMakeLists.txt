#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

zephyr_library()

zephyr_library_sources_ifdef(CONFIG_SIDEWALK_DFU_SERVICE_BLE nordic_dfu.c)
zephyr_library_sources_ifdef(CONFIG_SIDEWALK_DFU_IMG_UTILS nordic_dfu_img.c)
