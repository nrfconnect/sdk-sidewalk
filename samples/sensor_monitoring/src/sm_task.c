/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <app_ble_config.h>
#include <app_subGHz_config.h>
#include <sm_task.h>
#include <sm_leds.h>
#include <sm_buttons.h>
#include <sm_timers.h>
#include <sm_callbacks.h>
#include <sm_notifications.h>
#include <assert.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(sm_task, CONFIG_SIDEWALK_LOG_LEVEL);

#define RECEIVE_TASK_STACK_SIZE (4096)
#define RECEIVE_TASK_PRIORITY   (CONFIG_SIDEWALK_THREAD_PRIORITY + 1)

K_THREAD_STACK_DEFINE(sm_main_task_stack, CONFIG_SIDEWALK_THREAD_STACK_SIZE);
K_THREAD_STACK_DEFINE(sm_receive_task_stack, RECEIVE_TASK_STACK_SIZE);

K_MSGQ_DEFINE(sm_main_task_msgq, sizeof(enum event_type), CONFIG_SIDEWALK_THREAD_QUEUE_SIZE, 4);
K_MSGQ_DEFINE(sm_rx_task_msgq, sizeof(struct app_demo_rx_msg), CONFIG_SIDEWALK_THREAD_QUEUE_SIZE, 4);

static struct k_thread sm_receive_task;
static struct k_thread sm_main_task;

static app_context_t g_app_context = {
	.sidewalk_state = STATE_SIDEWALK_INIT,
	.app_state = DEMO_APP_STATE_INIT,
	.link_status.time_sync_status = SID_STATUS_NO_TIME,
};

static int32_t init_and_start_link(app_context_t *context, struct sid_config *config, uint32_t link_mask)
{
	struct sid_handle *sid_handle = NULL;

	LOG_INF("Initializing sidewalk with %s link mask.", SID_APP_LM_2_STR(link_mask));

	config->link_mask = link_mask;
	sid_error_t ret = sid_init(config, &sid_handle);

	if (ret != SID_ERROR_NONE) {
		LOG_ERR("Failed to initialize sidewalk err:%d", (int)ret);
		config->link_mask = 0;
		return -1;
	}
	context->sidewalk_handle = sid_handle;

	ret = sid_start(sid_handle, link_mask);
	if (ret != SID_ERROR_NONE) {
		LOG_ERR("Failed to start sidewalk, err:%d", (int)ret);
		(void)sid_deinit(context->sidewalk_handle);
		context->sidewalk_handle = NULL;
		config->link_mask = 0;
		return -1;
	}
	return 0;
}

static void factory_reset(app_context_t *context)
{
	sid_error_t ret = sid_set_factory_reset(context->sidewalk_handle);

	if (ret != SID_ERROR_NONE) {
		LOG_ERR("Notification of factory reset to sid api failed!");
	} else {
		LOG_INF("Wait for Sid api to notify to proceed with factory reset!");
	}
}

static void set_device_profile(struct sid_handle *sid_handle)
{
	sm_device_profile_timer_stop();
	struct sid_device_profile curr_dev_cfg = {
		.unicast_params = {
			.device_profile_id = SID_LINK2_PROFILE_2,
		},
	};
	sid_error_t ret = sid_option(sid_handle, SID_OPTION_900MHZ_GET_DEVICE_PROFILE,
				     &curr_dev_cfg, sizeof(curr_dev_cfg));

	if (ret == SID_ERROR_NONE) {
		struct sid_device_profile target_dev_cfg = {
			.unicast_params = {
				.device_profile_id = SID_LINK2_PROFILE_2,
				.rx_window_count = SID_RX_WINDOW_CNT_INFINITE,
				.unicast_window_interval = {
					.sync_rx_interval_ms =
						SID_LINK2_RX_WINDOW_SEPARATION_1,
				},
				.wakeup_type = SID_TX_AND_RX_WAKEUP,
			},
		};

		if (!memcmp(&curr_dev_cfg, &target_dev_cfg, sizeof(curr_dev_cfg))) {
			LOG_INF("Device profile for Link type 2 already set");
		} else {
			ret = sid_option(sid_handle,
					 SID_OPTION_900MHZ_SET_DEVICE_PROFILE,
					 &target_dev_cfg, sizeof(target_dev_cfg));
			if (ret != SID_ERROR_NONE) {
				LOG_ERR(
					"Device profile configuration for Link type 2 failed ret = %d",
					ret);
				sm_device_profile_timer_set_and_run(K_MSEC(PROFILE_CHECK_TIMER_DELAY_MS));
			} else {
				LOG_INF("Device profile Link type 2 set success");
			}
		}
	} else {
		LOG_ERR("Failed to get device profile configuration for Link type 2 ret = %d", ret);
		sm_device_profile_timer_set_and_run(K_MSEC(PROFILE_CHECK_TIMER_DELAY_MS));
	}
}

static void sidewalk_main_task(void *context, void *dummy1, void *dummy2)
{
	ARG_UNUSED(dummy1);
	ARG_UNUSED(dummy2);

	const struct sid_sub_ghz_links_config *sub_ghz_lc = NULL;

#ifdef CONFIG_SIDEWALK_LINK_MASK_FSK
	sub_ghz_lc = app_get_sub_ghz_config();
#elif CONFIG_SIDEWALK_LINK_MASK_LORA
	sub_ghz_lc = app_get_sub_ghz_config();
#endif

	app_context_t *app_context = (app_context_t *)context;
	struct sid_event_callbacks event_callbacks;

	sid_error_t ret_code = sm_callbacks_set(app_context, &event_callbacks);
	if (SID_ERROR_NONE != ret_code) {
		LOG_ERR("Failed to initialize callbacks, err: %d", (int)ret_code);
		return;
	}

	struct sid_config config = {
		.link_mask = 0,
		.time_sync_periodicity_seconds = 7200,
		.callbacks = &event_callbacks,
		.link_config = app_get_ble_config(),
		.sub_ghz_link_config = sub_ghz_lc,
	};

	if (init_and_start_link(app_context, &config, SID_LINK_TYPE_1 | BUILT_IN_LM)) {
		return;
	}

	struct sid_handle *sid_handle = app_context->sidewalk_handle;
	app_context->sidewalk_state = STATE_SIDEWALK_NOT_READY;

	LOG_INF("Starting %s ...", __FUNCTION__);

	while (1) {
		enum event_type event;

		if (!k_msgq_get(&sm_main_task_msgq, &event, K_FOREVER)) {
			switch (event) {
			case EVENT_TYPE_SIDEWALK:
			{
				sid_error_t ret = sid_process(sid_handle);
				if (ret) {
					LOG_WRN("Process error (code %d)", ret);
				}
				break;
			}
			case EVENT_FACTORY_RESET:
				factory_reset(app_context);
				break;
			case EVENT_BUTTON_PRESS:
				sm_notify_sensor_data(app_context, true);
				sm_buttons_pendig_flag_clear();
				break;
			case EVENT_NOTIFICATION_TIMER_FIRED:
				if (app_context->sidewalk_state != STATE_SIDEWALK_READY) {
					if (BUILT_IN_LM == SID_LINK_TYPE_1) {
						if (!(app_context->link_status.link_mask & SID_LINK_TYPE_1) &&
						    (app_context->app_state == DEMO_APP_STATE_REGISTERED)
						    && (app_context->link_status.time_sync_status ==
							SID_STATUS_TIME_SYNCED)) {
							sm_main_task_msg_q_write(EVENT_CONNECT_LINK_TYPE_1);
						}
					}
				} else if (app_context->app_state == DEMO_APP_STATE_NOTIFY_CAPABILITY) {
					sm_notify_capability(app_context);
				} else if (app_context->app_state == DEMO_APP_STATE_NOTIFY_SENSOR_DATA) {
					sm_notify_sensor_data(app_context, false);
				}
				break;
			case EVENT_BUTTON_PRESS_TIMER_FIRED:
				LOG_INF("Button press timeout check timer fired");
				sm_check_button_press_notify(app_context);
				break;
			case EVENT_CONNECT_LINK_TYPE_1:
				if (BUILT_IN_LM == SID_LINK_TYPE_1) {
					LOG_INF("Connecting link type 1");
					sid_error_t ret = sid_ble_bcn_connection_request(sid_handle, true);
					if (ret != SID_ERROR_NONE) {
						LOG_ERR("Failed to set connect request on link type 1 %d", ret);
					}
				}
				break;
			case EVENT_SET_DEVICE_PROFILE_LINK_TYPE_2:
				if (BUILT_IN_LM == SID_LINK_TYPE_2) {
					set_device_profile(sid_handle);
				}
				break;
			default:
				LOG_ERR("Invalid event queued %d", event);
				break;
			}
		}
	}
}

static void sidewalk_receive_task(void *context, void *dummy1, void *dummy2)
{
	ARG_UNUSED(context);
	ARG_UNUSED(dummy1);
	ARG_UNUSED(dummy2);

	LOG_INF("Starting %s ...", __FUNCTION__);

	while (1) {
		struct app_demo_rx_msg rx_msg;
		if (!k_msgq_get(&sm_rx_task_msgq, &rx_msg, K_FOREVER)) {
			struct sid_demo_msg_desc msg_desc = { 0 };
			static uint8_t temp_msg_payload[PAYLOAD_MAX_SIZE];
			memset(temp_msg_payload, 0, sizeof(temp_msg_payload));
			struct sid_demo_msg msg = { .payload = temp_msg_payload };
			static struct sid_parse_state state;
			memset(&state, 0, sizeof(state));

			sid_parse_state_init(&state, rx_msg.rx_payload, rx_msg.pld_size);
			sid_demo_app_msg_deserialize(&state, &msg_desc, &msg);
			LOG_INF("opc %d, class %d cmd %d status indicator %d status_code %d paylaod size %d",
				msg_desc.opc, msg_desc.cmd_class, msg_desc.cmd_id, msg_desc.status_hdr_ind,
				msg_desc.status_code, msg.payload_size);

			if (state.ret_code != SID_ERROR_NONE) {
				LOG_ERR("De-serialize demo app msg failed %d", state.ret_code);
			} else if (msg_desc.cmd_class == SID_DEMO_APP_CLASS) {
				if (msg_desc.status_hdr_ind && msg_desc.opc == SID_DEMO_MSG_TYPE_RESP &&
				    msg_desc.status_code == SID_ERROR_NONE) {
					if (msg_desc.cmd_id == SID_DEMO_APP_CLASS_CMD_CAP_DISCOVERY_ID
					    && msg.payload_size == 0) {
						LOG_INF("Capability response received");
						g_app_context.app_state = DEMO_APP_STATE_NOTIFY_SENSOR_DATA;
					} else if (msg_desc.cmd_id == SID_DEMO_APP_CLASS_CMD_ACTION) {
						LOG_INF("Action response received");
						sid_parse_state_init(&state, msg.payload, msg.payload_size);
						sm_buttons_action_response_process(&state);
					}
				} else if (msg_desc.opc == SID_DEMO_MSG_TYPE_WRITE &&
					   msg_desc.cmd_id == SID_DEMO_APP_CLASS_CMD_ACTION) {
					LOG_INF("Action request received");
					sid_parse_state_init(&state, msg.payload, msg.payload_size);
					sm_leds_action_request_process(&g_app_context, &state);
				}
			}
		}
	}
}

void sm_main_task_msg_q_write(enum event_type event)
{
	while (k_msgq_put(&sm_main_task_msgq, &event, K_NO_WAIT)) {
		LOG_WRN("The sm_main_task_msgq queue is full, purge old data");
		k_msgq_purge(&sm_main_task_msgq);
	}
}

void sm_rx_task_msg_q_write(struct app_demo_rx_msg *rx_msg)
{
	while (k_msgq_put(&sm_rx_task_msgq, rx_msg, K_NO_WAIT)) {
		LOG_WRN("The sm_rx_task_msgq queue is full, purge old data");
		k_msgq_purge(&sm_rx_task_msgq);
	}
}

bool sm_is_sidewalk_ready()
{
	return (g_app_context.sidewalk_state == STATE_SIDEWALK_READY);
}

enum demo_app_state sm_app_state_get()
{
	return g_app_context.app_state;
}

void sm_task_start(void)
{
	(void)k_thread_create(&sm_main_task, sm_main_task_stack,
			      K_THREAD_STACK_SIZEOF(sm_main_task_stack),
			      sidewalk_main_task, &g_app_context, NULL, NULL,
			      CONFIG_SIDEWALK_THREAD_PRIORITY, 0, K_NO_WAIT);

	(void)k_thread_create(&sm_receive_task, sm_receive_task_stack,
			      K_THREAD_STACK_SIZEOF(sm_receive_task_stack),
			      sidewalk_receive_task, NULL, NULL, NULL,
			      RECEIVE_TASK_PRIORITY, 0, K_NO_WAIT);

	k_thread_name_set(&sm_main_task, "sm_main_task");
	k_thread_name_set(&sm_receive_task, "sm_receive_task");
}

void sm_send_msg(const app_context_t *app_context, struct sid_msg_desc *desc, struct sid_msg *msg)
{
	assert(app_context);
	assert(desc);
	assert(msg);

	if (app_context->sidewalk_state == STATE_SIDEWALK_READY ||
	    app_context->sidewalk_state == STATE_SIDEWALK_SECURE_CONNECTION) {
		sid_error_t ret = sid_put_msg(app_context->sidewalk_handle, msg, desc);
		if (ret != SID_ERROR_NONE) {
			LOG_ERR("failed queueing data, err:%d", (int)ret);
			return;
		}
		LOG_INF("queued data message id:%u", desc->id);
	} else {
		LOG_ERR("sidewalk is not ready yet!");
	}
}

enum sid_link_type sm_active_link_type_get(const app_context_t *app_context)
{
	assert(app_context);

	if (app_context->link_status.link_mask & SID_LINK_TYPE_2) {
		return SID_LINK_TYPE_2;
	} else if (app_context->link_status.link_mask & SID_LINK_TYPE_3) {
		return SID_LINK_TYPE_3;
	}
	return SID_LINK_TYPE_1;
}
