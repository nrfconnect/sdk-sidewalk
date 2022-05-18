/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#include <sid_api.h>
#include <sid_error.h>

#include <logging/log.h>
LOG_MODULE_REGISTER(app, LOG_LEVEL_DBG);

static void on_sidewalk_event(bool in_isr, void *context)
{
	LOG_INF("In func: %s", __func__);
}
static void on_sidewalk_msg_received(const struct sid_msg_desc *msg_desc, const struct sid_msg *msg, void *context)
{
	LOG_INF("In func: %s", __func__);
}
static void on_sidewalk_msg_sent(const struct sid_msg_desc *msg_desc, void *context)
{
	LOG_INF("In func: %s", __func__);
}
static void on_sidewalk_send_error(sid_error_t error, const struct sid_msg_desc *msg_desc, void *context)
{
	LOG_INF("In func: %s", __func__);
}
static void on_sidewalk_status_changed(const struct sid_status *status, void *context)
{
	LOG_INF("In func: %s", __func__);
}
static void on_sidewalk_factory_reset(void *context)
{
	LOG_INF("In func: %s", __func__);
}

void main(void)
{
	LOG_INF("Hello Sidewalk World! %s\n", CONFIG_BOARD);

	struct sid_event_callbacks event_callbacks = {
		.context = NULL,
		.on_event = on_sidewalk_event,                          /* Called from ISR context */
		.on_msg_received = on_sidewalk_msg_received,            /* Called from sid_process() */
		.on_msg_sent = on_sidewalk_msg_sent,                    /* Called from sid_process() */
		.on_send_error = on_sidewalk_send_error,                /* Called from sid_process() */
		.on_status_changed = on_sidewalk_status_changed,        /* Called from sid_process() */
		.on_factory_reset = on_sidewalk_factory_reset,          /* Called from sid_process */
	};

	struct sid_config config = {
		.link_mask = SID_LINK_TYPE_1,
		.callbacks = &event_callbacks,
		.link_config = NULL,
	};

	struct sid_handle *sid_handle = NULL;
	sid_error_t ret = sid_init(&config, &sid_handle);

	if (ret != SID_ERROR_NONE) {
		LOG_ERR("failed to initialize sidewalk, err:%d", (int)ret);
	}

	ret = sid_start(sid_handle, SID_LINK_TYPE_1);
	if (ret != SID_ERROR_NONE) {
		LOG_ERR("failed to start sidewalk, err:%d", (int)ret);
	}
}
