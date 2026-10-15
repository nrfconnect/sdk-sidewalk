#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

if (CONFIG_SIDEWALK)
    if(CONFIG_SIDEWALK_PREBUILT_LIBRARIES_DEBUG)
        add_subdirectory(debug)
    else()
        add_subdirectory(release)
    endif()
endif(CONFIG_SIDEWALK)
