/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef SID_PAL_APP_CALLBACKS_H
#define SID_PAL_APP_CALLBACKS_H

#include <stdbool.h>
#include <zephyr/bluetooth/bluetooth.h>

int app_bt_enable(bt_ready_cb_t cb);

int app_bt_disable();

enum BT_id_values{
        _BT_ID_DEFAULT = BT_ID_DEFAULT,
        #if defined (CONFIG_SIDEWALK)
        BT_ID_SIDEWALK,
        #endif
        #if defined (CONFIG_SIDEWALK_DFU)
        BT_ID_SMP_DFU,
        #endif
        _BT_ID_MAX =  CONFIG_BT_ID_MAX
};

BUILD_ASSERT(_BT_ID_MAX <= CONFIG_BT_ID_MAX, "Too many BT Ids! Configured limit is %d, but used %d", CONFIG_BT_ID_MAX, _BT_ID_MAX);

#endif
