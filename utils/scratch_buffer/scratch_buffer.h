/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef SCRATCH_BUFFER_H
#define SCRATCH_BUFFER_H

#include <stdint.h>
#include <stddef.h>

void scratch_buffer_init(void);

void *scratch_buffer_create(uint32_t id, size_t size);

void scratch_buffer_remove(uint32_t id);

#endif /* SCRATCH_BUFFER_H */
