#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
#

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(sidewalk_test_gpio_utils)
add_definitions(-DNRF52840_XXAA)

cmock_handle($ENV{ZEPHYR_BASE}/include/zephyr/drivers/gpio.h zephyr/drivers)

target_include_directories(app PRIVATE $ENV{ZEPHYR_BASE}/../modules/hal/nordic/nrfx/mdk)
target_include_directories(app PRIVATE $ENV{ZEPHYR_BASE}/../sidewalk/subsys/include)
FILE(GLOB app_sources src/*.c $ENV{ZEPHYR_BASE}/../sidewalk/subsys/pal/src/sid_gpio_utils.c)
set(SOURCES ${app_sources})

target_sources(app PRIVATE ${app_sources})
target_include_directories(app PRIVATE .)

# generate runner for the test
test_runner_generate(src/main.c)
