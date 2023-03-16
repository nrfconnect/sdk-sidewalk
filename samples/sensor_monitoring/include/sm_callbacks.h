/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef SM_CALLBACKS_H
#define SM_CALLBACKS_H

#include <sid_api.h>

/**
 * @brief This function sets callbacks which will be invoked Sidewalk events occurs.
 *
 * @param ctx current user context.
 * @param cb pointer to object which stores callbacks.
 * @return SID_ERROR_NONE when success or error code otherwise.
 */
sid_error_t sm_callbacks_set(void *ctx, struct sid_event_callbacks *cb);

#endif /* SM_CALLBACKS_H */
