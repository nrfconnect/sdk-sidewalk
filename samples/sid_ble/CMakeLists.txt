#
# Copyright (c) 2022 Nordic Semiconductor
#
# SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
#

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(sidewalk)

set(sid_lib_dir ${ZEPHYR_BASE}/../sidewalk/lib)

set(sid_libs
    ${sid_lib_dir}/libsid_api.a
    ${sid_lib_dir}/libsid_rng.a
    ${sid_lib_dir}/libsid_network_address.a
    ${sid_lib_dir}/libsid_time_ops.a
    ${sid_lib_dir}/libsid_event_queue.a
    ${sid_lib_dir}/libsall_mgm_core.a
    ${sid_lib_dir}/libsid_parser_utils.a
    ${sid_lib_dir}/libsid_memory_pool.a
    ${sid_lib_dir}/libsid_clock.a
    ${sid_lib_dir}/libsid_ble_network_impl.a
    ${sid_lib_dir}/libsid_ama_adapter_impl.a
    ${sid_lib_dir}/libace_ama.a
    ${sid_lib_dir}/libext_nanopb.a
    ${sid_lib_dir}/libsall_security.a
    ${sid_lib_dir}/libsid_security.a
    ${sid_lib_dir}/libsall.a
    ${sid_lib_dir}/libsid_asl_parser.a
    ${sid_lib_dir}/libsid_flex_parser.a
    ${sid_lib_dir}/libsid_parser_utils.a
)

target_sources(app PRIVATE src/main.c)
target_sources(app PRIVATE src/assert_action.c)
target_sources(app PRIVATE src/ace_utils.c)

target_link_libraries(app PRIVATE ${sid_libs})
