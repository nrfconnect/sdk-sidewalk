/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <string.h>

#include <app.h>
#include <sidewalk.h>
#include <app_ble_config.h>
#include <app_subGHz_config.h>
#include <sid_qa.h>
#include <sid_config_cli.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/reboot.h>

#include <bt_app_callbacks.h>
#include <cli/app_shell.h>
#if defined(CONFIG_SIDEWALK_DFU_SERVICE_BLE)
#include <cli/nordic_dfu_shell.h>
#endif

LOG_MODULE_REGISTER(app, CONFIG_SIDEWALK_LOG_LEVEL);

static bool gatt_authorize(struct bt_conn *conn, const struct bt_gatt_attr *attr)
{
	struct bt_conn_info cinfo = {};
	int ret = bt_conn_get_info(conn, &cinfo);
	if (ret != 0) {
		LOG_ERR("Failed to get id of connection err %d", ret);
		return false;
	}

	if (cinfo.id == BT_ID_SIDEWALK) {
		if (sid_ble_bt_attr_is_SMP(attr)) {
			return false;
		}
	}

#if defined(CONFIG_SIDEWALK_DFU)
	if (cinfo.id == BT_ID_SMP_DFU) {
		if (sid_ble_bt_attr_is_SIDEWALK(attr)) {
			return false;
		}
	}
#endif //defined(CONFIG_SIDEWALK_DFU)
	return true;
}

static const struct bt_gatt_authorization_cb gatt_authorization_callbacks = {
	.read_authorize = gatt_authorize,
	.write_authorize = gatt_authorize,
};

static void qa_reboot(void)
{
	sys_reboot(SYS_REBOOT_WARM);
}

static void qa_set_sub_ghz_cfg(struct sid_sub_ghz_links_config *sub_ghz_cfg)
{
#if defined(CONFIG_SIDEWALK_SUBGHZ_SUPPORT)
	if (sub_ghz_cfg != NULL) {
		memcpy(app_get_sub_ghz_config(), sub_ghz_cfg, sizeof(*sub_ghz_cfg));
	}
#else
	ARG_UNUSED(sub_ghz_cfg);
#endif /* CONFIG_SIDEWALK_SUBGHZ_SUPPORT */
}

static void app_event_qa_process(sidewalk_ctx_t *sid, void *ctx)
{
	static struct sid_qa_callbacks qa_callbacks = {
		.reboot_cmd = qa_reboot,
		.set_sub_ghz_cfg = qa_set_sub_ghz_cfg,
	};

	sid_error_t err;
	int ret;

	ARG_UNUSED(ctx);

	sid_qa_set_config(&sid->config, app_shell_qa_ifc());

	err = sid_qa_init(&qa_callbacks);
	if (err != SID_ERROR_NONE) {
		LOG_ERR("Sidewalk QA init err %d", (int)err);
		return;
	}

	err = sid_config_cli_init();
	if (err != SID_ERROR_NONE) {
		LOG_ERR("Sidewalk config CLI init err %d", (int)err);
		return;
	}

#if defined(CONFIG_SIDEWALK_DFU_SERVICE_BLE)
	ret = nordic_dfu_shell_init();
	if (ret) {
		LOG_ERR("Nordic DFU shell init err %d", ret);
		return;
	}
#endif /* CONFIG_SIDEWALK_DFU_SERVICE_BLE */

	ret = app_shell_init();
	if (ret) {
		LOG_ERR("Shell init err %d", ret);
		return;
	}

	/* From now on the Sidewalk thread is owned by the Sidewalk QA library. */
	while (true) {
		sid_qa_process(QA_PROC_WAIT);
	}
}

void app_start(void)
{
	static sidewalk_ctx_t sid_ctx = { 0 };

	struct sid_end_device_characteristics dev_ch = {
		.type = SID_END_DEVICE_TYPE_STATIC,
		.power_type = SID_END_DEVICE_POWERED_BY_BATTERY_AND_LINE_POWER,
		.qualification_id = 0x0001,
	};

	sid_ctx.config = (struct sid_config){
		.dev_ch = dev_ch,
		.link_config = app_get_ble_config(),
		.sub_ghz_link_config = app_get_sub_ghz_config(),
	};

	int err = bt_gatt_authorization_cb_register(&gatt_authorization_callbacks);
	if (err) {
		LOG_ERR("Registering GATT authorization callbacks failed (err %d)", err);
		return;
	}

	sidewalk_start(&sid_ctx);
	sidewalk_event_send(sidewalk_event_platform_init, NULL, NULL);
	sidewalk_event_send(app_event_qa_process, NULL, NULL);

#if CONFIG_SID_END_DEVICE_CLI_ALIVE_MARKER
	while (true) {
		k_sleep(K_MSEC(10000));
		LOG_INF("alive");
	}
#endif /* CONFIG_SID_END_DEVICE_CLI_ALIVE_MARKER */
}
