/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#ifndef SID_API_DELEGATED_H
#define SID_API_DELEGATED_H
#include <sid_api.h>
#include <zephyr/kernel.h>

sid_error_t sid_init_delegated(const struct sid_config *config, struct sid_handle **handle);
sid_error_t sid_deinit_delegated(struct sid_handle *handle);
sid_error_t sid_start_delegated(struct sid_handle *handle, uint32_t link_mask);
sid_error_t sid_stop_delegated(struct sid_handle *handle, uint32_t link_mask);
sid_error_t sid_put_msg_delegated(struct sid_handle *handle, const struct sid_msg *msg,
				  struct sid_msg_desc *msg_desc);
sid_error_t sid_set_factory_reset_delegated(struct sid_handle *handle);
sid_error_t sid_get_mtu_delegated(struct sid_handle *handle, enum sid_link_type link_type,
				  size_t *mtu);
sid_error_t sid_get_status_delegated(struct sid_handle *handle, struct sid_status *current_status);
sid_error_t sid_ble_bcn_connection_request_delegated(struct sid_handle *handle, bool set);
sid_error_t sid_get_time_delegated(struct sid_handle *handle, enum sid_time_format format,
				   struct sid_timespec *curr_time);
sid_error_t sid_set_msg_dest_id_delegated(struct sid_handle *handle, uint32_t id);
sid_error_t sid_option_delegated(struct sid_handle *handle, enum sid_option option, void *data,
				 size_t len);

void sid_api_delegated(struct k_work_q *workq);

#endif /* SID_API_DELEGATED_H */
