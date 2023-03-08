/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#include <zephyr/logging/log.h>

#include <sidewalk_version.h>

#include <sid_error.h>
#include <sid_thread.h>
#include <sid_api_delegated.h>
#include <board_init.h>
#include <dk_buttons_and_leds.h>
#include <buttons.h>
#if defined(CONFIG_SIDEWALK_SUBGHZ)
#include <sid_900_cfg.h>
#endif

#include <zephyr/kernel.h>
#if CONFIG_STATE_NOTIFIER
#include <state_notifier.h>
#endif
#if CONFIG_SIDEWALK_DFU_SERVICE_BLE
#include <nordic_dfu.h> 
#endif




extern struct notifier_ctx global_state_notifier;
LOG_MODULE_REGISTER(sid_main, LOG_LEVEL_INF);

struct app_ctx {
	struct k_work_q *workq;
	struct k_work factory_reset;
        struct k_work set_battery_level;
	struct k_work get_device_profile;
	struct k_work set_device_profile;
	struct k_work send_hello;
	struct k_work enter_dfu;

        struct sid_handle ** handle;

#if CONFIG_SIDEWALK_DFU_SERVICE_BLE
        bool dfu_state;
#endif
};
static struct app_ctx ctx;

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

static void get_device_profile_worker(struct k_work* work)
{
        static struct sid_device_profile profile_from_dev = {
#if defined(CONFIG_SIDEWALK_LINK_MASK_FSK)
	.unicast_params.device_profile_id = SID_LINK2_PROFILE_1
#elif defined(CONFIG_SIDEWALK_LINK_MASK_LORA)
        .unicast_params.device_profile_id = SID_LINK3_PROFILE_A
#endif
};
        struct app_ctx *ctx = CONTAINER_OF(work, struct app_ctx, get_device_profile);
        sid_error_t ret = sid_option(*ctx->handle, SID_OPTION_900MHZ_GET_DEVICE_PROFILE,
				     &profile_from_dev, sizeof(profile_from_dev));

	if (ret) {
		LOG_ERR("Profile get failed (err %d)", ret);
		return;
	}

	LOG_INF("Profile id 0x%x", profile_from_dev.unicast_params.device_profile_id);
	LOG_INF("Profile dl count %d", profile_from_dev.unicast_params.rx_window_count);
	LOG_INF("Profile dl interval %d", profile_from_dev.unicast_params.unicast_window_interval.async_rx_interval_ms);
	LOG_INF("Profile wakeup %d", profile_from_dev.unicast_params.wakeup_type);
}


static void set_device_profile_worker(struct k_work* work)
{
#if defined(CONFIG_SIDEWALK_LINK_MASK_FSK)
        static struct sid_device_profile profile_light = {
                .unicast_params.device_profile_id = SID_LINK2_PROFILE_1,
                .unicast_params.rx_window_count = SID_RX_WINDOW_CNT_INFINITE,
                .unicast_params.unicast_window_interval.sync_rx_interval_ms = SID_LINK2_RX_WINDOW_SEPARATION_3,
                .unicast_params.wakeup_type = SID_TX_AND_RX_WAKEUP,
        };

        static struct sid_device_profile profile_fast = {
                .unicast_params.device_profile_id = SID_LINK2_PROFILE_2,
                .unicast_params.rx_window_count = SID_RX_WINDOW_CNT_INFINITE,
                .unicast_params.unicast_window_interval.sync_rx_interval_ms = SID_LINK2_RX_WINDOW_SEPARATION_3,
                .unicast_params.wakeup_type = SID_TX_AND_RX_WAKEUP,
        };
#elif defined(CONFIG_SIDEWALK_LINK_MASK_LORA)

        static struct sid_device_profile profile_light = {
                .unicast_params.device_profile_id = SID_LINK3_PROFILE_A,
                .unicast_params.rx_window_count =  SID_RX_WINDOW_CNT_2,
                .unicast_params.unicast_window_interval.async_rx_interval_ms = SID_LINK3_RX_WINDOW_SEPARATION_3,
                .unicast_params.wakeup_type = SID_TX_AND_RX_WAKEUP,
        };

        static struct sid_device_profile profile_fast = {
                .unicast_params.device_profile_id = SID_LINK3_PROFILE_B,
                .unicast_params.rx_window_count = SID_RX_WINDOW_CNT_INFINITE,
                .unicast_params.unicast_window_interval.async_rx_interval_ms = SID_LINK3_RX_WINDOW_SEPARATION_3,
                .unicast_params.wakeup_type = SID_TX_AND_RX_WAKEUP,
        };

#endif


        struct app_ctx *ctx = CONTAINER_OF(work, struct app_ctx, set_device_profile);
        static struct sid_device_profile *new_profile = &profile_light;

	LOG_INF("Profile set %s", (&profile_light == new_profile) ? "light" : "fast");

	sid_error_t ret = sid_option(*ctx->handle, SID_OPTION_900MHZ_SET_DEVICE_PROFILE, new_profile,
				     sizeof(*new_profile));

	if (!ret) {
		new_profile = (&profile_light == new_profile) ? &profile_fast : &profile_light;
		LOG_INF("Profile set success.");
	} else {
		LOG_ERR("Profile set failed (err %d)", ret);
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
#if CONFIG_SIDEWALK_DFU_SERVICE_BLE
static void enter_dfu_worker(struct k_work* work)
{
        struct app_ctx *ctx = CONTAINER_OF(work, struct app_ctx, enter_dfu);

        if (!ctx->dfu_state) {
		sid_error_t ret = SID_ERROR_NONE;

                LOG_INF("Deinitialize Sidewalk");
                ret = sid_deinit(*ctx->handle);
                if (ret) {
                        LOG_ERR("Sidewalk deinit error (code %d)", ret);
                }

		int ret2 = nordic_dfu_ble_start();
		if (ret2) {
			LOG_ERR("DFU SMP start error (code %d)", ret2);
		}

		ctx->dfu_state = true;
		application_state_dfu(&global_state_notifier, true);
	}
}
#endif
static void init_event_workers()
{
	k_work_init(&ctx.factory_reset, factory_reset_worker);
	k_work_init(&ctx.set_battery_level, set_battery_level_worker);
	k_work_init(&ctx.get_device_profile, get_device_profile_worker);
	k_work_init(&ctx.set_device_profile, set_device_profile_worker);
	k_work_init(&ctx.send_hello, send_hello_worker);
	#if CONFIG_SIDEWALK_DFU_SERVICE_BLE
	k_work_init(&ctx.enter_dfu, enter_dfu_worker);
#endif
}

static int init_button_actions()
{
	button_set_action_long_press(DK_BTN1, submit_to_work, (uint32_t)&ctx.factory_reset);
	button_set_action_short_press(DK_BTN2, submit_to_work, (uint32_t)&ctx.get_device_profile);
	button_set_action_long_press(DK_BTN2, submit_to_work, (uint32_t)&ctx.set_device_profile);
	button_set_action(DK_BTN3, submit_to_work, (uint32_t)&ctx.send_hello);
	#if CONFIG_SIDEWALK_DFU_SERVICE_BLE
	button_set_action_long_press(DK_BTN4, submit_to_work, (uint32_t)&ctx.enter_dfu);
#endif
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
	return 0;
}

void main(void)
{
	PRINT_SIDEWALK_VERSION();

	if (sidewalk_board_init() != SID_ERROR_NONE) {
		return;
	}

	if (application_board_init() != 0) {
		return;
	}

	ctx.workq = sid_thread_init();
	sid_api_delegated_init(ctx.workq);
        ctx.handle = get_sidewalk_handle();

        sid_init_delegated(get_sidewalk_config(), ctx.handle);

        sid_start_delegated(*ctx.handle, SID_LINK_TYPE_1);

}
