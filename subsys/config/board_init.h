/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#include <sid_error.h>

/**
 * @brief Initial board configuration.
 *
 * @return SID_ERROR_NONE when success or error code otherwise.
 */
sid_error_t sidewalk_board_init(void);

#endif /* BOARD_CONFIG_H */
