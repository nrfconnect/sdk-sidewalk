#
# Copyright 2025 Amazon.com, Inc. or its affiliates. All rights reserved.
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

add_library(sid_time_sync_config INTERFACE)

target_compile_features(sid_time_sync_config INTERFACE c_std_99)

target_include_directories(sid_time_sync_config
  INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
)

target_sources(sid_time_sync_config
  INTERFACE
    sid_time_sync_config.h
)

install(TARGETS sid_time_sync_config OPTIONAL)
