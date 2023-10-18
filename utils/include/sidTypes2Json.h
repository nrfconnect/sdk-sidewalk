/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <sidTypes2str.h>

#define JSON_VAL_sid_msg_desc(name, msg_desc, msg_desc_attr_rx, comma_or_empty)                    \
	JSON_VAL_DICT(name, comma_or_empty, {                                                      \
		JSON_VAL_INT("link_type", msg_desc->link_type, JSON_NEXT);                         \
		JSON_VAL_INT("type", msg_desc->type, JSON_NEXT);                                   \
		JSON_VAL_STR("type_str", SID_MSG_TYPE_STR(msg_desc->type), JSON_NEXT);             \
		JSON_VAL_INT("link_mode", msg_desc->link_mode, JSON_NEXT);                         \
		JSON_VAL_STR("link_mode_str", (SID_LINK_MODE_STR(msg_desc->link_mode)),            \
			     JSON_NEXT);                                                           \
		JSON_VAL_INT("id", msg_desc->id, JSON_NEXT);                                       \
		JSON_VAL_DICT("msg_desc_attr", JSON_LAST, {                                        \
			if (msg_desc_attr_rx) {                                                    \
				JSON_VAL_DICT("rx_attr", JSON_LAST, {                              \
					JSON_VAL_BOOL("is_msg_ack",                                \
						      msg_desc->msg_desc_attr.rx_attr.is_msg_ack,  \
						      JSON_NEXT);                                  \
					JSON_VAL_BOOL(                                             \
						"is_msg_duplicate",                                \
						msg_desc->msg_desc_attr.rx_attr.is_msg_duplicate,  \
						JSON_NEXT);                                        \
					JSON_VAL_BOOL(                                             \
						"ack_requested",                                   \
						msg_desc->msg_desc_attr.rx_attr.ack_requested,     \
						JSON_NEXT);                                        \
					JSON_VAL_INT("rssi", msg_desc->msg_desc_attr.rx_attr.rssi, \
						     JSON_NEXT);                                   \
					JSON_VAL_INT("snr", msg_desc->msg_desc_attr.rx_attr.snr,   \
						     JSON_LAST);                                   \
				});                                                                \
			} else {                                                                   \
				JSON_VAL_DICT("tx_attr", JSON_LAST, {                              \
					JSON_VAL_BOOL("request_ack",                               \
						      msg_desc->msg_desc_attr.tx_attr.request_ack, \
						      JSON_NEXT);                                  \
					JSON_VAL_INT("num_retries",                                \
						     msg_desc->msg_desc_attr.tx_attr.num_retries,  \
						     JSON_NEXT);                                   \
					JSON_VAL_INT(                                              \
						"ttl_in_seconds",                                  \
						msg_desc->msg_desc_attr.tx_attr.ttl_in_seconds,    \
						JSON_LAST);                                        \
				});                                                                \
			}                                                                          \
		});                                                                                \
	})

#define JSON_VAL_sid_error_t(name, error, comma_or_empty)                                          \
	JSON_VAL_DICT(name, comma_or_empty, {                                                      \
		JSON_VAL_INT("value", error, JSON_NEXT);                                           \
		JSON_VAL_STR("str", SID_ERROR_T_STR(error), JSON_LAST);                            \
	});
