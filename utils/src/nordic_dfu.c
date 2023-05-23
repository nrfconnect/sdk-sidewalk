/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "nordic_dfu.h"
#include <dk_buttons_and_leds.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/mgmt/mcumgr/transport/smp_bt.h>
#include <zephyr/mgmt/mcumgr/grp/img_mgmt/img_mgmt.h>
#include <zephyr/mgmt/mcumgr/grp/img_mgmt/img_mgmt_callbacks.h>
#include <zephyr/mgmt/mcumgr/mgmt/callbacks.h>
#include <zephyr/sys/reboot.h>
#include <state_notifier.h>
#include <flashing_led.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(nordic_dfu, CONFIG_SIDEWALK_LOG_LEVEL);

static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA_BYTES(BT_DATA_UUID128_ALL,
		      0x84, 0xaa, 0x60, 0x74, 0x52, 0x8a, 0x8b, 0x86,
		      0xd3, 0x4c, 0xb7, 0x1d, 0x1d, 0xdc, 0x53, 0x8d),
};
struct k_work_delayable dfu_not_started_handler;
struct k_work_delayable failed_to_finish_update;
static pattern_id LED_pattern_id;
static void DFU_LED_controll(int led_state, void *ctx)
{
	ARG_UNUSED(ctx);
	dk_set_led(DK_LED1, led_state);
	dk_set_led(DK_LED2, led_state);
	dk_set_led(DK_LED3, led_state);
	dk_set_led(DK_LED4, led_state);
}

static void LED_bar_controll(int led_state, void *ctx)
{
	ARG_UNUSED(ctx);
	static int state = 0;

	switch (state) {
	case 0:
		dk_set_led(DK_LED1, true);
		dk_set_led(DK_LED2, true);
		dk_set_led(DK_LED4, false);
		dk_set_led(DK_LED3, false);
		break;
	case 1:
		dk_set_led(DK_LED1, false);
		dk_set_led(DK_LED2, true);
		dk_set_led(DK_LED4, true);
		dk_set_led(DK_LED3, false);
		break;
	case 2:
		dk_set_led(DK_LED1, false);
		dk_set_led(DK_LED2, false);
		dk_set_led(DK_LED4, true);
		dk_set_led(DK_LED3, true);
		break;
	case 3:
		dk_set_led(DK_LED1, true);
		dk_set_led(DK_LED2, false);
		dk_set_led(DK_LED4, false);
		dk_set_led(DK_LED3, true);
		break;
	default:
		break;
	}
	state++;
	state = state % 4;
}

DEFINE_PATTERN(DFU_mode_pattern, 500);

DEFINE_PATTERN(DFU_transfer_PATTERN, 150);

static int32_t dfu_mode_cb(uint32_t event, int32_t rc, bool *abort_more,
			   void *data, size_t data_size)
{
	switch (event) {
	case MGMT_EVT_OP_IMG_MGMT_DFU_STARTED:
		LOG_INF("DFU Started");
		k_work_cancel_delayable(&dfu_not_started_handler);
		stop_toggle_pattern(LED_pattern_id);
		LED_pattern_id = play_toggle_pattern(false, LED_bar_controll, NULL, DFU_transfer_PATTERN, LED_PATTERN_LENGTH(
							     DFU_transfer_PATTERN), true);

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
	return MGMT_ERR_EOK;
}

static void exit_dfu_mode(struct k_work *work)
{
	LOG_ERR("DFU did not start or could not complete. Reset to exit dfu mode");
	k_sleep(K_MSEC(100));
	sys_reboot(SYS_REBOOT_COLD);
}

static struct mgmt_callback dfu_mode_mgmt_cb = {
	.callback = dfu_mode_cb,
	.event_id = MGMT_EVT_OP_IMG_MGMT_DFU_STARTED |
		    MGMT_EVT_OP_IMG_MGMT_DFU_STOPPED |
		    MGMT_EVT_OP_IMG_MGMT_DFU_PENDING |
		    MGMT_EVT_OP_IMG_MGMT_DFU_CHUNK,
};

int nordic_dfu_ble_start(void)
{
	LOG_INF("Entering into DFU mode");
	application_state_dfu(&global_state_notifier, true);
	int err = bt_enable(NULL);

	mgmt_callback_register(&dfu_mode_mgmt_cb);

	k_work_init_delayable(&dfu_not_started_handler, exit_dfu_mode);
	k_work_reschedule(&dfu_not_started_handler, K_MINUTES(CONFIG_DFU_UPLOAD_START_TIMEOUT));

	k_work_init_delayable(&failed_to_finish_update, exit_dfu_mode);
	k_work_reschedule(&failed_to_finish_update, K_MINUTES(CONFIG_DFU_UPLOAD_COMPLETE_TIMEOUT));

	if (err && err != -EALREADY) {
		LOG_ERR("Bluetooth enable failed (err %d)", err);
		return err;
	}

	err = bt_le_adv_start(BT_LE_ADV_CONN_NAME, ad, ARRAY_SIZE(ad), NULL, 0);
	if (err) {
		LOG_ERR("Bluetooth advertising start failed (err %d)", err);
		return err;
	}

	LOG_INF("Advertising successfully started");

	dk_leds_init();
	LED_pattern_id =
		play_toggle_pattern(false, DFU_LED_controll, NULL, DFU_mode_pattern, LED_PATTERN_LENGTH(
					    DFU_mode_pattern),
				    true);
	return 0;
}
