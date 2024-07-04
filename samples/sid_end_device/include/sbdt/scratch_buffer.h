/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef SCRATCH_BUFFER_H
#define SCRATCH_BUFFER_H

#include <stdint.h>
#include <stddef.h>

/**
 * @brief Init scratch buffer
 * 
 * Inilizing module is neccessary to find files by id.
 * 
 */
void scratch_buffer_init(void);

/**
 * @brief Init scratch buffer
 * 
 * Free all allocated data and set all file id as UNUSED.
 * After calling deinit there is no need to call init again.
 * 
 */
void scratch_bufer_deinit(void);

/**
 * @brief Create scratch buffor assigned for file id.
 * 
 * @param id file id
 * @param size size of file buffer
 * @return void* pointer to allocated memory. NULL on error.
 */
void *scratch_buffer_create(uint32_t id, size_t size);

/**
 * @brief Remove scratch buffer for file id.
 * 
 * @param id file id
 */
void scratch_buffer_remove(uint32_t id);

#endif /* SCRATCH_BUFFER_H */
