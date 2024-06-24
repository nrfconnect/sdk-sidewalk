/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <sid_hal_memory_ifc.h>
#include <string.h>
#include <zephyr/syscall.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(scratch_buffer);

#define SCRATCH_FILE_ID_UNUSED UINT32_MAX
#define SCRATCH_BUFFERS_MAX (3)

typedef struct {
	uint32_t id;
	void *buffer;
} file_ctx_t;

static file_ctx_t files[SCRATCH_BUFFERS_MAX];

void scratch_buffer_init(void)
{
	for (uint8_t i = 0; i < SCRATCH_BUFFERS_MAX; i++) {
		files[i].id = SCRATCH_FILE_ID_UNUSED;
	}
}

void *scratch_buffer_create(uint32_t file_id, size_t size)
{
	file_ctx_t *p_file = NULL;
	for (uint8_t i = 0; i < SCRATCH_BUFFERS_MAX; i++) {
		if (!p_file && files[i].id == SCRATCH_FILE_ID_UNUSED) {
			p_file = &files[i];
		}
		if(files[i].id == file_id){
			LOG_ERR("buffer already assigned to file (id %d)", file_id);
			return NULL;
		}
	}

	if (!p_file) {
		LOG_ERR("too many buffers (max %d)", SCRATCH_BUFFERS_MAX);
		return NULL;
	}

	p_file->id = file_id;

	p_file->buffer = sid_hal_malloc(size);
	if (!p_file->buffer) {
		LOG_ERR("buffor alloc fail (size %d)", size);
		return NULL;
	}

	memset(p_file->buffer, 0x0, size);

	return p_file->buffer;
}

void scratch_buffer_remove(uint32_t file_id)
{
	file_ctx_t *p_file = NULL;
	for (uint8_t i = 0; i < SCRATCH_BUFFERS_MAX; i++) {
		if (files[i].id == file_id) {
			p_file = &files[i];
			break;
		}
	}

	if (!p_file) {
		LOG_ERR("buffer not found for file (id %d)", file_id);
		return;
	}

	p_file->id = SCRATCH_FILE_ID_UNUSED;
	if (p_file->buffer) {
		sid_hal_free(p_file->buffer);
		p_file->buffer = NULL;
	}
}
