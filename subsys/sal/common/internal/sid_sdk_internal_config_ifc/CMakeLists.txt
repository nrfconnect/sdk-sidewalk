#
# Copyright 2022-2026 Amazon.com, Inc. or its affiliates. All rights reserved.
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

add_library(sid_sdk_internal_config_ifc INTERFACE)

target_compile_features(sid_sdk_internal_config_ifc INTERFACE c_std_99)

target_include_directories(sid_sdk_internal_config_ifc
  INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
)

target_sources(sid_sdk_internal_config_ifc
  INTERFACE
    sid_sdk_internal_config.h
)

# sid_sdk_gw_config_ifc is intentionally omitted: sid_sdk_gw_config.h is only
# pulled in for gateway builds (SID_GW_BUILD / GW_SUPPORT), which are not part
# of this add-on.
target_link_libraries(sid_sdk_internal_config_ifc
  INTERFACE
    sid_sdk_config_ifc
    sid_utils_ifc
)
