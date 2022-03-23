#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
#

zephyr_library()

zephyr_library_sources(
  src/log.c
  src/sid_storage.c
  src/assert.c
  src/sid_mfg_storage.c
  src/sid_timer.c
)
