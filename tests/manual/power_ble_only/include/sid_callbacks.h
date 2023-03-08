/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */
#ifndef SID_CALLBACKS_H
#define SID_CALLBACKS_H
 #include <sid_api.h>
 
/**
 * @brief This function sets callbacks which will be invoked Sidewalk events occurs.
 *
 * @param ctx current user context.
 * @param cb pointer to object which stores callbacks.
 * @return 0 when success or negative value otherwise.
 */
int sid_callbacks_set(void *ctx, struct sid_event_callbacks *cb);

#endif /* SID_CALLBACKS_H */
