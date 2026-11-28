#
# Copyright (c) 2026 Seeed Technology Co., Ltd.
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#
# Builds sid_pal_radio_lr20xx_impl: the Sidewalk PAL over Semtech lr20xx_driver.
# Mirrors lr11xx/CMakeLists.txt. The LR2021 has no GNSS, so smtc_lbm
# (location) is not linked.
#

# Vendored lr20xx_driver sources (all 20; --gc-sections drops unused
# modulations like flrc/ook/oqpsk/wi_sun/wm_bus/z_wave/bluetooth_le/bpsk/lr_fhss/rttof).
set(LR20XX_DRIVER_SOURCES
	semtech/src/lr20xx_driver_version.c
	semtech/src/lr20xx_patch.c
	semtech/src/lr20xx_pram_load.c
	semtech/src/lr20xx_radio_bluetooth_le.c
	semtech/src/lr20xx_radio_bpsk.c
	semtech/src/lr20xx_radio_common.c
	semtech/src/lr20xx_radio_fifo.c
	semtech/src/lr20xx_radio_flrc.c
	semtech/src/lr20xx_radio_fsk.c
	semtech/src/lr20xx_radio_lora.c
	semtech/src/lr20xx_radio_lr_fhss.c
	semtech/src/lr20xx_radio_ook.c
	semtech/src/lr20xx_radio_oqpsk_15_4.c
	semtech/src/lr20xx_radio_wi_sun.c
	semtech/src/lr20xx_radio_wm_bus.c
	semtech/src/lr20xx_radio_z_wave.c
	semtech/src/lr20xx_regmem.c
	semtech/src/lr20xx_rttof.c
	semtech/src/lr20xx_system.c
	semtech/src/lr20xx_workarounds.c
)

add_library(sid_pal_radio_lr20xx_impl STATIC
	lr20xx_radio.c
	lr20xx_hal.c
	lr20xx_ral_bsp.c
	ral/ral_lr20xx.c
	${LR20XX_DRIVER_SOURCES}
)

# lr20xx_hal.c directly includes <zephyr/drivers/spi.h> + <zephyr/kernel.h>,
# which pull in the generated <zephyr/offsets.h> (from offsets.c). Plain static
# libs don't inherit the offsets build-order dep from zephyr_interface, so add
# it explicitly — otherwise offsets.h is missing when this lib compiles early.
add_dependencies(sid_pal_radio_lr20xx_impl offsets_h)

target_include_directories(sid_pal_radio_lr20xx_impl PRIVATE
	${SEMTECH_COMMON_INCLUDES}
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/semtech/inc
	${CMAKE_CURRENT_SOURCE_DIR}/ral
	${CMAKE_CURRENT_SOURCE_DIR}/../lbm_port
	${SID_PAL_PUBLIC_INCLUDES}
)

target_compile_definitions(sid_pal_radio_lr20xx_impl PRIVATE
	RTOS_ZEPHYR=1
	LR20XX
	LR2021
	REGION_US_915
)

target_link_libraries(sid_pal_radio_lr20xx_impl PUBLIC
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
