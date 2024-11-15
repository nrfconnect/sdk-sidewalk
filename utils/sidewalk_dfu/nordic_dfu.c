/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <sidewalk_dfu/nordic_dfu.h>
#include <dk_buttons_and_leds.h>
#include <stdbool.h>
#include <string.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/mgmt/mcumgr/transport/smp_bt.h>
#include <zephyr/mgmt/mcumgr/mgmt/callbacks.h>
#include <zephyr/sys/reboot.h>
#include <state_notifier/state_notifier.h>
#include <bt_app_callbacks.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(nordic_dfu, CONFIG_SIDEWALK_LOG_LEVEL);

#define LED_PERIOD_TOGGLE_ALL 500
#define LED_PERIOD_LOADING_WHEEL 150

static void pending_adv_start(struct k_work *work);
static struct bt_le_ext_adv *adv = NULL;
static struct bt_le_ext_adv_start_param ext_adv_param = { .num_events = 0, .timeout = 0 };
static struct k_work_delayable adv_work = Z_WORK_DELAYABLE_INITIALIZER(pending_adv_start);

static struct bt_le_adv_param adv_params = { .id = BT_ID_DEFAULT,
					     .sid = 0,
					     .secondary_max_skip = 0,
					     .options = BT_LE_ADV_OPT_CONNECTABLE,
					     .interval_min = BT_GAP_ADV_SLOW_INT_MIN,
					     .interval_max = BT_GAP_ADV_SLOW_INT_MAX,
					     .peer = NULL };

static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA_BYTES(BT_DATA_UUID128_ALL, 0x84, 0xaa, 0x60, 0x74, 0x52, 0x8a, 0x8b, 0x86, 0xd3,
		      0x4c, 0xb7, 0x1d, 0x1d, 0xdc, 0x53, 0x8d),
};

static const struct bt_data sd[] = {
	BT_DATA(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME, sizeof(CONFIG_BT_DEVICE_NAME) - 1),
};

static enum led_status_e {
	LED_TOGGLE_ALL,
	LED_LOADING_WHEEL,
} led_status;

static struct k_timer led_timer;
static struct k_timer exit_timer;

static int create_ble_id(void)
{
	int ret;
	size_t count = 0;

	/* Check if Bluetooth identites weren't already created. */
	bt_id_get(NULL, &count);
	if (count > BT_ID_SMP_DFU) {
		return 0;
	}

	do {
		ret = bt_id_create(NULL, NULL);
		if (ret < 0) {
			return ret;
		}
	} while (ret != BT_ID_SMP_DFU);

	return BT_ID_DEFAULT;
}

static void led_action(struct k_timer *timer_id)
{
	switch (led_status) {
	case LED_TOGGLE_ALL:
		static bool toggle_state = false;

		dk_set_leds(toggle_state ? 0 : DK_ALL_LEDS_MSK);
		toggle_state = !toggle_state;
		break;
	case LED_LOADING_WHEEL:
		static int led_state = 0;
		static int led_state_repo[] = {
			DK_LED1_MSK | DK_LED2_MSK,
			DK_LED2_MSK | DK_LED4_MSK,
			DK_LED4_MSK | DK_LED3_MSK,
			DK_LED3_MSK | DK_LED1_MSK,
		};

		dk_set_leds(led_state_repo[led_state]);
		led_state = (led_state + 1) % 4;
		break;
	}
}

static void exit_dfu_mode(struct k_timer *timer_id)
{
	LOG_ERR("DFU did not start or could not complete. Exit dfu mode");
}

static void connected(struct bt_conn *conn, uint8_t err)
{
	struct bt_conn_info cinfo;
	int ec;

	ec = bt_conn_get_info(conn, &cinfo);
	if (ec) {
		printk("Unable to get connection info (err %d)\n", ec);
		return;
	}

	if (cinfo.id != adv_params.id) {
		return;
	}

	printk("Connected to SMP service: err %d id %d\n", err, cinfo.id);
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	int err;
	struct bt_conn_info cinfo;

	err = bt_conn_get_info(conn, &cinfo);
	if (err) {
		printk("Unable to get connection info (err %d)\n", err);
		return;
	}

	if (cinfo.id != adv_params.id) {
		return;
	}

	printk("Disconnected from SMP service: reason %d id %d\n", reason, cinfo.id);

	k_work_schedule(&adv_work, K_NO_WAIT);
}

static struct bt_conn_cb conn_callbacks = {
	.connected = connected,
	.disconnected = disconnected,
};

static enum mgmt_cb_return dfu_mode_cb(uint32_t event, enum mgmt_cb_return prev_status, int32_t *rc,
				       uint16_t *group, bool *abort_more, void *data,
				       size_t data_size)
{
	ARG_UNUSED(prev_status);
	ARG_UNUSED(rc);
	ARG_UNUSED(group);
	ARG_UNUSED(abort_more);
	ARG_UNUSED(data);
	ARG_UNUSED(data_size);

	switch (event) {
	case MGMT_EVT_OP_IMG_MGMT_DFU_STARTED:
		LOG_INF("DFU Started");
		k_timer_start(&exit_timer, K_MINUTES(CONFIG_DFU_UPLOAD_COMPLETE_TIMEOUT),
			      K_NO_WAIT);

		led_status = LED_LOADING_WHEEL;
		k_timer_start(&led_timer, K_MSEC(LED_PERIOD_LOADING_WHEEL),
			      K_MSEC(LED_PERIOD_LOADING_WHEEL));
		break;
	case MGMT_EVT_OP_IMG_MGMT_DFU_STOPPED:
		LOG_INF("DFU Stopped");
		sys_reboot(SYS_REBOOT_COLD);
		break;
	case MGMT_EVT_OP_IMG_MGMT_DFU_PENDING:
		LOG_INF("DFU Pending");
		break;
	case MGMT_EVT_OP_IMG_MGMT_DFU_CHUNK:
		break;
	default:
		LOG_ERR("Unknown event %d", event);
		break;
	}
	return MGMT_CB_OK;
}

static struct mgmt_callback dfu_mode_mgmt_cb = {
	.callback = dfu_mode_cb,
	.event_id = MGMT_EVT_OP_IMG_MGMT_DFU_STARTED | MGMT_EVT_OP_IMG_MGMT_DFU_STOPPED |
		    MGMT_EVT_OP_IMG_MGMT_DFU_PENDING | MGMT_EVT_OP_IMG_MGMT_DFU_CHUNK,
};

static void pending_adv_start(struct k_work *work)
{
	int err;

	err = bt_le_ext_adv_start(adv, &ext_adv_param);
	if (err) {
		printk("Unable to restart SMP service advertising (err %d)\n", err);
		return;
	}

	printk("SMP service advertising restarted\n");
}

int nordic_dfu_ble_start(void)
{
	LOG_INF("Entering into DFU mode");

	application_state_dfu(&global_state_notifier, true);
	dk_leds_init();

	int err = app_bt_enable(NULL);
	if (err && err != -EALREADY) {
		LOG_ERR("Bluetooth enable failed (err %d %s)", err, strerror(err));
		return err;
	}

	bt_conn_cb_register(&conn_callbacks);
	mgmt_callback_register(&dfu_mode_mgmt_cb);
	int ret = create_ble_id();
	if (ret < 0) {
		LOG_ERR("Failed to create BLE ID errno %d %s", ret, strerror(ret));
		return err;
	}

	adv_params.id = BT_ID_SMP_DFU;
	err = bt_le_ext_adv_create(&adv_params, NULL, &adv);
	if (err) {
		LOG_ERR("Bluetooth advertising create failed (err %d %s)", err, strerror(err));
		return err;
	}
	err = bt_le_ext_adv_set_data(adv, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
	if (err) {
		LOG_ERR("Bluetooth advertising data set failed (err %d %s)", err, strerror(err));
		return err;
	}
	err = bt_le_ext_adv_start(adv, &ext_adv_param);
	if (err) {
		LOG_ERR("Bluetooth advertising start failed (err %d %s)", err, strerror(err));
		return err;
	}

	LOG_INF("Advertising successfully started");

	k_timer_init(&exit_timer, exit_dfu_mode, NULL);
	k_timer_start(&exit_timer, K_MINUTES(CONFIG_DFU_UPLOAD_START_TIMEOUT), K_NO_WAIT);

	led_status = LED_TOGGLE_ALL;
	k_timer_init(&led_timer, led_action, NULL);
	k_timer_start(&led_timer, K_MSEC(LED_PERIOD_TOGGLE_ALL), K_MSEC(LED_PERIOD_TOGGLE_ALL));

	return 0;
}

int nordic_dfu_ble_stop(void)
{
	LOG_INF("Exiting DFU mode");

	bt_conn_cb_unregister(&conn_callbacks);
	mgmt_callback_unregister(&dfu_mode_mgmt_cb);

	int err = bt_le_ext_adv_stop(adv);
	if (err) {
		LOG_ERR("Bluetooth advertising stop failed (err %d %s)", err, strerror(err));
		return err;
	}
	err = bt_le_ext_adv_delete(adv);
	if (err) {
		LOG_ERR("Bluetooth advertising delete failed (err %d %s)", err, strerror(err));
		return err;
	}
	adv = NULL;

	err = app_bt_disable();
	if (err) {
		LOG_ERR("Bluetooth disable failed (err %d %s)", err, strerror(err));
		return err;
	}

	LOG_INF("Advertising successfully stoped");

	k_timer_stop(&led_timer);
	k_timer_stop(&exit_timer);

	application_state_dfu(&global_state_notifier, false);

	return 0;
}
