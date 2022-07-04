#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
#

if (CONFIG_SIDEWALK)
	set(SID_LIB_DIR ${ZEPHYR_BASE}/../sidewalk/lib)

	set(GCC_LIB_DIR ${ZEPHYR_SDK_INSTALL_DIR}/arm-zephyr-eabi/lib/gcc/arm-zephyr-eabi)
	find_library(LIB_GCC NAMES gcc libgcc.a HINTS ${GCC_LIB_DIR}/* REQUIRED)

	set(SID_LIBS
		${SID_LIB_DIR}/libsid_api.a
		${SID_LIB_DIR}/libsid_900_network_impl.a
		${SID_LIB_DIR}/libsid_ble_network_impl.a
		${SID_LIB_DIR}/libsid_protocol_mac.a
		${SID_LIB_DIR}/libsid_protocol_utils.a
		${SID_LIB_DIR}/libsid_protocol_dcr.a
		${SID_LIB_DIR}/libsid_protocol_phy.a
		${SID_LIB_DIR}/libsid_protocol_metrics.a
		${SID_LIB_DIR}/libsid_pal_radio_sx126x_impl.a
		${SID_LIB_DIR}/libsid_protocol_regulatory.a
		${SID_LIB_DIR}/libsall_link_metrics.a
		${SID_LIB_DIR}/libsid_rng.a
		${SID_LIB_DIR}/libsid_network_address.a
		${SID_LIB_DIR}/libsid_event_queue.a
		${SID_LIB_DIR}/libsall_mgm.a
		${SID_LIB_DIR}/libsall_mgm_core.a
		${SID_LIB_DIR}/libsid_parser_utils.a
		${SID_LIB_DIR}/libsid_memory_pool.a
		${SID_LIB_DIR}/libsid_clock.a
		${SID_LIB_DIR}/libsid_ama_adapter_impl.a
		${SID_LIB_DIR}/libace_ama.a
		${SID_LIB_DIR}/libext_nanopb.a
		${SID_LIB_DIR}/libsall_security.a
		${SID_LIB_DIR}/libsid_security.a
		${SID_LIB_DIR}/libsall.a
		${SID_LIB_DIR}/libsid_asl_parser.a
		${SID_LIB_DIR}/libsid_flex_parser.a
		${SID_LIB_DIR}/libsid_parser_utils.a
		${SID_LIB_DIR}/libsid_time_ops.a
		${LIB_GCC}
	)

	zephyr_link_libraries(${SID_LIBS})
endif(CONFIG_SIDEWALK)
