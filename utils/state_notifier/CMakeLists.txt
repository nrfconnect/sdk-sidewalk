#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

zephyr_library()

zephyr_library_sources(
    state_notifier.c
    state_notifier_gpio_backend.c
)

zephyr_library_sources_ifdef(CONFIG_LOG
    state_notifier_log_backend.c
)
