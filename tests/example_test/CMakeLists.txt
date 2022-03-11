#
# Copyright (c) 2019 Nordic Semiconductor
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(example_test)

# generate runner for the test
test_runner_generate(src/example_test.c)

# add test file
target_sources(app PRIVATE src/example_test.c)
target_include_directories(app PRIVATE .)
