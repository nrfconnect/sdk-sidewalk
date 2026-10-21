#
# Copyright 2021-2026 Amazon.com, Inc. or its affiliates. All rights reserved.
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

add_library(sid_pal_log_ifc INTERFACE)

target_include_directories(sid_pal_log_ifc
  INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
)

target_sources(sid_pal_log_ifc
  INTERFACE

    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/sid_pal_log_ifc.h>
    $<INSTALL_INTERFACE:sid_pal_log_ifc.h>
)

set_target_properties(sid_pal_log_ifc PROPERTIES PUBLIC_HEADER "${CMAKE_CURRENT_LIST_DIR}/sid_pal_log_ifc.h")

# sid_log_control is intentionally omitted: the SDK ships only the header-only
# sid_log_control_ifc target, and sid_log_control_get_current_log_level() is
# implemented by this add-on in subsys/sal/sid_pal/src/sid_log.c.
