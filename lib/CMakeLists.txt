#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

if(CONFIG_SIDEWALK)
    if(CONFIG_CPU_CORTEX_M33)
        add_subdirectory(cortex-m33)
    endif(CONFIG_CPU_CORTEX_M33)
endif(CONFIG_SIDEWALK)
