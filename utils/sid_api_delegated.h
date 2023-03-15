/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#ifndef SID_API_DELEGATED_H
#define SID_API_DELEGATED_H
#include <sid_api.h>
#include <zephyr/kernel.h>
#include <window_iterator_macro.h>

/**
 * @brief X macro for api functions:
 * X(API_function_name, [argument_type, argument_name] * N)
 *
 */
#define API_FUNCTIONS									    \
	X(sid_init, const struct sid_config *, config, struct sid_handle **, handle)	    \
	X(sid_deinit, struct sid_handle *, handle)					    \
	X(sid_start, struct sid_handle *, handle, uint32_t, link_mask)			    \
	X(sid_stop, struct sid_handle *, handle, uint32_t, link_mask)			    \
	X(sid_put_msg,									    \
	  struct sid_handle *, handle, const struct sid_msg *, msg,			    \
	  struct sid_msg_desc *, msg_desc)						    \
	X(sid_set_factory_reset, struct sid_handle *, handle)				    \
	X(sid_get_mtu,									    \
	  struct sid_handle *, handle, enum sid_link_type, link_type, size_t *, mtu)	    \
	X(sid_get_status, struct sid_handle *, handle, struct sid_status *, current_status) \
	X(sid_ble_bcn_connection_request, struct sid_handle *, handle, bool, set)	    \
	X(sid_get_time,									    \
	  struct sid_handle *, handle, enum sid_time_format, format,			    \
	  struct sid_timespec *, curr_time)						    \
	X(sid_set_msg_dest_id, struct sid_handle *, handle, uint32_t, id)		    \
	X(sid_option,									    \
	  struct sid_handle *, handle, enum sid_option, option, void *, data, size_t, len)

/**
 * @brief Declare sid_api delegated functions
 *
 */
#define ARGUMENT_LIST(a, b) a b
#define X(name, ...) sid_error_t name ## _delegated(WINDOW_ITERATOR_SIZE_2(ARGUMENT_LIST, ( , ), __VA_ARGS__));
API_FUNCTIONS
#undef X
#undef ARGUMENT_LIST

void sid_api_delegated(struct k_work_q *workq);

#endif /* SID_API_DELEGATED_H */
