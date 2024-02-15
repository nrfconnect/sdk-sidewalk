/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#define eval(x) x
#define concat(a, b) a##b
#define eval_bool(x) x ? 1 : 0
#define COND_TRUE(cond, code...) concat(COND_true_, cond)(code)
#define COND_FALSE(cond, code...) concat(COND_false_, cond)(code)

#define COND_true_0(code...)
#define COND_true_1(code...) code

#define COND_false_0(code...) code
#define COND_false_1(code...)

#include <sidTypes2str.h>
#include <json_printer.h>

#if defined(CONFIG_SIDEWALK_SUBGHZ_SUPPORT)
#include <sid_900_cfg.h>
#endif

/**
 * @brief prepare printf formater and arguments to print sid_msg_desc object
 *
 * @arg name: name of the JOSN object to print
 * @arg msg_desc pointer to sid_msg_desc object to print
 * @arg msg_desc_attr_rx flag to determine variant of union to print
 *	valid values are `1` or `0`, and those need to be literals
 *
 * @result result of this function can be passed to printf like function, or other JSON macro
 */
#define JSON_VAL_sid_msg_desc(name, msg_desc, msg_desc_attr_rx)                                                                      \
	JSON_NAME(                                                                                                                   \
		name,                                                                                                                \
		JSON_OBJ(JSON_LIST_7(                                                                                                \
			JSON_NAME("link_type", JSON_INT(msg_desc->link_type)),                                                       \
			JSON_NAME("type", JSON_INT(msg_desc->type)),                                                                 \
			JSON_NAME("type_str", JSON_STR(SID_MSG_TYPE_STR(msg_desc->type))),                                           \
			JSON_NAME("link_mode", JSON_INT(msg_desc->link_mode)),                                                       \
			JSON_NAME("link_mode_str",                                                                                   \
				  JSON_STR(SID_LINK_MODE_STR(msg_desc->link_mode))),                                                 \
			JSON_NAME("id", JSON_INT(msg_desc->id)),                                                                     \
			JSON_NAME(                                                                                                   \
				"msg_desc_attr",                                                                                     \
				JSON_OBJ(COND_TRUE(                                                                                  \
					msg_desc_attr_rx,                                                                            \
					JSON_NAME(                                                                                   \
						"rx_attr",                                                                           \
						JSON_OBJ(JSON_LIST_5(                                                                \
							JSON_NAME("is_msg_ack",                                                      \
								  JSON_BOOL(msg_desc->msg_desc_attr                                  \
										    .rx_attr                                         \
										    .is_msg_ack)),                                   \
							JSON_NAME(                                                                   \
								"is_msg_duplicate",                                                  \
								JSON_BOOL(                                                           \
									msg_desc->msg_desc_attr                                      \
										.rx_attr                                             \
										.is_msg_duplicate)),                                 \
							JSON_NAME(                                                                   \
								"ack_requested",                                                     \
								JSON_BOOL(msg_desc->msg_desc_attr                                    \
										  .rx_attr                                           \
										  .ack_requested)),                                  \
							JSON_NAME("rssi",                                                            \
								  JSON_INT(msg_desc->msg_desc_attr                                   \
										   .rx_attr.rssi)),                                  \
							JSON_NAME(                                                                   \
								"snr",                                                               \
								JSON_INT(msg_desc->msg_desc_attr                                     \
										 .rx_attr.snr))))))                                  \
						 COND_FALSE(                                                                         \
							 msg_desc_attr_rx,                                                           \
							 JSON_NAME(                                                                  \
								 "tx_attr",                                                          \
								 JSON_OBJ(JSON_LIST_4(                                               \
									 JSON_NAME(                                                  \
										 "request_ack",                                      \
										 JSON_BOOL(                                          \
											 msg_desc->msg_desc_attr                     \
												 .tx_attr                            \
												 .request_ack)),                     \
									 JSON_NAME(                                                  \
										 "num_retries",                                      \
										 JSON_INT(                                           \
											 msg_desc->msg_desc_attr                     \
												 .tx_attr                            \
												 .num_retries)),                     \
									 JSON_NAME(                                                  \
										 "additional_attr",                                  \
										 JSON_STR(SID_MSG_DESC_TX_ADDITIONAL_ATTRIBUTES_STR( \
											 msg_desc->msg_desc_attr                     \
												 .tx_attr                            \
												 .additional_attr))),                \
									 JSON_NAME(                                                  \
										 "ttl_in_seconds",                                   \
										 JSON_INT(                                           \
											 msg_desc->msg_desc_attr                     \
												 .tx_attr                            \
												 .ttl_in_seconds)))))))))))

/**
 * @brief prepare printf formater and arguments to print sid_error_t object
 *
 * @arg name: name of the JSON object to print
 * @arg error: object of sid_error_t to print
 *
 * @result result of this function can be passed to printf like function, or other JSON macro
 */
#define JSON_VAL_sid_error_t(name, error)                                                          \
	JSON_NAME(name, JSON_OBJ(JSON_LIST_2(JSON_NAME("value", JSON_INT(error)),                  \
					     JSON_NAME("str", JSON_STR(SID_ERROR_T_STR(error))))))

#define JSON_VAL_sid_link3_low_latency_config_t(name, val)                                         \
	JSON_NAME(name, JSON_OBJ(JSON_LIST_4(JSON_NAME("enabled", JSON_BOOL(val->enabled)),        \
					     JSON_NAME("latency", JSON_INT(val->latency)),         \
					     JSON_NAME("repetitions", JSON_INT(val->repetitions)), \
					     JSON_NAME("rate_limit", JSON_INT(val->rate_limit)))))

#define JSON_VAL_sid_control_event_data_t(name, val)                                               \
	JSON_NAME(name,                                                                            \
		  JSON_OBJ(JSON_LIST_2(                                                            \
			  JSON_NAME("event_type",                                                  \
				    JSON_STR(SID_CONTROL_EVENT_TYPE_STR(val->event_type))),        \
			  JSON_VAL_sid_link3_low_latency_config_t(                                 \
				  "event_data",                                                    \
				  ((struct sid_link3_low_latency_config *)val->event_data)))))

#define JSON_VAL_sid_bulk_data_transfer_desc(name, desc)                                           \
	JSON_NAME(name,                                                                            \
		  JSON_OBJ(JSON_LIST_3(JSON_NAME("link_type", JSON_INT(desc->link_type)),          \
				       JSON_NAME("file_id", JSON_INT(desc->file_id)),              \
				       JSON_NAME("file_offset", JSON_INT(desc->file_offset)))))

#define JSON_VAL_sid_bulk_data_transfer_request(name, req)                                         \
	JSON_NAME(name,                                                                            \
		  JSON_OBJ(JSON_LIST_4(JSON_NAME("fragment_size", JSON_INT(req->fragment_size)),   \
				       JSON_NAME("file_id", JSON_INT(req->file_id)),               \
				       JSON_NAME("file_size", JSON_INT(req->file_size)),           \
				       JSON_NAME("minimum_scratch_buffer_size",                    \
						 JSON_INT(req->minimum_scratch_buffer_size)))))
