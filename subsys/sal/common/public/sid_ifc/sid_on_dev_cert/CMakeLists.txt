#
# Copyright 2023 Amazon.com, Inc. or its affiliates. All rights reserved.
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

add_library(sid_on_dev_cert_ifc INTERFACE)

target_compile_features(sid_on_dev_cert_ifc INTERFACE c_std_99)

target_include_directories(sid_on_dev_cert_ifc
  INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
)

target_sources(sid_on_dev_cert_ifc
  INTERFACE
  sid_on_dev_cert.h
)

target_link_libraries(sid_on_dev_cert_ifc
  INTERFACE
    sid_error
)
