#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
#

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
set(SIDEAWLK_BASE $ENV{ZEPHYR_BASE}/../sidewalk)

project(sidewalk_test_gpio)
add_definitions(-DNRF52840_XXAA)

cmock_handle($ENV{ZEPHYR_BASE}/include/zephyr/drivers/gpio.h zephyr/drivers)
cmock_handle($ENV{ZEPHYR_BASE}/../modules/hal/nordic/nrfx/mdk/nrf52840_peripherals.h)
cmock_handle($ENV{ZEPHYR_BASE}/../modules/hal/nordic/nrfx/mdk/nrf_peripherals.h)
cmock_handle(${SIDEAWLK_BASE}/subsys/sal/sid_pal/include/sid_gpio_utils.h zephyr/drivers)
cmock_handle(${SIDEAWLK_BASE}/subsys/sal/sid_pal/include/sid_gpio_irq.h)
cmock_handle(${SIDEAWLK_BASE}/subsys/sal/sid_pal/include/sid_gpio_irq_handler.h)


FILE(GLOB app_sources src/*.c ${SIDEAWLK_BASE}/subsys/sal/sid_pal/src/sid_gpio.c)
set(SOURCES ${app_sources})

target_include_directories(app PRIVATE ${SIDEAWLK_BASE}/subsys/sal/common/sid_pal_ifc)
target_include_directories(app PRIVATE ${SIDEAWLK_BASE}/subsys/sal/common/sid_ifc)
target_include_directories(app PRIVATE ${SIDEAWLK_BASE}/subsys/sal/sid_pal/include)
target_sources(app PRIVATE ${app_sources})

# generate runner for the test
test_runner_generate(src/main.c)
