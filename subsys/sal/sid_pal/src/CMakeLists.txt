#
# Copyright (c) 2023 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

if(NOT(CONFIG_LOG) OR CONFIG_SIDEWALK_LOG_LEVEL_OFF)
	zephyr_compile_definitions(SID_PAL_LOG_ENABLED=0)
else()
	zephyr_compile_definitions(SID_PAL_LOG_ENABLED=1)
	# Sidewalk log levels are defined in sid_pal_log_ifc.h
	# Zephyr log levels are defined in ${ZEPHYR_BASE}/subsys/logging/Kconfig.template.log_config
	# To covert Zephyr log level to Sidewalk log level it hest to be decreased by one.
	zephyr_compile_definitions(SID_PAL_LOG_LEVEL=${CONFIG_SIDEWALK_LOG_LEVEL}-1)
endif() # CONFIG_SIDEWALK_LOG_LEVEL_OFF
zephyr_library_sources_ifdef(CONFIG_SIDEWALK_LOG sid_log.c)

zephyr_compile_definitions_ifndef(CONFIG_SIDEWALK_ASSERT SID_PAL_ASSERT_DISABLED)
zephyr_library_sources_ifdef(CONFIG_SIDEWALK_ASSERT sid_assert.c)

zephyr_library_sources_ifdef(CONFIG_SIDEWALK_BLE
	sid_ble_adapter.c
	sid_ble_service.c
	sid_ble_ama_service.c
	sid_ble_adapter_callbacks.c
	sid_ble_advert.c
	sid_ble_connection.c
	hci_utils.c
)

zephyr_library_sources_ifdef(CONFIG_SIDEWALK_VENDOR_SERVICE sid_ble_vnd_service.c)

zephyr_library_sources_ifdef(CONFIG_SIDEWALK_LOGGING_SERVICE sid_ble_log_service.c)

zephyr_library_sources_ifdef(CONFIG_SIDEWALK_CRYPTO sid_crypto.c)
zephyr_library_sources_ifdef(CONFIG_SIDEWALK_CRYPTO_PSA_KEY_STORAGE sid_crypto_keys.c)

zephyr_library_sources_ifdef(CONFIG_SIDEWALK_MFG_STORAGE sid_mfg_storage.c sid_mfg_hex_v8.c)
zephyr_library_sources_ifdef(CONFIG_SIDEWALK_MFG_STORAGE_SUPPORT_HEX_v7 sid_mfg_hex_v7.c)
zephyr_library_sources_ifdef(CONFIG_DEPRECATED_SIDEWALK_MFG_STORAGE sid_mfg_storage_deprecated.c)

zephyr_library_sources_ifdef(CONFIG_SIDEWALK_STORAGE sid_storage.c)

zephyr_library_sources_ifdef(CONFIG_SIDEWALK_TIMER sid_timer.c)

zephyr_library_sources_ifdef(CONFIG_SIDEWALK_UPTIME
	sid_uptime.c
	zephyr_time.c
)

zephyr_library_sources_ifdef(CONFIG_SIDEWALK_CRITICAL_REGION sid_critical_region.c)

zephyr_library_sources_ifdef(CONFIG_SIDEWALK_GPIO sid_gpio.c sid_gpio_utils.c)

if(CONFIG_SOC_SERIES_NRF53X)
zephyr_library_sources_ifdef(CONFIG_SIDEWALK_TEMPERATURE sid_temperature_mock.c)
message("Note: Sidewalk uses MOCK temperature implementation")
else()
zephyr_library_sources_ifdef(CONFIG_SIDEWALK_TEMPERATURE sid_temperature.c)
endif() # CONFIG_SOC_SERIES_NRF53X

zephyr_library_sources_ifdef(CONFIG_SIDEWALK_SW_INTERRUPTS sid_sw_interrupts.c)

zephyr_library_sources_ifdef(CONFIG_SIDEWALK_DELAY sid_delay.c)

if(CONFIG_SOC_NRF52840)
zephyr_library_sources_ifdef(CONFIG_SIDEWALK_SPI_BUS sid_pal_serial_bus_nrfx_spi.c)
else()
zephyr_library_sources_ifdef(CONFIG_SIDEWALK_SPI_BUS sid_pal_serial_bus_spi.c)
endif() # CONFIG_SOC_NRF52840

zephyr_library_sources_ifdef(CONFIG_SIDEWALK sid_common.c)

zephyr_library_sources_ifdef(DEPRECATED_SIDEWALK_PAL_INIT pal_init.c)

zephyr_library_sources(bt_app_callbacks.c)
