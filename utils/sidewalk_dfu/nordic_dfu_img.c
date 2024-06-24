/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <sidewalk_dfu/nordic_dfu_img.h>
#include <dfu/dfu_multi_image.h>
#include <dfu/dfu_target.h>
#include <dfu/dfu_target_mcuboot.h>

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(nordic_dfu_img, CONFIG_SIDEWALK_LOG_LEVEL);

#define IMAGE_MCUBOOT_UPDATE_ALL (-1)

#ifdef CONFIG_SIDEWALK_DFU_IMG_BUFFER_SIZE
#define IMAGE_BUFFER_SIZE CONFIG_SIDEWALK_DFU_IMG_BUFFER_SIZE
#else
#define IMAGE_BUFFER_SIZE 64
#warning "image buffer size not configuread"
#endif /* CONFIG_SIDEWALK_DFU_IMG_BUFFER_SIZE */

#ifdef CONFIG_UPDATEABLE_IMAGE_NUMBER
#define IMAGE_NUMBER CONFIG_UPDATEABLE_IMAGE_NUMBER
#else
#define IMAGE_NUMBER 1
#warning "updateable image number not configuread"
#endif /* CONFIG_UPDATEABLE_IMAGE_NUMBER */

static uint8_t image_buf[IMAGE_BUFFER_SIZE] __aligned(4);

static struct dfu_image_writer writers[IMAGE_NUMBER];

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

int nordic_dfu_img_init(void)
{
	int err = 0;
	err = dfu_target_mcuboot_set_buf(image_buf, sizeof(image_buf));
	if (err) {
		LOG_ERR("mcuboot set buffor fail %d", err);
		return -EINVAL;
	}

	err = dfu_multi_image_init(image_buf, sizeof(image_buf));
	if (err) {
		LOG_ERR("multi imge init fail %d", err);
		return -EINVAL;
	}

	for (uint8_t img_id = 0; img_id < IMAGE_NUMBER; img_id++) {
		writers[img_id].image_id = img_id;
		writers[img_id].open = open;
		writers[img_id].write = write;
		writers[img_id].close = close;
		err = dfu_multi_image_register_writer(&writers[img_id]);
		if (err) {
			LOG_ERR("img id %d register fail %d", img_id, err);
			return -ENOMEM;
		}
	}

	return 0;
}

int nordic_dfu_img_write(size_t offset, void *data, size_t data_size)
{
	return dfu_multi_image_write(offset, data, data_size);
}

int nordic_dfu_img_cancel(void)
{
	return dfu_multi_image_done(false);
}

int nordic_dfu_img_finalize(void)
{
	int err = 0;
	err = dfu_multi_image_done(true);
	if (err) {
		LOG_ERR("coplete dfu fail %d", err);
		return -ESPIPE;
	}

	err = dfu_target_schedule_update(IMAGE_MCUBOOT_UPDATE_ALL);
	if (err) {
		LOG_ERR("schedule update fail %d", err);
		return -EACCES;
	}

	return 0;
}

void ordic_dfu_img_deinit(void)
{
	// Nothging to do
}
