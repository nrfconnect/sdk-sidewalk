#
# Copyright (c) 2022 Nordic Semiconductor
#
# SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
#

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

#add_definitions(-DMBEDTLS_USE_PSA_CRYPTO)

file(COPY ${ZEPHYR_BASE}/../modules/crypto/mbedtls/include/mbedtls DESTINATION ${ZEPHYR_BASE}/../sidewalk/include)
file(COPY ${ZEPHYR_BASE}/../modules/crypto/mbedtls/include/psa DESTINATION ${ZEPHYR_BASE}/../sidewalk/include)

# fff.h
include_directories(${ZEPHYR_BASE}/subsys/testsuite/include)

#include_directories(${ZEPHYR_BASE}/../sidewalk/include/psa)
#cmock_handle(${ZEPHYR_BASE}/../sidewalk/include/psa/crypto.h)
#cmock_handle(${ZEPHYR_BASE}/../sidewalk/include/psa/crypto_struct.h)

# add test file
FILE(GLOB app_sources src/*.c)
target_sources(app PRIVATE ${app_sources})
target_include_directories(app PRIVATE .)

# generate runner for the test
test_runner_generate(${app_sources})

add_custom_command(TARGET app POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E rm -r ${ZEPHYR_BASE}/../sidewalk/include/psa
    COMMAND ${CMAKE_COMMAND} -E rm -r ${ZEPHYR_BASE}/../sidewalk/include/mbedtls
)
