#
# Copyright (c) 2026 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

if(CONFIG_SIDEWALK_USE_PREBUILT_LIBRARIES)
	add_library(sidewalk INTERFACE)

	target_link_options(sidewalk INTERFACE "LINKER:--start-group")

	target_link_libraries(sidewalk INTERFACE
		app_utils
		sidewalk_sdk
		sidewalk_pal
		sid_base64
	)

	target_link_options(sidewalk INTERFACE "LINKER:--end-group")

	if(CONFIG_SIDEWALK_ON_DEV_CERT)
		target_link_libraries(sidewalk INTERFACE
			sid_on_dev_cert_ifc
			sid_on_dev_cert_impl
		)
	endif()
endif()
