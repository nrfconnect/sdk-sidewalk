/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */
#include "sid_api.h"
#include "sid_error.h"
#include <sid_thread.h>
#include <zephyr/kernel.h>
#include <dk_buttons_and_leds.h>
#include <buttons.h>
#include <zephyr/logging/log.h>

#include <sid_api_delegated.h>

#if CONFIG_BOOTLOADER_MCUBOOT
#include <zephyr/dfu/mcuboot.h>
#endif
#include <board_init.h>
#include <state_notifier.h>

#include <sidewalk_version.h>

#if CONFIG_SIDEWALK_DFU_SERVICE_USB
#include <zephyr/usb/usb_device.h>
#endif
#if CONFIG_SIDEWALK_DFU_SERVICE_BLE
#include <nordic_dfu.h> 
#endif

#include <zephyr/settings/settings.h>

LOG_MODULE_REGISTER(sid_template, CONFIG_SIDEWALK_LOG_LEVEL);

struct app_ctx {
	struct k_work_q *workq;
	struct k_work factory_reset;
        struct k_work set_battery_level;
	struct k_work conneciton_request;
	struct k_work send_hello;
	struct k_work enter_dfu;
        bool conn_request_value;

        struct sid_handle ** handle;

};
static struct app_ctx ctx;

bool factory_reset_bypass = false;

int alpha_handle_set(const char *name, size_t len, settings_read_cb read_cb, void *cb_arg);
int alpha_handle_export(int (*cb)(const char *name, const void *value, size_t val_len));

struct settings_handler alph_handler = {
	.name = "test",
	.h_get = NULL,
	.h_set = alpha_handle_set,
	.h_commit = NULL,
	.h_export = alpha_handle_export
};

K_SEM_DEFINE(registered_sem, 0, 1);
K_SEM_DEFINE(not_sending_sem, 0, 1);
K_SEM_DEFINE(working_sem, 0, 1);
K_SEM_DEFINE(connected_sem, 0, 1);
K_SEM_DEFINE(time_sync_sem, 0, 1);
K_SEM_DEFINE(link_sem, 0, 1);

volatile struct notifier_state current_app_state = {};

int alpha_handle_set(const char *name, size_t len, settings_read_cb read_cb,
		     void *cb_arg)
{
	const char *next;

	if (settings_name_steq(name, "factory_reset_bypass", &next)) {
		if (len != sizeof(factory_reset_bypass)) {
			return -EINVAL;
		}
		read_cb(cb_arg, &factory_reset_bypass, sizeof(factory_reset_bypass));
		return 0;
	}
	return -ENOENT;
}

int alpha_handle_export(int (*cb)(const char *name,
				  const void *value, size_t val_len))
{
	(void)cb("test/factory_reset_bypass", &factory_reset_bypass, sizeof(factory_reset_bypass));
	return 0;
}

void assert_post_action(const char *file, unsigned int line)
{
	LOG_ERR("Assert in %s:%d", file, line);

	k_panic();
}



static void submit_to_work(uint32_t work)
{
	struct k_work *work_ptr = (struct k_work *) work;

	if (ctx.workq == NULL) {
                LOG_ERR("Work queue has not been initialized!");
		return;
	}
	int r = k_work_submit_to_queue(ctx.workq, work_ptr);

	if (r) {
		LOG_ERR("Failed to schedule task with error %d", r);
	}
}

static void factory_reset_worker(struct k_work* work){
        struct app_ctx *ctx = CONTAINER_OF(work, struct app_ctx, factory_reset);
        sid_error_t ret = sid_set_factory_reset(*ctx->handle);

	if (SID_ERROR_NONE != ret) {
		LOG_ERR("Notification of factory reset to sid api failed!");
	} else {
		LOG_INF("Wait for Sid api to notify to proceed with factory reset!");
	}
}

static void connection_request_worker(struct k_work* work)
{
        struct app_ctx *ctx = CONTAINER_OF(work, struct app_ctx, conneciton_request);
        struct sid_status status;
        sid_get_status(*ctx->handle, &status);

        if (SID_STATE_READY == status.state) {
		LOG_WRN("Sidewalk ready, operation not valid");
	} else {
		bool next = !ctx->conn_request_value;
		LOG_INF("%s connection request", next ? "Set" : "Clear");
		sid_error_t ret = sid_ble_bcn_connection_request(*ctx->handle, next);
		if (SID_ERROR_NONE == ret) {
			ctx->conn_request_value = next;
		} else {
			LOG_ERR("Connection request failed %d", ret);
		}
	}
}

static void set_battery_level_worker(struct k_work* work){
        struct app_ctx *ctx = CONTAINER_OF(work, struct app_ctx, set_battery_level);
        static uint8_t fake_bat_lev = 70;

	++fake_bat_lev;
	if (fake_bat_lev > 100) {
		fake_bat_lev = 0;
	}
	sid_error_t ret = sid_option(*ctx->handle, SID_OPTION_BLE_BATTERY_LEVEL,
				     &fake_bat_lev, sizeof(fake_bat_lev));

	if (SID_ERROR_NONE != ret) {
		LOG_ERR("failed setting Sidewalk option!");
	} else {
		LOG_DBG("set battery level to %d", fake_bat_lev);
	}
}

static void send_hello_worker(struct k_work* work){
        struct app_ctx *ctx = CONTAINER_OF(work, struct app_ctx, send_hello);

        struct sid_status status;
        sid_get_status(*ctx->handle, &status);

        static uint8_t counter = 0; 
        static struct sid_msg msg;
        memset(&msg, 0, sizeof(msg));
        static struct sid_msg_desc desc;
        memset(&desc, 0, sizeof(desc));

        if (status.state == SID_STATE_READY || status.state == SID_STATE_SECURE_CHANNEL_READY) {
		LOG_INF("sending counter update: %d", counter);
		
                msg = (struct sid_msg){ .data = (uint8_t *)&counter, .size = sizeof(uint8_t) };
                desc = (struct sid_msg_desc){
			.type = SID_MSG_TYPE_NOTIFY,
			.link_type = SID_LINK_TYPE_ANY,
			.link_mode = SID_LINK_MODE_CLOUD,
		};
                if ((status.detail.link_status_mask & SID_LINK_TYPE_1) && (status.detail.supported_link_modes[SID_LINK_TYPE_1_IDX] & SID_LINK_MODE_MOBILE))
		{
			desc.link_mode = SID_LINK_MODE_MOBILE;
		}
		application_state_sending(&global_state_notifier, true);
		sid_error_t ret = sid_put_msg(*ctx->handle, &msg, &desc);
		if (SID_ERROR_NONE != ret) {
			LOG_ERR("failed queueing data, err:%d", (int) ret);
		} else {
			LOG_INF("queued data message id:%u", desc.id);
			counter++;
		}
	} else {
		LOG_ERR("Sidewalk is not ready yet!");
	}
}

static void init_event_workers()
{
	k_work_init(&ctx.factory_reset, factory_reset_worker);
	k_work_init(&ctx.set_battery_level, set_battery_level_worker);
	k_work_init(&ctx.conneciton_request, connection_request_worker);
	k_work_init(&ctx.send_hello, send_hello_worker);
}

static int init_button_actions()
{
	button_set_action_long_press(DK_BTN1, submit_to_work, (uint32_t)&ctx.factory_reset);
	button_set_action(DK_BTN2, submit_to_work, (uint32_t)&ctx.conneciton_request);
	button_set_action_short_press(DK_BTN4, submit_to_work, (uint32_t)&ctx.set_battery_level);
	button_set_action(DK_BTN3, submit_to_work, (uint32_t)&ctx.send_hello);
	return buttons_init();
}

static int application_board_init()
{
	init_event_workers();
	int err = init_button_actions();

	if (err) {
		LOG_ERR("Failed to initialize buttons (err: %d)", err);
		return err;
	}

	err = dk_leds_init();
	if (err) {
		LOG_ERR("Failed to initialize LEDs (err: %d)", err);
		return err;
	}

#if CONFIG_SIDEWALK_DFU_SERVICE_USB
	err = usb_enable(NULL);
	if (err != 0) {
		LOG_ERR("Failed to enable USB");
		return err;
	}
#endif
	#if CONFIG_BOOTLOADER_MCUBOOT
	if (!boot_is_img_confirmed()) {
		int ret = boot_write_img_confirmed();

		if (ret) {
			LOG_ERR("Couldn't confirm image: %d", ret);
		} else {
			LOG_INF("Marked image as OK");
		}
	}
	#endif

	return 0;
}

static void state_change_handler_power_test(const struct notifier_state *state)
{
	current_app_state = *state;
	if (current_app_state.registered) {
		k_sem_give(&registered_sem);
	}
	if (current_app_state.sending == 0) {
		k_sem_give(&not_sending_sem);
	}
	if (current_app_state.connected) {
		k_sem_give(&connected_sem);
	}
	if (current_app_state.time_sync) {
		k_sem_give(&time_sync_sem);
	}
	if (current_app_state.working) {
		k_sem_give(&working_sem);
	}
	if (current_app_state.link) {
		k_sem_give(&link_sem);
	}

}

void wait_for_registered()
{
	LOG_INF("%s", __func__);
	if (!current_app_state.registered) {
		k_sem_take(&registered_sem, K_FOREVER);
	}
}

void wait_for_not_sending()
{
	LOG_INF("%s", __func__);
	if (current_app_state.sending) {
		k_sem_take(&not_sending_sem, K_FOREVER);
	}
}

void wait_for_connected()
{
	LOG_INF("%s", __func__);
	if (!current_app_state.connected) {
		k_sem_take(&connected_sem, K_FOREVER);
	}
}

void wait_for_time_sync()
{
	LOG_INF("%s", __func__);
	if (!current_app_state.time_sync) {
		k_sem_take(&time_sync_sem, K_FOREVER);
	}
}

void wait_for_link()
{
	LOG_INF("%s", __func__);
	if (!current_app_state.link) {
		k_sem_take(&link_sem, K_FOREVER);
	}
}

static inline void perform_power_test(void)
{
	LOG_INF("%s", __func__);
	wait_for_registered();
	wait_for_time_sync();

	#if defined(CONFIG_SIDEWALK_LINK_MASK_FSK) || defined(CONFIG_SIDEWALK_LINK_MASK_LORA)
	k_work_submit_to_queue(ctx.workq, &ctx.send_hello);
	wait_for_time_sync();
	wait_for_connected();
	#endif

	for (int i = 0; i < CONFIG_MESSAGES_TO_SEND; i++) {

	#if !defined(CONFIG_SIDEWALK_LINK_MASK_FSK) && !defined(CONFIG_SIDEWALK_LINK_MASK_LORA)
		if (!current_app_state.connected) {
			wait_for_registered();
			wait_for_time_sync();
			k_work_submit_to_queue(ctx.workq, &ctx.conneciton_request);
			wait_for_connected();
		}
	#endif
		wait_for_connected();
		wait_for_not_sending();
		k_work_submit_to_queue(ctx.workq, &ctx.send_hello);
		k_sleep(K_MSEC(CONFIG_DELAY_BETWEN_MESSAGES));
	}
}

void main(void)
{
	PRINT_SIDEWALK_VERSION();

	if (!subscribe_for_state_change(&global_state_notifier, state_change_handler_power_test))
	{
		__ASSERT(false, "failed to initialize the state watch, is the CONFIG_STATE_NOTIFIER_HANDLER_MAX too low ?");
	}
	if (sidewalk_board_init() != SID_ERROR_NONE) {
		return;
	}
	if(application_board_init()){
		return;
	}

	ctx.workq = sid_thread_init();
	sid_api_delegated_init(ctx.workq);
        ctx.handle = get_sidewalk_handle();

	sid_init_delegated(get_sidewalk_config(), ctx.handle);

        sid_start_delegated(*ctx.handle, SID_LINK_TYPE_1);


	LOG_INF("Sidewalk example started!");


	if (!current_app_state.working) {
		k_sem_take(&working_sem, K_FOREVER);
	}

	settings_subsys_init();
	settings_register(&alph_handler);
	settings_load();

	if (factory_reset_bypass == false) {
		factory_reset_bypass = true;
		settings_save();
		settings_commit();

		LOG_INF("Perform factory reset");
		k_sleep(K_SECONDS(1));
		k_work_submit_to_queue(ctx.workq, &ctx.factory_reset);
		k_sleep(K_FOREVER);
	}
	factory_reset_bypass = false;
	settings_save();
	settings_commit();

	perform_power_test();
}
