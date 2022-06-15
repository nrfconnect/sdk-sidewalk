#
# Copyright (c) 2022 Nordic Semiconductor
#
# SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
#

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(sidewalk_test_temperature)

cmock_handle($ENV{ZEPHYR_BASE}/../zephyr/include/zephyr/drivers/sensor.h drivers)
cmock_handle($ENV{ZEPHYR_BASE}/../zephyr/include/zephyr/device.h)

FILE(GLOB app_sources src/*.c $ENV{ZEPHYR_BASE}/../sidewalk/pal/src/sid_temperature.c)
set(SOURCES ${app_sources})

target_sources(app PRIVATE ${app_sources})
target_include_directories(app PRIVATE .)
set_property(SOURCE $ENV{ZEPHYR_BASE}/../sidewalk/pal/src/sid_temperature.c PROPERTY COMPILE_FLAGS
	"-include src/kconfig_mock.h")

# generate runner for the test
test_runner_generate(src/main.c)
