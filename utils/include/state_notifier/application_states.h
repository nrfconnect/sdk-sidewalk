/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef APPLICATION_STATES_H
#define APPLICATION_STATES_H

/* X(state_name) */

#define X_APPLICAITON_STATES                                                                       \
	X(error)                                                                                   \
	X(working)                                                                                 \
	X(registered)                                                                              \
	X(time_sync)                                                                               \
	X(connected)                                                                               \
	X(dfu)                                                                                     \
	X(sending)                                                                                 \
	X(receiving)

#endif /* APPLICATION_STATES_H */
