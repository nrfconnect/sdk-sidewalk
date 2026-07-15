#
# Copyright (c) 2024 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

if(NOT CONFIG_SIDEWALK_PAL_ZEPHYR_LIBS_DISABLED AND CONFIG_SIDEWALK_ON_DEV_CERT)
	zephyr_library_named(sid_on_dev_cert_impl)
	target_sources(sid_on_dev_cert_impl PRIVATE sid_on_dev_cert.c)

	target_include_directories(sid_on_dev_cert_impl PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/../include
	)

	target_link_libraries(sid_on_dev_cert_impl PRIVATE
		sid_on_dev_cert_ifc
		sid_endian
		sid_pal_log_ifc
		sid_pal_crypto_ifc
		sid_pal_mfg_store_ifc
	)
endif()
