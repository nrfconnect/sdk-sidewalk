/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef APPLICATION_STATES_H
#define APPLICATION_STATES_H
#include <zephyr/kernel.h>

/* X(state_name, gpio_port, gpio_pin) */
#if defined(NRF52840_XXAA)
#define X_APPLICAITON_STATES                                                                       \
	X(error, gpio1, 7)                                                                         \
	X(working, gpio0, 16)                                                                      \
	X(registered, gpio0, 15)                                                                   \
	X(time_sync, gpio0, 14)                                                                    \
	X(connected, gpio0, 13)                                                                    \
	X(dfu, gpio1, 1)                                                                           \
	X(sending, gpio1, 3)                                                                       \
	X(receiving, gpio1, 2)
#elif defined(NRF5340_XXAA)
#define X_APPLICAITON_STATES                                                                       \
	X(error, gpio1, 8)                                                                         \
	X(working, gpio0, 31)                                                                      \
	X(registered, gpio0, 30)                                                                   \
	X(time_sync, gpio0, 29)                                                                    \
	X(connected, gpio0, 28)                                                                    \
	X(dfu, gpio1, 2)                                                                           \
	X(sending, gpio1, 4)                                                                       \
	X(receiving, gpio1, 3)
#elif defined(NRF54L15_ENGA_XXAA)
#define X_APPLICAITON_STATES                                                                       \
	X(error, gpio0, 4)                                                                         \
	X(working, gpio1, 14)                                                                      \
	X(registered, gpio1, 13)                                                                   \
	X(time_sync, gpio1, 8)                                                                     \
	X(connected, gpio0, 4)                                                                     \
	X(dfu, gpio0, 4)                                                                           \
	X(sending, gpio0, 4)                                                                       \
	X(receiving, gpio0, 4)
#else
#error "Unknow device application states."
#endif

#endif /* APPLICATION_STATES_H */
