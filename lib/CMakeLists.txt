#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

if (CONFIG_SIDEWALK)
    if(CONFIG_SOC_SERIES_NRF52X)
        add_subdirectory(cortex-m4)
    else()
        add_subdirectory(cortex-m33)
    endif()
endif(CONFIG_SIDEWALK)
