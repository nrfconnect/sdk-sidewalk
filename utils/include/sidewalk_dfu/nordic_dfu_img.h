/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef NORDIC_DFU_IMG_H
#define NORDIC_DFU_IMG_H

#include <stddef.h>

/**
 * @brief Init dfu image management based on dfu mutli image and mcuboot
 * 
 * @return 0 on success, negative error code othervise
 */
int nordic_dfu_img_init(void);

/**
 * @brief Write a chunk of data to the new image parition
 * 
 * @param offset chunk data offset
 * @param data pointer to data buffer
 * @param data_size size of the buffer data
 * @return 0 on success, negative error code othervise 
 */
int nordic_dfu_img_write(size_t offset, void *data, size_t data_size);

/**
 * @brief Cancel dfu image processing.
 * 
 * @return 0 on success, negative error code othervise
 */
int nordic_dfu_img_cancel(void);

/**
 * @brief Finalize dfu image writing process. Mark image as ready.
 * 
 * @return 0 on success, negative error code othervise
 */
int nordic_dfu_img_finalize(void);

/**
 * @brief Denit dfu image management
 */
void nordic_dfu_img_deinit(void);

#endif /* NORDIC_DFU_IMG_H */
