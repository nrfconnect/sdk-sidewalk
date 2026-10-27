/*
 * Copyright 2026 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.TXT file.  This file is a
 * Modifiable File, as defined in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef SID_MFG_CREDENTIALS_H
#define SID_MFG_CREDENTIALS_H

/**
 * @file sid_mfg_credentials.h
 *
 * Persistence of registration credentials in the manufacturing (MFG) page.
 *
 * A device registered at the factory holds its registration credentials only in
 * the rewritable KV store (NVM3 on SiLabs, FDS/TLV on Nordic). When the device is
 * later flashed with production firmware the KV store can be wiped, losing the
 * registration even though the MFG page survives the reflash.
 *
 * This module mirrors the credentials between the two stores:
 *   - commit()  : KV  -> MFG   (factory, write-once)
 *   - recover() : MFG -> KV    (production, at boot, only if KV is empty)
 *
 * Because the MFG page survives a reflash, a factory-registered device stays
 * operational across a KV-wiping reflash.
 */

#include <sid_error.h>

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Fixed size in bytes of the WAN/APP master keys handled by this module. */
#define SID_MFG_CREDENTIALS_MASTER_KEY_SZ 16

/**
 * Snapshot of the live session credentials that must survive a factory reset.
 *
 * The MFG page holds the factory-provisioned values, but the WAN master key may
 * have been rotated (LW key refresh) and the time-sync counter advanced since
 * factory. Restoring only the factory values would desynchronize the device from
 * the cloud (stale key) and risk time-sync counter reuse. Take a snapshot with
 * sid_mfg_credentials_backup() before wiping the KV store and re-apply it with
 * sid_mfg_credentials_apply_backup() after sid_mfg_credentials_recover().
 */
struct sid_mfg_credentials_backup {
    uint8_t wan_master_key[SID_MFG_CREDENTIALS_MASTER_KEY_SZ];
    uint32_t tsync_counter_seed; /**< STORAGE_KV_TSYNC_COUNTER_SEED, raw (stored byte order); 0 when absent. */
    uint32_t tsync_counter;      /**< STORAGE_KV_TSYNC_COUNTER, raw (stored byte order); 0 when absent. */
};

/**
 * Copy the registration credentials from the KV store into the MFG page.
 *
 * Intended for factory firmware only, run once after registration completes and
 * all credentials are present in the KV store. Writes are gated so that:
 *   - it is a no-op (returns SID_ERROR_NONE) if a valid credential set already
 *     exists in the MFG page (write-once), and
 *   - a completion marker (magic + CRC32 over the mirrored fields) is written
 *     last, so a partial/interrupted commit never looks valid to recover().
 *
 * @retval SID_ERROR_NONE             on success or when already committed.
 * @retval SID_ERROR_NOT_FOUND        a required credential is missing from KV.
 * @retval SID_ERROR_STORAGE_FULL /
 *         SID_ERROR_GENERIC          on MFG write failure.
 */
sid_error_t sid_mfg_credentials_commit(void);

/**
 * Restore the registration credentials from the MFG page into the KV store.
 *
 * Intended for production firmware, run at boot before Sidewalk starts. It acts
 * only when a valid credential set is present in the MFG page AND the KV store
 * does not already hold them; otherwise it is a no-op. Because it checks before
 * acting, it is safe to call on every boot (and after a factory reset).
 *
 * On success the device's registration state is normalized so the data path
 * treats it as registered.
 *
 * @retval SID_ERROR_NONE             restored, or nothing to do (no-op).
 * @retval SID_ERROR_GENERIC          on KV write failure.
 */
sid_error_t sid_mfg_credentials_recover(void);

/**
 * Snapshot the live WAN master key, time-sync counter seed and time-sync counter
 * from the KV store.
 *
 * Call BEFORE committing to a factory reset (i.e. before wiping the KV store):
 * a failed snapshot means the wipe would restore a stale factory key, so the
 * caller must abort the reset on error. An absent WAN master key means a fresh /
 * unregistered device with nothing to preserve (SID_ERROR_NOT_FOUND, skip the
 * apply). An absent seed or counter is captured as 0, matching the security
 * layer's absent-record semantics (init_tsync_counter() defaults both to 0).
 *
 * @param[out] backup  receives the snapshot.
 *
 * @retval SID_ERROR_NONE               snapshot taken.
 * @retval SID_ERROR_NOT_FOUND          no WAN master key present (fresh device);
 *                                      apply may safely be skipped.
 * @retval SID_ERROR_STORAGE_READ_FAIL  a read failed with a genuine storage
 *                                      error; do NOT proceed with the wipe.
 * @retval SID_ERROR_NULL_POINTER       @p backup is NULL.
 */
sid_error_t sid_mfg_credentials_backup(struct sid_mfg_credentials_backup *backup);

/**
 * Re-apply a snapshot taken by sid_mfg_credentials_backup() to the KV store.
 *
 * Call after sid_mfg_credentials_recover() so the live (possibly rotated) WAN
 * master key, the live time-sync counter seed and the advanced time-sync counter
 * overwrite the factory values restored from the MFG page.
 *
 * @param[in] backup  snapshot to apply.
 *
 * @retval SID_ERROR_NONE          snapshot written.
 * @retval SID_ERROR_GENERIC       on KV write failure.
 * @retval SID_ERROR_NULL_POINTER  @p backup is NULL.
 */
sid_error_t sid_mfg_credentials_apply_backup(const struct sid_mfg_credentials_backup *backup);

#ifdef __cplusplus
}
#endif

#endif /* SID_MFG_CREDENTIALS_H */
