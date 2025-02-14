#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

if(CONFIG_SIDEWALK_BUILD)
    if (CONFIG_SIDEWALK_USE_PREBUILTS)
        add_subdirectory(lib)
    endif()
    add_subdirectory(subsys)
    add_subdirectory(utils)
endif()

if(CONFIG_SOC_SERIES_NRF54LX AND CONFIG_SETTINGS_NVS)
    message(WARNING "
    The nRF54L series is not optimized for the NVS settings backend due to RRAM specifications.
    Consider using the CONFIG_SETTINGS_ZMS Kconfig option, which is recommended for this type of non-volatile memory.")
endif()
