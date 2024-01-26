#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#
if(CONFIG_STATE_NOTIFIER)
zephyr_library_sources(
    state_notifier.c
    state_notifier_gpio_backend.c
)
zephyr_library_sources_ifdef(CONFIG_LOG
    state_notifier_log_backend.c
)
endif() # CONFIG_STATE_NOTIFIER

if(CONFIG_SIDEWALK_BUTTONS)
	zephyr_library_sources(buttons.c)
	zephyr_compile_definitions(HAL_BUTTONS_LOG_LEVEL=${CONFIG_SIDEWALK_LOG_LEVEL})
endif() # CONFIG_SIDEWALK_BUTTONS

if (CONFIG_SIDEWALK_CLI)
    zephyr_library_sources(shell.c)
endif() # CONFIG_SIDEWALK_CLI

zephyr_library_sources_ifdef(CONFIG_SIDEWALK_DFU_SERVICE_BLE nordic_dfu.c)
zephyr_library_sources_ifdef(CONFIG_SIDEWALK_SETTINGS_UTILS settings_utils.c)

zephyr_library_sources_ifdef(CONFIG_SIDEWALK_ON_DEV_CERT sid_on_dev_cert_cli.c sid_on_dev_cert.c)
