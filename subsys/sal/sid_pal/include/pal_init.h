/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef PAL_INIT_H
#define PAL_INIT_H

#include <sid_error.h>

/**
 * @brief Initial board configuration.
 *
 * @return SID_ERROR_NONE when success or error code otherwise.
 */
sid_error_t application_pal_init(void);

#endif /* PAL_INIT_H */
