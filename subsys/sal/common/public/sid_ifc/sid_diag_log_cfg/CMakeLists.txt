#
# Copyright 2025-2026 Amazon.com, Inc. or its affiliates. All rights reserved.
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

add_library(sid_diag_log_cfg INTERFACE)

target_compile_features(sid_diag_log_cfg INTERFACE c_std_99)

target_include_directories(sid_diag_log_cfg
  INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
)

target_sources(sid_diag_log_cfg
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/sid_diag_log_cfg.h>
    $<INSTALL_INTERFACE:sid_diag_log_cfg.h>
)

set_target_properties(sid_diag_log_cfg PROPERTIES PUBLIC_HEADER "${CMAKE_CURRENT_LIST_DIR}/sid_diag_log_cfg.h")

install(TARGETS sid_diag_log_cfg OPTIONAL)
