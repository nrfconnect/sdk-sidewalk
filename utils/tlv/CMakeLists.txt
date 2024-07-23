#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

zephyr_library()

zephyr_library_sources(
    tlv.c
)

zephyr_library_sources_ifdef(SIDEWALK_TLV_FLASH
    tlv_flash_storage_impl.c
)
zephyr_library_sources_ifdef(SIDEWALK_TLV_RAM
    tlv_ram_storage_impl.c
)
