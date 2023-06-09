/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <sid_api.h>
#include <sid_error.h>
#include <sid_900_cfg.h>

#include <state_notifier.h>
#include <board_events.h>

#if defined(CONFIG_SIDEWALK_DFU)
#include <nordic_dfu.h>
#endif

#include <zephyr/logging/log.h>
#include <zephyr/settings/settings.h>
#include <zephyr/sys/reboot.h>
#include <zephyr/kernel.h>

LOG_MODULE_REGISTER(board_events, CONFIG_SIDEWALK_LOG_LEVEL);

#if defined(CONFIG_SIDEWALK_LINK_MASK_FSK)
static struct sid_device_profile profile_light = {
	.unicast_params.device_profile_id = SID_LINK2_PROFILE_1,
	.unicast_params.rx_window_count = SID_RX_WINDOW_CNT_INFINITE,
	.unicast_params.unicast_window_interval.sync_rx_interval_ms =
		SID_LINK2_RX_WINDOW_SEPARATION_3,
	.unicast_params.wakeup_type = SID_TX_AND_RX_WAKEUP,
};

static struct sid_device_profile profile_fast = {
	.unicast_params.device_profile_id = SID_LINK2_PROFILE_2,
	.unicast_params.rx_window_count = SID_RX_WINDOW_CNT_INFINITE,
	.unicast_params.unicast_window_interval.sync_rx_interval_ms =
		SID_LINK2_RX_WINDOW_SEPARATION_3,
	.unicast_params.wakeup_type = SID_TX_AND_RX_WAKEUP,
};

static struct sid_device_profile profile_from_dev = { .unicast_params.device_profile_id =
							      SID_LINK2_PROFILE_1 };
#elif defined(CONFIG_SIDEWALK_LINK_MASK_LORA)
static struct sid_device_profile profile_light = {
	.unicast_params.device_profile_id = SID_LINK3_PROFILE_A,
	.unicast_params.rx_window_count = SID_RX_WINDOW_CNT_2,
	.unicast_params.unicast_window_interval.async_rx_interval_ms =
		SID_LINK3_RX_WINDOW_SEPARATION_3,
	.unicast_params.wakeup_type = SID_TX_AND_RX_WAKEUP,
};

static struct sid_device_profile profile_fast = {
	.unicast_params.device_profile_id = SID_LINK3_PROFILE_B,
	.unicast_params.rx_window_count = SID_RX_WINDOW_CNT_INFINITE,
	.unicast_params.unicast_window_interval.async_rx_interval_ms =
		SID_LINK3_RX_WINDOW_SEPARATION_3,
	.unicast_params.wakeup_type = SID_TX_AND_RX_WAKEUP,
};

static struct sid_device_profile profile_from_dev = { .unicast_params.device_profile_id =
							      SID_LINK3_PROFILE_A };
#endif

void button_event_send_hello(app_ctx_t *application_ctx)
{
	struct sid_status status = { .state = SID_STATE_NOT_READY };
	sid_error_t err;

	static uint8_t counter = 0;
	static struct sid_msg msg;
	static struct sid_msg_desc desc;

	err = sid_get_status(application_ctx->handle, &status);
	switch (err) {
	case SID_ERROR_NONE:
		break;
	case SID_ERROR_INVALID_ARGS:
		LOG_ERR("Sidewalk library is not initialzied!");
		return;
	default:
		LOG_ERR("Unknown error during sid_get_status() -> %d", err);
		return;
	}

	if (status.state != SID_STATE_READY && status.state != SID_STATE_SECURE_CHANNEL_READY) {
		LOG_ERR("Sidewalk Status is invalid!, expected SID_STATE_READY or SID_STATE_SECURE_CHANNEL_READY, got %d",
			status.state);
		return;
	}

	msg = (struct sid_msg){ .data = (uint8_t *)&counter, .size = sizeof(uint8_t) };
	desc = (struct sid_msg_desc){
		.type = SID_MSG_TYPE_NOTIFY,
		.link_type = SID_LINK_TYPE_ANY,
		.link_mode = SID_LINK_MODE_CLOUD,
	};

	desc.msg_desc_attr.tx_attr.request_ack = true;
	desc.msg_desc_attr.tx_attr.num_retries = 3;
	desc.msg_desc_attr.tx_attr.ttl_in_seconds = 20;

	err = sid_put_msg(application_ctx->handle, &msg, &desc);
	switch (err) {
	case SID_ERROR_NONE: {
		application_state_sending(&global_state_notifier, true);
		counter++;
		LOG_INF("queued data message id:%d", desc.id);
		break;
	}
	case SID_ERROR_TRY_AGAIN: {
		LOG_ERR("there is no space in the transmit queue, Try again.");
		break;
	}
	default:
		LOG_ERR("Unknown error returned from sid_put_msg() -> %d", err);
	}
}

void button_event_set_battery(app_ctx_t *application_ctx)
{
	static uint8_t fake_bat_lev = 70;

	++fake_bat_lev;
	if (fake_bat_lev > 100) {
		fake_bat_lev = 0;
	}
	sid_error_t ret = sid_option(application_ctx->handle, SID_OPTION_BLE_BATTERY_LEVEL,
				     &fake_bat_lev, sizeof(fake_bat_lev));

	if (SID_ERROR_NONE != ret) {
		LOG_ERR("failed setting Sidewalk option!");
	} else {
		LOG_DBG("set battery level to %d", fake_bat_lev);
	}
}

#if defined(CONFIG_SIDEWALK_DFU_SERVICE_BLE)
void button_event_DFU(app_ctx_t *application_ctx)
{
	bool DFU_mode = true;

	(void)settings_save_one(CONFIG_DFU_FLAG_SETTINGS_KEY, (const void *)&DFU_mode,
				sizeof(DFU_mode));

	sid_deinit(application_ctx->handle);
	k_sleep(K_SECONDS(1));

	sys_reboot(SYS_REBOOT_COLD);
}

#endif /* CONFIG_SIDEWALK_DFU_SERVICE_BLE */

void button_event_factory_reset(app_ctx_t *application_ctx)
{
	sid_error_t ret = sid_set_factory_reset(application_ctx->handle);

	if (SID_ERROR_NONE != ret) {
		LOG_ERR("Notification of factory reset to sid api failed!");
	} else {
		LOG_DBG("Wait for Sid api to notify to proceed with factory reset!");
	}
}

void button_event_get_profile(app_ctx_t *application_ctx)
{
	sid_error_t ret = sid_option(application_ctx->handle, SID_OPTION_900MHZ_GET_DEVICE_PROFILE,
				     &profile_from_dev, sizeof(profile_from_dev));

	if (ret) {
		LOG_ERR("Profile get failed (err %d)", ret);
		return;
	}

	LOG_INF("\n"
		"Profile id 0x%x\n"
		"Profile dl count %d\n"
		"Profile dl interval %d\n"
		"Profile wakeup %d\n",
		profile_from_dev.unicast_params.device_profile_id,
		profile_from_dev.unicast_params.rx_window_count,
		profile_from_dev.unicast_params.unicast_window_interval.async_rx_interval_ms,
		profile_from_dev.unicast_params.wakeup_type);
}

void button_event_set_ptofile(app_ctx_t *application_ctx)
{
	static struct sid_device_profile *new_profile = &profile_light;

	LOG_INF("Profile set %s", (&profile_light == new_profile) ? "light" : "fast");

	sid_error_t ret = sid_option(application_ctx->handle, SID_OPTION_900MHZ_SET_DEVICE_PROFILE,
				     new_profile, sizeof(*new_profile));

	if (!ret) {
		new_profile = (&profile_light == new_profile) ? &profile_fast : &profile_light;
		LOG_INF("Profile set success.");
	} else {
		LOG_ERR("Profile set failed (err %d)", ret);
	}
}
