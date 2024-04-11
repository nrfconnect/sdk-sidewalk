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
	X(error, not_connected, 0)                                                                          \
	X(working, not_connected, 0)                                                                        \
	X(registered, not_connected, 0)                                                                     \
	X(time_sync, not_connected, 0)                                                                      \
	X(connected, not_connected, 0)                                                                      \
	X(dfu, not_connected, 0)                                                                            \
	X(sending, not_connected, 0)                                                                        \
	X(receiving, not_connected, 0)
#else
#error "Unknow device application states."
#endif

#endif /* APPLICATION_STATES_H */
