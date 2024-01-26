#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

set(SID_LIB_DIR ${ZEPHYR_BASE}/../sidewalk/lib/cortex-m33/ble_only)

zephyr_library_link_libraries(${SID_LIB_DIR}/libsid_protocol_ble.a)
