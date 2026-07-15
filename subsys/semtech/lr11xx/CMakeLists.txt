#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

add_library(sid_pal_radio_lr11xx_impl STATIC
	lr11xx_radio.c
	lr11xx_hal.c
	lr11xx_radio_fsk.c
	lr11xx_radio_lora.c
	lr11xx_halo.c
	semtech/lr11xx_bootloader.c
	semtech/lr11xx_crypto_engine.c
	semtech/lr11xx_driver_version.c
	semtech/lr11xx_gnss.c
	semtech/lr11xx_lr_fhss.c
	semtech/lr11xx_radio.c
	semtech/lr11xx_radio_timings.c
	semtech/lr11xx_rttof.c
	semtech/lr11xx_regmem.c
	semtech/lr11xx_system.c
	semtech/lr11xx_wifi.c
)

target_include_directories(sid_pal_radio_lr11xx_impl PRIVATE
	${SEMTECH_COMMON_INCLUDES}
	${LR11XX_COMMON_INCLUDES}
	${CMAKE_CURRENT_SOURCE_DIR}
	${SID_PAL_PUBLIC_INCLUDES}
)

target_compile_definitions(sid_pal_radio_lr11xx_impl PRIVATE
	DUAL_LINK_SUPPORT=1
	RTOS_ZEPHYR=1
	LR11XX_DISABLE_WARNINGS=1
	SMTC_MODEM_HAL_IRQ_FROM_SID_PAL
	RP2_103
	NUMBER_OF_STACKS=1
	LR11XX
	REGION_US_915
)

target_link_libraries(sid_pal_radio_lr11xx_impl PUBLIC
	smtc_lbm
	semtech_radio_ifc
	sid_pal_radio_ifc
	sid_pal_serial_bus_ifc
	sid_pal_gpio_ifc
	sid_pal_delay_ifc
	sid_pal_log_ifc
	sid_pal_timer_ifc
	sid_pal_critical_region_ifc
	sid_pal_mfg_store_ifc
	sid_900_cfg_ifc
	sid_time_ops
	sid_clock_ifc
	zephyr_interface
)
