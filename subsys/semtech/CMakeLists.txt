#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

zephyr_include_directories(include)
zephyr_include_directories_ifdef(CONFIG_SIDEWALK_SUBGHZ_RADIO_SX126X sx126x/include)

zephyr_compile_definitions(RP2_103 NUMBER_OF_STACKS=1 LR11XX REGION_US_915)
zephyr_include_directories_ifdef(CONFIG_SIDEWALK_SUBGHZ_RADIO_LR1110
    lr1110/include
    lr1110_location/include
    SWDR007_v1-3-1
    SWDR007_v1-3-1/swdr007_silabs_sid_pal_lr11xx
    SWDR007_v1-3-1/swdr007_silabs_sid_pal_lr11xx/config
    SWDR007_v1-3-1/swdr007_silabs_sid_pal_lr11xx/RALBSP
    SWDR007_v1-3-1/swdr007_silabs_sid_pal_lr11xx/lr11xx_hal
    SWDR007_v1-3-1/swdr007_silabs_lbm_lib/smtc_modem_core/radio_drivers/lr11xx_driver/src

    SWDR007_v1-3-1/swdr007_silabs_lbm_lib/smtc_modem_core/smtc_ral/src
    SWDR007_v1-3-1/swdr007_silabs_lbm_lib/smtc_modem_core/smtc_ralf/src
    SWDR007_v1-3-1/swdr007_silabs_lbm_lib/smtc_modem_core/geolocation_services
    SWDR007_v1-3-1/swdr007_silabs_lbm_lib/smtc_modem_core
    SWDR007_v1-3-1/swdr007_silabs_lbm_lib/smtc_modem_api
    SWDR007_v1-3-1/swdr007_silabs_lbm_lib/smtc_modem_hal
    SWDR007_v1-3-1/swdr007_silabs_lbm_lib/smtc_modem_core/radio_drivers/sx126x_driver/src
    SWDR007_v1-3-1/swdr007_silabs_lbm_lib/smtc_modem_core/lorawan_manager
    SWDR007_v1-3-1/swdr007_silabs_lbm_lib/smtc_modem_core/lr1mac/src
    SWDR007_v1-3-1/swdr007_silabs_lbm_lib/smtc_modem_core/smtc_modem_crypto/smtc_secure_element
    SWDR007_v1-3-1/swdr007_silabs_lbm_lib/smtc_modem_core/lr1mac/src/smtc_real/src
    SWDR007_v1-3-1/swdr007_silabs_lbm_lib/smtc_modem_core/radio_planner/src
    SWDR007_v1-3-1/swdr007_silabs_lbm_lib/smtc_modem_core/lr1mac/src/services
    SWDR007_v1-3-1/swdr007_silabs_lbm_lib/smtc_modem_core/modem_supervisor
    SWDR007_v1-3-1/swdr007_silabs_lbm_lib/smtc_modem_core/modem_utilities
    SWDR007_v1-3-1/swdr007_silabs_lbm_lib/smtc_modem_core/lorawan_packages/lorawan_certification
    SWDR007_v1-3-1/swdr007_silabs_lbm_lib/smtc_modem_core/lorawan_api
    SWDR007_v1-3-1/swdr007_silabs_lbm_lib/smtc_modem_core/smtc_modem_crypto
    SWDR007_v1-3-1/swdr007_silabs_lbm_lib
    SWDR007_v1-3-1/swdr007_silabs_lbm_lib/smtc_modem_core/modem_config
    SWDR007_v1-3-1/swdr007_silabs_lbm_lib/smtc_modem_core/lr1mac/src/services/smtc_multicast
    SWDR007_v1-3-1/swdr007_silabs_lbm_lib/smtc_modem_core/lr1mac
    SWDR007_v1-3-1/swdr007_silabs_lbm_lib/smtc_modem_core/modem_services/cloud_dm_package
    SWDR007_v1-3-1/swdr007_silabs_lbm_lib/smtc_modem_core/modem_services
    SWDR007_v1-3-1/swdr007_silabs_lbm_lib/smtc_modem_core/modem_services/lfu_service
    SWDR007_v1-3-1/swdr007_silabs_lbm_lib/smtc_modem_core/lorawan_packages/application_layer_clock_synchronization
)

add_subdirectory_ifdef(CONFIG_SIDEWALK_SUBGHZ_RADIO_SX126X sx126x)
add_subdirectory_ifdef(CONFIG_SIDEWALK_SUBGHZ_RADIO_SX126X sx126x_location)
# add_subdirectory_ifdef(CONFIG_SIDEWALK_SUBGHZ_RADIO_LR1110 lr1110)
add_subdirectory_ifdef(CONFIG_SIDEWALK_SUBGHZ_RADIO_LR1110 lr11xx)
add_subdirectory_ifdef(CONFIG_SIDEWALK_SUBGHZ_RADIO_LR1110 lr1110_location)

