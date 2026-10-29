/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef ROW_SHELL_H
#define ROW_SHELL_H

#include <stdbool.h>

/**
 * @brief Check if the Registration over Wire stack is initialized.
 *
 * The Sidewalk stack must not be initialized while it is.
 *
 * @return true if the serial registration stack is initialized, false otherwise.
 */
bool row_shell_is_active(void);

#endif /* ROW_SHELL_H */
