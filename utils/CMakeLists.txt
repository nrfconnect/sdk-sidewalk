#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

zephyr_include_directories(include)
add_subdirectory(src)

add_subdirectory_ifdef(CONFIG_SIDEWALK_FILE_TRANSFER scratch_buffer)
add_subdirectory_ifdef(CONFIG_SIDEWALK_FILE_TRANSFER dfu_multi_image_utils)
