#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
#

zephyr_library()

add_subdirectory(utils)

zephyr_library_sources(
  src/sid_assert.c
  src/sid_log.c
  src/sid_mfg_storage.c
  src/sid_storage.c
  src/sid_timer.c
  src/sid_uptime.c
)
