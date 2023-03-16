#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

set(SID_LIB_DIR ${ZEPHYR_BASE}/../sidewalk/lib/ble_only)

set(SID_LIBS
	${SID_LIB_DIR}/libsid_api.a
	${SID_LIB_DIR}/libsid_ble_network_impl.a
	${SID_LIB_DIR}/libsid_rng.a
	${SID_LIB_DIR}/libsid_network_address.a
	${SID_LIB_DIR}/libsid_event_queue.a
	${SID_LIB_DIR}/libsall_mgm_core.a
	${SID_LIB_DIR}/libsid_memory_pool.a
	${SID_LIB_DIR}/libsid_clock.a
	${SID_LIB_DIR}/libsid_ama_adapter_impl.a
	${SID_LIB_DIR}/libace_ama.a
	${SID_LIB_DIR}/libext_nanopb.a
	${SID_LIB_DIR}/libsall_security.a
	${SID_LIB_DIR}/libsid_security.a
	${SID_LIB_DIR}/libsid_msg_info_utils.a
	${SID_LIB_DIR}/libsall.a
	${SID_LIB_DIR}/libsid_asl_parser.a
	${SID_LIB_DIR}/libsid_flex_parser.a
	${SID_LIB_DIR}/libsid_parser_utils.a
	${SID_LIB_DIR}/libsid_time_ops.a
)

zephyr_link_libraries(${SID_LIBS})
