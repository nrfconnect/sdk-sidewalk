/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <dfu/dfu_multi_image.h>
#include <dfu/dfu_target.h>
#include <dfu/dfu_target_mcuboot.h>

#define IMGAGE_BUFFER_SIZE 1024

#ifdef CONFIG_UPDATEABLE_IMAGE_NUMBER
#define IMAGE_NUMBER CONFIG_UPDATEABLE_IMAGE_NUMBER
#else 
#define IMAGE_NUMBER 1
#warning "updateable image number not configuread"
#endif /* CONFIG_UPDATEABLE_IMAGE_NUMBER */

static uint8_t image_buf[IMGAGE_BUFFER_SIZE];

static struct dfu_image_writer writers[CONFIG_UPDATEABLE_IMAGE_NUMBER];

static int open(int image_id, size_t image_size)
{
	return dfu_target_init(DFU_TARGET_IMAGE_TYPE_MCUBOOT, image_id, image_size, NULL);
}

static int write(const uint8_t *chunk, size_t chunk_size)
{
	return dfu_target_write(chunk, chunk_size);
}

static int close(bool success)
{
	return success ? dfu_target_done(success) : dfu_target_reset();
}

int dfu_multi_image_init_target_mcuboot(void)
{
	int err = dfu_target_mcuboot_set_buf(image_buf, sizeof(image_buf));
	if (err) {
		return err;
	}

	err = dfu_multi_image_init(image_buf, sizeof(image_buf));
	if (err) {
		return err;
	}

	for (uint8_t img_id = 0; img_id < CONFIG_UPDATEABLE_IMAGE_NUMBER; img_id++) {
		writers[img_id].image_id = img_id;
		writers[img_id].open = open;
		writers[img_id].write = write;
		writers[img_id].close = close;
		err = dfu_multi_image_register_writer(&writers[img_id]);
		if (err) {
			return err;
		}
	}

    return 0;
}
