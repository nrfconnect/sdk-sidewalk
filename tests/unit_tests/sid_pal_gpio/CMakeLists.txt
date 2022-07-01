#
# Copyright (c) 2022 Nordic Semiconductor
#
# SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
#

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(sidewalk_test_gpio)
add_definitions(-DNRF52840_XXAA)

cmock_handle($ENV{ZEPHYR_BASE}/../zephyr/include/zephyr/drivers/gpio.h drivers)
cmock_handle(${ZEPHYR_BASE}/../modules/hal/nordic/nrfx/mdk/nrf52840_peripherals.h)
cmock_handle(${ZEPHYR_BASE}/../modules/hal/nordic/nrfx/mdk/nrf_peripherals.h)
cmock_handle(${ZEPHYR_BASE}/../sidewalk/include/sid_gpio_utils.h drivers)
cmock_handle(${ZEPHYR_BASE}/../sidewalk/include/sid_gpio_irq.h)
cmock_handle(${ZEPHYR_BASE}/../sidewalk/include/sid_gpio_irq_handler.h)

FILE(GLOB app_sources src/*.c $ENV{ZEPHYR_BASE}/../sidewalk/pal/src/sid_gpio.c)
set(SOURCES ${app_sources})

target_sources(app PRIVATE ${app_sources})

# generate runner for the test
test_runner_generate(src/main.c)