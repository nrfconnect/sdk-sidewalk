#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
#

zephyr_library_sources_ifdef(CONFIG_SIDEWALK_ASSERT sid_assert.c)
zephyr_library_sources_ifdef(CONFIG_SIDEWALK_BLE sid_ble_adapter.c sid_ble_service.c)
zephyr_library_sources_ifdef(CONFIG_SIDEWALK_CRYPTO sid_crypto.c)
zephyr_library_sources_ifdef(CONFIG_SIDEWALK_LOG sid_log.c)
zephyr_library_sources_ifdef(CONFIG_SIDEWALK_MFG_STORAGE sid_mfg_storage.c)
zephyr_library_sources_ifdef(CONFIG_SIDEWALK_STORAGE sid_storage.c)
zephyr_library_sources_ifdef(CONFIG_SIDEWALK_TIMER sid_timer.c)
zephyr_library_sources_ifdef(CONFIG_SIDEWALK_UPTIME sid_uptime.c)
