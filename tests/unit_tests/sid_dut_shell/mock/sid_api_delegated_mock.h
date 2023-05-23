/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <sid_api.h>
#include <zephyr/fff.h>
#include <stdint.h>
#include <sid_api_delegated.h>
DEFINE_FFF_GLOBALS;

FAKE_VALUE_FUNC(sid_error_t, sid_init_delegated, const struct sid_config *, struct sid_handle **);
FAKE_VALUE_FUNC(sid_error_t, sid_deinit_delegated, struct sid_handle *);
FAKE_VALUE_FUNC(sid_error_t, sid_start_delegated, struct sid_handle *, uint32_t);
FAKE_VALUE_FUNC(sid_error_t, sid_stop_delegated, struct sid_handle *, uint32_t);
FAKE_VALUE_FUNC(sid_error_t, sid_put_msg_delegated, struct sid_handle *, const struct sid_msg *,
		struct sid_msg_desc *);
FAKE_VALUE_FUNC(sid_error_t, sid_set_factory_reset_delegated, struct sid_handle *);
FAKE_VALUE_FUNC(sid_error_t, sid_get_mtu_delegated, struct sid_handle *, enum sid_link_type,
		size_t *);
FAKE_VALUE_FUNC(sid_error_t, sid_get_status_delegated, struct sid_handle *, struct sid_status *);
FAKE_VALUE_FUNC(sid_error_t, sid_ble_bcn_connection_request_delegated, struct sid_handle *, bool);
FAKE_VALUE_FUNC(sid_error_t, sid_get_time_delegated, struct sid_handle *, enum sid_time_format,
		struct sid_timespec *);
FAKE_VALUE_FUNC(sid_error_t, sid_set_msg_dest_id_delegated, struct sid_handle *, uint32_t);
FAKE_VALUE_FUNC(sid_error_t, sid_option_delegated, struct sid_handle *, enum sid_option, void *,
		size_t);
