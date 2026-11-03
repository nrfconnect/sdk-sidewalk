/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef SID_STATE_LITE_H
#define SID_STATE_LITE_H

/*
 * Subset of the Sidewalk SDK internal sid_state_ifc.h.
 *
 * The full header depends on SDK internals that are not part of this add-on.
 * The prototypes below must stay identical to the ones in sid_state_ifc.h,
 * because they are implemented by the prebuilt Sidewalk library.
 */

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Value of the CONFIGURATION_COMPLETE record: device is not registered. */
#define SID_STATE_LITE_DEVICE_NOT_REGISTERED 0
/** Value of the CONFIGURATION_COMPLETE record: device is registered. */
#define SID_STATE_LITE_DEVICE_REGISTERED 1
/** Value of the CONFIGURATION_COMPLETE record: device is factory-registered. */
#define SID_STATE_LITE_DEVICE_FACTORY_REGISTERED 2

/**
 * @brief Check if the device is factory-registered.
 *
 * @return true if the device is factory-registered, false otherwise.
 */
bool sid_state_is_device_factory_registered(void);

/**
 * @brief Get the device registration state.
 *
 * @return One of the SID_STATE_LITE_DEVICE_* values.
 */
uint8_t sid_state_get_device_state(void);

#ifdef __cplusplus
}
#endif

#endif /* SID_STATE_LITE_H */
