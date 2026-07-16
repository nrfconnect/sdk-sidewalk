#
# Copyright 2020-2024 Amazon.com, Inc. or its affiliates. All rights reserved.
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

add_library(sid_pal_crypto_ifc INTERFACE)

target_include_directories(sid_pal_crypto_ifc
  INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
)

target_sources(sid_pal_crypto_ifc
  INTERFACE
    sid_pal_crypto_ifc.h
)

target_link_libraries(sid_pal_crypto_ifc
  INTERFACE
    sid_error
)
