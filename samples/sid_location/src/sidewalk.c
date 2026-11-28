/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <sidewalk.h>

#include <app_ble_config.h>
#include <app_mfg_config.h>
#include <app_subGHz_config.h>
#include <bt_app_callbacks.h>
#include <json_printer/sidTypes2str.h>
#include <sid_hal_reset_ifc.h>
#include <sid_pal_common_ifc.h>
#ifdef CONFIG_SIDEWALK_SUBGHZ_SUPPORT
#include <sid_pal_radio_ifc.h>
#endif /* CONFIG_SIDEWALK_SUBGHZ_SUPPORT */
#ifdef CONFIG_SIDEWALK_SUBGHZ_RADIO_LR1110
#include <app_location_lr11xx_config.h>
#endif /* CONFIG_SIDEWALK_SUBGHZ_RADIO_LR1110 */

#include <stdio.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(sid_location_app, CONFIG_SIDEWALK_LOG_LEVEL);

#ifdef CONFIG_SIDEWALK_SUBGHZ_SUPPORT
#define APP_LINK_MASK (SID_LINK_TYPE_1 | SID_LINK_TYPE_3)
#else
#define APP_LINK_MASK (SID_LINK_TYPE_1)
#endif /* CONFIG_SIDEWALK_SUBGHZ_SUPPORT */

#ifdef CONFIG_SIDEWALK_SUBGHZ_REGION_EU868
#define APP_SUBGHZ_REGION SID_PAL_RADIO_RC_EU
#else
#define APP_SUBGHZ_REGION SID_PAL_RADIO_RC_NA
#endif /* CONFIG_SIDEWALK_SUBGHZ_REGION_EU868 */

#ifdef CONFIG_SIDEWALK_SUBGHZ_RADIO_LR1110
#define APP_LOCATION_METHODS                                                                       \
	(SID_LOCATION_METHOD_GATEWAY | SID_LOCATION_METHOD_WIFI | SID_LOCATION_METHOD_GNSS)
#define APP_LOCATION_MAX_EFFORT SID_LOCATION_EFFORT_L4
#define APP_LOCATION_AUTO_MODE	SID_LOCATION_EFFORT_DEFAULT
#else
#define APP_LOCATION_METHODS	SID_LOCATION_METHOD_GATEWAY
#define APP_LOCATION_MAX_EFFORT SID_LOCATION_EFFORT_L1
#define APP_LOCATION_AUTO_MODE	SID_LOCATION_EFFORT_L1
#endif /* CONFIG_SIDEWALK_SUBGHZ_RADIO_LR1110 */

static struct sid_handle *handle;
static const struct sidewalk_callbacks *app_callbacks;

static void on_sidewalk_event(bool in_isr, void *context)
{
	ARG_UNUSED(in_isr);
	ARG_UNUSED(context);
	app_callbacks->on_process();
}

static void on_sidewalk_status_changed(const struct sid_status *status, void *context)
{
	ARG_UNUSED(context);

	LOG_INF("Sidewalk state %d, registered %d, time synced %d, links 0x%x", status->state,
		status->detail.registration_status == SID_STATUS_REGISTERED,
		status->detail.time_sync_status == SID_STATUS_TIME_SYNCED,
		status->detail.link_status_mask);
	app_callbacks->on_status(status);
}

static void on_sidewalk_msg_received(const struct sid_msg_desc *msg_desc, const struct sid_msg *msg,
				     void *context)
{
	ARG_UNUSED(msg_desc);
	ARG_UNUSED(msg);
	ARG_UNUSED(context);
}

static void on_sidewalk_msg_sent(const struct sid_msg_desc *msg_desc, void *context)
{
	ARG_UNUSED(msg_desc);
	ARG_UNUSED(context);
}

static void on_sidewalk_send_error(sid_error_t error, const struct sid_msg_desc *msg_desc,
				   void *context)
{
	ARG_UNUSED(msg_desc);
	ARG_UNUSED(context);
	LOG_WRN("Send error %d (%s)", (int)error, SID_ERROR_T_STR(error));
}

static void on_sidewalk_factory_reset(void *context)
{
	ARG_UNUSED(context);

	LOG_INF("Factory reset done, rebooting");
	if (sid_hal_reset(SID_HAL_RESET_NORMAL)) {
		LOG_WRN("Cannot reboot");
	}
}

static void on_location_update(const struct sid_location_result *const result, void *context)
{
	ARG_UNUSED(context);
	app_callbacks->on_location(result);
}

#ifdef CONFIG_SIDEWALK_SUBGHZ_SUPPORT
static sid_pal_radio_rx_packet_t radio_rx_packet;

static void radio_event_notifier(sid_pal_radio_events_t event)
{
	ARG_UNUSED(event);
}

static void radio_irq_handler(void)
{
}
#endif /* CONFIG_SIDEWALK_SUBGHZ_SUPPORT */

static int platform_init(void)
{
	platform_parameters_t platform_parameters = {
		.mfg_store_region.addr_start = APP_MFG_CFG_FLASH_START,
		.mfg_store_region.addr_end = APP_MFG_CFG_FLASH_END,
#ifdef CONFIG_SIDEWALK_SUBGHZ_SUPPORT
		.platform_init_parameters.radio_cfg = get_radio_cfg(),
		.platform_init_parameters.region = APP_SUBGHZ_REGION,
		.platform_init_parameters.sub_ghz_cfg = app_get_sub_ghz_config(),
#ifdef CONFIG_SIDEWALK_SUBGHZ_RADIO_LR1110
		.platform_init_parameters.gnss_wifi_cfg =
			(lr11xx_gnss_wifi_config_t *)get_location_cfg(),
#endif /* CONFIG_SIDEWALK_SUBGHZ_RADIO_LR1110 */
#endif /* CONFIG_SIDEWALK_SUBGHZ_SUPPORT */
	};

	sid_error_t err = sid_platform_init(&platform_parameters);

	if (err) {
		LOG_ERR("Sidewalk platform init err %d (%s)", (int)err, SID_ERROR_T_STR(err));
		return -EIO;
	}

	if (app_mfg_cfg_is_empty()) {
		LOG_ERR("No manufacturing data at 0x%08x, provision the device",
			APP_MFG_CFG_FLASH_START);
		return -ENODATA;
	}

#ifdef CONFIG_SIDEWALK_SUBGHZ_SUPPORT
	int32_t radio_err =
		sid_pal_radio_init(radio_event_notifier, radio_irq_handler, &radio_rx_packet);

	if (radio_err) {
		LOG_ERR("Radio init err %d", radio_err);
		return -EIO;
	}
	(void)sid_pal_radio_sleep(0);
#endif /* CONFIG_SIDEWALK_SUBGHZ_SUPPORT */

	return 0;
}

/* The Sidewalk and SMP services are always registered, each identity may use only its own */
static bool gatt_authorize(struct bt_conn *conn, const struct bt_gatt_attr *attr)
{
	struct bt_conn_info info;

	if (bt_conn_get_info(conn, &info)) {
		return false;
	}
	if (info.id == BT_ID_SIDEWALK) {
		return !sid_ble_bt_attr_is_SMP(attr);
	}
	if (info.id == BT_ID_SMP_DFU) {
		return !sid_ble_bt_attr_is_SIDEWALK(attr);
	}
	return true;
}

static const struct bt_gatt_authorization_cb gatt_authorization = {
	.read_authorize = gatt_authorize,
	.write_authorize = gatt_authorize,
};

int sidewalk_init(const struct sidewalk_callbacks *callbacks)
{
	static struct sid_event_callbacks sid_callbacks = {
		.on_event = on_sidewalk_event,
		.on_msg_received = on_sidewalk_msg_received,
		.on_msg_sent = on_sidewalk_msg_sent,
		.on_send_error = on_sidewalk_send_error,
		.on_status_changed = on_sidewalk_status_changed,
		.on_factory_reset = on_sidewalk_factory_reset,
	};
	struct sid_config config = {
		.link_mask = APP_LINK_MASK,
		.dev_ch = {
			.type = SID_END_DEVICE_TYPE_STATIC,
			.power_type = SID_END_DEVICE_POWERED_BY_BATTERY_AND_LINE_POWER,
			.qualification_id = 0x0001,
		},
		.callbacks = &sid_callbacks,
		.link_config = app_get_ble_config(),
		.sub_ghz_link_config = app_get_sub_ghz_config(),
	};

	app_callbacks = callbacks;

	int ret = platform_init();

	if (ret) {
		return ret;
	}

	if (bt_gatt_authorization_cb_register(&gatt_authorization)) {
		LOG_ERR("GATT authorization register failed");
		return -EIO;
	}

	sid_error_t err = sid_init(&config, &handle);

	if (err) {
		LOG_ERR("Sidewalk init err %d (%s)", (int)err, SID_ERROR_T_STR(err));
		return -EIO;
	}

	ret = sidewalk_start();
	if (ret) {
		return ret;
	}

	enum sid_link_connection_policy policy = SID_LINK_CONNECTION_POLICY_AUTO_CONNECT;
	struct sid_link_auto_connect_params params = {
		.link_type = SID_LINK_TYPE_1,
		.enable = true,
		.connection_attempt_timeout_seconds = SIDEWALK_BLE_CONNECT_TIMEOUT_S,
	};

	(void)sid_option(handle, SID_OPTION_SET_LINK_CONNECTION_POLICY, &policy, sizeof(policy));
	(void)sid_option(handle, SID_OPTION_SET_LINK_POLICY_AUTO_CONNECT_PARAMS, &params,
			 sizeof(params));

	return 0;
}

void sidewalk_process(void)
{
	(void)sid_process(handle);
}

int sidewalk_start(void)
{
	sid_error_t err = sid_start(handle, APP_LINK_MASK);

	if (err) {
		LOG_ERR("Sidewalk start err %d (%s)", (int)err, SID_ERROR_T_STR(err));
		return -EIO;
	}
	return 0;
}

int sidewalk_stop(void)
{
	sid_error_t err = sid_stop(handle, APP_LINK_MASK);

	if (err) {
		LOG_ERR("Sidewalk stop err %d (%s)", (int)err, SID_ERROR_T_STR(err));
		return -EIO;
	}
	return 0;
}

void sidewalk_ble_connect_request(void)
{
	(void)sid_ble_bcn_connection_request(handle, true);
}

int sidewalk_location_init(void)
{
	struct sid_location_config config = {
		.sid_location_type_mask = APP_LOCATION_METHODS,
		.max_effort = APP_LOCATION_MAX_EFFORT,
		.manage_effort = true,
		.callbacks = {
			.on_update = on_location_update,
		},
		.auto_link_pause_resume = IS_ENABLED(CONFIG_SIDEWALK_SUBGHZ_SUPPORT),
	};

	sid_error_t err = sid_location_init(handle, &config);

	if (err) {
		LOG_ERR("Location init err %d (%s)", (int)err, SID_ERROR_T_STR(err));
		return -EIO;
	}
	return 0;
}

static int location_run(struct sid_location_run_config *config)
{
	sid_error_t err = sid_location_run(handle, config, 0);

	if (err) {
		LOG_ERR("Location run err %d (%s)", (int)err, SID_ERROR_T_STR(err));
		return -EIO;
	}
	LOG_INF("Location run started at L%d", sid_location_get_effort_mode(handle));
	return 0;
}

int sidewalk_location_run_auto(void)
{
	struct sid_location_run_config config = {
		.mode = APP_LOCATION_AUTO_MODE,
		.type = SID_LOCATION_SCAN_AND_SEND,
	};

	return location_run(&config);
}

int sidewalk_location_run_level(enum sid_location_effort_mode mode)
{
	struct sid_location_run_config config = {
		.type = SID_LOCATION_SCAN_AND_SEND,
		.levels = {mode},
		.num_levels = 1,
	};

	if (!IS_ENABLED(CONFIG_SIDEWALK_SUBGHZ_RADIO_LR1110)) {
		return -ENOTSUP;
	}
	return location_run(&config);
}

enum sid_location_effort_mode sidewalk_location_effort(void)
{
	return sid_location_get_effort_mode(handle);
}

int sidewalk_location_msg_send(void)
{
	static unsigned int counter;
	char text[24];
	struct sid_msg msg = {
		.data = text,
		.size = snprintf(text, sizeof(text), "Message %u", ++counter),
	};
	struct sid_msg_desc desc = {
		.type = SID_MSG_TYPE_NOTIFY,
		.link_type = SID_LINK_TYPE_1,
		.link_mode = SID_LINK_MODE_CLOUD,
		.msg_desc_attr.tx_attr.resolve_location = SID_TX_LOCATION_LINK_RESOLVE,
	};

	sid_error_t err = sid_put_msg(handle, &msg, &desc);

	if (err) {
		LOG_ERR("Message err %d (%s)", (int)err, SID_ERROR_T_STR(err));
		return -EIO;
	}
	LOG_INF("\"%s\" with location resolution queued", text);
	return 0;
}
