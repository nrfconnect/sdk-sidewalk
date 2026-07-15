#
# Copyright 2023-2024 Amazon.com, Inc. or its affiliates. All rights reserved.
#
# AMAZON PROPRIETARY/CONFIDENTIAL
#
# You may not use this file except in compliance with the terms and conditions
# set forth in the accompanying LICENSE.txt file. This file is a
# Modifiable File, as defined in the accompanying LICENSE.txt file.
#
# THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
# DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
# IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
#

add_library(ace STATIC)
target_compile_features(ace PRIVATE c_std_99)
target_compile_options(ace PRIVATE -Wno-unused-parameter)

target_sources(ace
  PRIVATE
    osal_alloc.c
  PUBLIC
    include/ace/ace_config.h
    include/ace/ace_log.h
    include/ace/ace_modules.h
    include/ace/ace_status.h
    include/ace/osal_alloc.h
    include/ace/uthash.h
)

target_include_directories(ace
  PUBLIC
    include
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../app_utils/hal/include
)

target_link_libraries(ace PRIVATE zephyr_interface)

target_compile_definitions(ace
  PUBLIC
    ACE_LOG_MIN_LVL=6
)

install(TARGETS ace OPTIONAL)
