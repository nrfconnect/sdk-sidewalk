#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

add_subdirectory(sal)

if (CONFIG_SIDEWALK_USE_PREBUILT_LIBRARIES)
    add_subdirectory(app_utils)
    add_subdirectory(semtech)

    if (CONFIG_SIDEWALK_MFG_STORAGE)
        target_link_libraries(sid_pal_mfg_store_impl PUBLIC app_utils)
    endif()
endif()

add_subdirectory_ifdef(CONFIG_SIDEWALK_DEMO_PARSER demo)
