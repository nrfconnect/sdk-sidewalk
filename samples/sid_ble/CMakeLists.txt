#
# Copyright (c) 2022 Nordic Semiconductor
#
# SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
#

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(sidewalk)

target_sources(app PRIVATE src/main.c)

target_link_libraries(app PRIVATE ${ZEPHYR_BASE}/../sidewalk/lib/libsid_api.a)
target_link_libraries(app PRIVATE ${ZEPHYR_BASE}/../sidewalk/lib/libsid_rng.a)
target_link_libraries(app PRIVATE ${ZEPHYR_BASE}/../sidewalk/lib/libsid_network_address.a)
target_link_libraries(app PRIVATE ${ZEPHYR_BASE}/../sidewalk/lib/libsid_time_ops.a)
target_link_libraries(app PRIVATE ${ZEPHYR_BASE}/../sidewalk/lib/libsid_event_queue.a)
target_link_libraries(app PRIVATE ${ZEPHYR_BASE}/../sidewalk/lib/libsall_mgm_core.a)
target_link_libraries(app PRIVATE ${ZEPHYR_BASE}/../sidewalk/lib/libsid_security.a)
target_link_libraries(app PRIVATE ${ZEPHYR_BASE}/../sidewalk/lib/libsid_parser_utils.a)
target_link_libraries(app PRIVATE ${ZEPHYR_BASE}/../sidewalk/lib/libsid_memory_pool.a)
target_link_libraries(app PRIVATE ${ZEPHYR_BASE}/../sidewalk/lib/libsid_clock.a)
# in ncs pal ?
# target_link_libraries(app PRIVATE ${ZEPHYR_BASE}/../sidewalk/lib/libsid_ble_network_impl.a)
# ble adapter in ncs pal ?
# target_link_libraries(app PRIVATE ${ZEPHYR_BASE}/../sidewalk/lib/libsid_ama_adapter_impl.a)
# uses VFP register arguments
# target_link_libraries(app PRIVATE ${ZEPHYR_BASE}/../sidewalk/lib/libace_ama.a)
# uses VFP register arguments
# target_link_libraries(app PRIVATE ${ZEPHYR_BASE}/../sidewalk/lib/libext_nanopb.a)
target_link_libraries(app PRIVATE ${ZEPHYR_BASE}/../sidewalk/lib/libsall_security.a)
target_link_libraries(app PRIVATE ${ZEPHYR_BASE}/../sidewalk/lib/libsall.a)
target_link_libraries(app PRIVATE ${ZEPHYR_BASE}/../sidewalk/lib/libsid_asl_parser.a)
target_link_libraries(app PRIVATE ${ZEPHYR_BASE}/../sidewalk/lib/libsid_flex_parser.a)
target_link_libraries(app PRIVATE ${ZEPHYR_BASE}/../sidewalk/lib/libsid_parser_utils.a)
