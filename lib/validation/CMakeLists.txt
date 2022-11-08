#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
#

set(SID_TEST_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(SID_900_LIB_DIR ${ZEPHYR_BASE}/../sidewalk/lib/lora_fsk)
set(GCC_LIB_DIR ${ZEPHYR_SDK_INSTALL_DIR}/arm-zephyr-eabi/lib/gcc/arm-zephyr-eabi)
find_library(LIB_GCC NAMES gcc libgcc.a HINTS ${GCC_LIB_DIR}/* REQUIRED)

# test libs
set(SID_TEST_LIBS	
	${SID_TEST_LIB_DIR}/libsid_tests.a
	${SID_TEST_LIB_DIR}/libsall_security_tests.a
	${SID_TEST_LIB_DIR}/libsdb_mac_tests.a
	${SID_TEST_LIB_DIR}/libsid_pal_crypto_tests.a
	${SID_TEST_LIB_DIR}/libsid_pal_storage_kv_tests.a
	${SID_TEST_LIB_DIR}/libsid_pal_timer_tests.a
	${SID_TEST_LIB_DIR}/libsid_protocol_mac_tests.a
	${SID_TEST_LIB_DIR}/libsid_security_tests.a
	${SID_TEST_LIB_DIR}/libsdb_mac.a
	${SID_TEST_LIB_DIR}/libsall_security.a
	)
list(APPEND SID_LIBS ${SID_TEST_LIBS})

# sidewalk libs
set(SID_PROTOCOL_LIBS
	${SID_900_LIB_DIR}/libsid_api.a
	${SID_900_LIB_DIR}/libsid_900_network_impl.a
	${SID_900_LIB_DIR}/libsid_ble_network_impl.a
	${SID_900_LIB_DIR}/libsid_protocol_mac.a
	${SID_900_LIB_DIR}/libsid_protocol_utils.a
	${SID_900_LIB_DIR}/libsid_protocol_dcr.a
	${SID_900_LIB_DIR}/libsid_protocol_phy.a
	${SID_900_LIB_DIR}/libsid_protocol_metrics.a
	${SID_900_LIB_DIR}/libsid_pal_radio_sx126x_impl.a
	${SID_900_LIB_DIR}/libsid_protocol_regulatory.a
	${SID_900_LIB_DIR}/libsall_link_metrics.a
	${SID_900_LIB_DIR}/libsid_rng.a
	${SID_900_LIB_DIR}/libsid_network_address.a
	${SID_900_LIB_DIR}/libsid_event_queue.a
	${SID_900_LIB_DIR}/libsall_mgm.a
	${SID_900_LIB_DIR}/libsall_mgm_core.a
	${SID_900_LIB_DIR}/libsid_parser_utils.a
	${SID_900_LIB_DIR}/libsid_memory_pool.a
	${SID_900_LIB_DIR}/libsid_clock.a
	${SID_900_LIB_DIR}/libsid_ama_adapter_impl.a
	${SID_900_LIB_DIR}/libace_ama.a
	${SID_900_LIB_DIR}/libext_nanopb.a
	${SID_900_LIB_DIR}/libsall_security.a
	${SID_900_LIB_DIR}/libsall.a
	${SID_900_LIB_DIR}/libsid_asl_parser.a
	${SID_900_LIB_DIR}/libsid_flex_parser.a
	${SID_900_LIB_DIR}/libsid_parser_utils.a
	${SID_900_LIB_DIR}/libsid_time_ops.a
	${SID_TEST_LIB_DIR}/libsdb_mac.a
	${SID_TEST_LIB_DIR}/libsall_security.a	
	${SID_TEST_LIB_DIR}/libsid_security.a
	${LIB_GCC}
)

list(APPEND SID_LIBS ${SID_PROTOCOL_LIBS})

# debug
message(STATUS "SID_LIBS: ${SID_LIBS}")

# link libraries
zephyr_link_libraries(${SID_LIBS})
