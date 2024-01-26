#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

set(SID_LIB_DIR ${ZEPHYR_BASE}/../sidewalk/lib/cortex-m4/lora_fsk)

set(GCC_LIB_DIR ${ZEPHYR_SDK_INSTALL_DIR}/arm-zephyr-eabi/lib/gcc/arm-zephyr-eabi)
find_library(LIB_GCC NAMES gcc libgcc.a HINTS ${GCC_LIB_DIR}/* REQUIRED)

zephyr_library_link_libraries(${SID_LIB_DIR}/libsid_protocol_all.a
	${LIB_GCC}
)

zephyr_link_libraries_ifndef(CONFIG_SIDEWALK_PAL_RADIO_SOURCE ${SID_LIB_DIR}/libsid_pal_radio_sx126x_impl.a)
