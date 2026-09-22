/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef SID_MFG_STORAGE_H
#define SID_MFG_STORAGE_H

#include <stdint.h>

/**
 * @brief Flush the staged manufacturing data.
 *
 * Writes from sid_pal_mfg_store_write() are staged in RAM and become durable
 * only when this function is called. Staged data is discarded by
 * sid_pal_mfg_store_erase() and sid_pal_mfg_store_deinit(), and is kept when
 * the flush fails, so that it can be retried.
 *
 * @return 0 on success, -ERRNO on error, or SID_ERROR_NOSUPPORT when built
 *         without CONFIG_SIDEWALK_MFG_ALLOW_WRITE.
 */
int32_t sid_mfg_storage_flush(void);

#endif /* SID_MFG_STORAGE_H */
