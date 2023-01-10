/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#ifndef APPLICATION_STATES_H
#define APPLICATION_STATES_H

/* X(state_name, gpio_port, gpio_pin) */
#define X_APPLICAITON_STATES	 \
	X(error, gpio1, 2)	 \
	X(registered, gpio0, 14) \
	X(time_sync, gpio0, 15)	 \
	X(link, gpio0, 16)	 \
	X(connected, gpio0, 13)	 \
	X(dfu, gpio1, 1)

#endif /* APPLICATION_STATES_H */
