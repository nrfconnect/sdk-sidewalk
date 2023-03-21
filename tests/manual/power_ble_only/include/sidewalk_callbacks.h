/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef SIDEWALK_CALLBACKS_H
#define SIDEWALK_CALLBACKS_H

#include <sid_api.h>

/**
 * @brief Initialize sidewalk callbacks
 *
 * @param context Application context passed to callbacks
 * @param callbacks [out] callbacks to set
 * @returns #SID_ERROR_NONE in case of success negative error code otherwise.
 */
sid_error_t sidewalk_callbacks_set(void *context, struct sid_event_callbacks *callbacks);

#endif /* SIDEWALK_CALLBACKS_H */
