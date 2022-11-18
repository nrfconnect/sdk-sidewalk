/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */
#include <sidewalk_thread.h>

#include <zephyr.h>
#include <dk_buttons_and_leds.h>
#include <logging/log.h>

#if CONFIG_BOOTLOADER_MCUBOOT
#include <zephyr/dfu/mcuboot.h>
#endif

#include <sidewalk_version.h>

#if CONFIG_USB_DFU_ENABLE_UPLOAD
#include <zephyr/usb/usb_device.h>
#endif

LOG_MODULE_REGISTER(sid_template, CONFIG_SIDEWALK_LOG_LEVEL);

#define IS_RESET_BTN_PRESSED(_btn)      (_btn & DK_BTN1_MSK)
#if !defined(CONFIG_SIDEWALK_LINK_MASK_FSK) && !defined(CONFIG_SIDEWALK_LINK_MASK_LORA)
#define IS_CONN_REQ_BTN_PRESSED(_btn)   (_btn & DK_BTN2_MSK)
#else /* !defined(CONFIG_SIDEWALK_LINK_MASK_FSK) && !defined(CONFIG_SIDEWALK_LINK_MASK_LORA) */
#define IS_SET_DEV_PROFILE_BTN_PRESSED(_btn)   (_btn & DK_BTN2_MSK)
#endif /* !defined(CONFIG_SIDEWALK_LINK_MASK_FSK) && !defined(CONFIG_SIDEWALK_LINK_MASK_LORA) */
#define IS_SEND_MSG_BTN_PRESSED(_btn)   (_btn & DK_BTN3_MSK)
#define IS_SET_BAT_LV_BTN_PRESSED(_btn) (_btn & DK_BTN4_MSK)

#ifndef CONFIG_SIDEWALK_CLI
static
#endif
void sidewalk_button_pressed(uint32_t button_bit)
{
	LOG_INF("Pressed button %d", button_bit + 1);
	switch (button_bit) {
	case DK_BTN1: sidewalk_thread_message_q_write(EVENT_TYPE_FACTORY_RESET); break;
#if !defined(CONFIG_SIDEWALK_LINK_MASK_FSK) && !defined(CONFIG_SIDEWALK_LINK_MASK_LORA)
	case DK_BTN2: sidewalk_thread_message_q_write(EVENT_TYPE_CONNECTION_REQUEST); break;
#else
	case DK_BTN2: sidewalk_thread_message_q_write(EVENT_TYPE_SET_DEVICE_PROFILE); break;
#endif
	case DK_BTN3: sidewalk_thread_message_q_write(EVENT_TYPE_SEND_HELLO); break;
	case DK_BTN4: sidewalk_thread_message_q_write(EVENT_TYPE_SET_BATTERY_LEVEL); break;
	default:
		LOG_ERR("UNKNOWN BUTTON PRESSED");
		return;
	}
}

static void button_handler(uint32_t button_state, uint32_t has_changed)
{
	uint32_t button = button_state & has_changed;

	for (int button_bit = DK_BTN1; button_bit <= DK_BTN4; button_bit++) {
		if (button & BIT(button_bit)) {
			sidewalk_button_pressed(button_bit);
		}
	}
}

void assert_post_action(const char *file, unsigned int line)
{
	LOG_ERR("Assert in %s:%d", file, line);

	k_panic();
}

static int board_init(void)
{
	int err = dk_buttons_init(button_handler);

	if (err) {
		LOG_ERR("Failed to initialize buttons (err: %d)", err);
		return err;
	}

	err = dk_leds_init();
	if (err) {
		LOG_ERR("Failed to initialize LEDs (err: %d)", err);
		return err;
	}

#if CONFIG_USB_DFU_ENABLE_UPLOAD
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
	if (0 != board_init()) {
		return;
	}

	LOG_INF("Sidewalk example started!");

	PRINT_SIDEWALK_VERSION(Z_LOG_PRINTK);

	sidewalk_thread_enable();

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
}
