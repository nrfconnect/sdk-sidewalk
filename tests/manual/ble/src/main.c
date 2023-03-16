/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <sid_pal_ble_adapter_ifc.h>

#include <dk_buttons_and_leds.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(ble_test, LOG_LEVEL_DBG);

#define BTN_ADV DK_BTN1_MSK
#define BTN_ADV_DATA DK_BTN2_MSK
#define BTN_SEND DK_BTN3_MSK
#define BTN_INIT DK_BTN4_MSK
#define LED_CONN DK_LED1_MSK

#define BTN_LONG_PRESS (3UL * 1000UL) // miliseconds

#define DATA_SIZE 7

static uint8_t data[DATA_SIZE] = { 0x01, 0x02, 0xde, 0xad, 0xbe, 0xef, 0x00 };
static uint8_t data_oversize[64];

static sid_pal_ble_adapter_interface_t p_ble_ifc;
static const sid_ble_config_t ble_cfg;

static volatile uint64_t btn_press_time;

void app_button_handler(uint32_t button_state, uint32_t has_changed)
{
	sid_error_t ret = SID_ERROR_NONE;

	if (button_state & has_changed) {
		LOG_DBG("btn pressed %d", has_changed);
		switch (has_changed) {
		case BTN_ADV:
			ret = p_ble_ifc->stop_adv();
			LOG_INF("adv stop status %d", ret);
			break;
		case BTN_ADV_DATA:
			btn_press_time = k_uptime_get_32();
			break;
		case BTN_SEND:
			btn_press_time = k_uptime_get_32();
			break;
		case BTN_INIT:
			ret = p_ble_ifc->deinit();
			LOG_INF("deinit status %d", ret);
			break;
		default:
			LOG_DBG("no action");
			break;
		}
	} else {
		LOG_DBG("button released %d", has_changed);
		uint64_t delta_time = 0;
		switch (has_changed) {
		case BTN_ADV:
			ret = p_ble_ifc->start_adv();
			LOG_INF("adv start status %d", ret);
			break;
		case BTN_ADV_DATA:
			delta_time = k_uptime_get() - btn_press_time;
			if (delta_time < BTN_LONG_PRESS) {
				data[DATA_SIZE - 1]++;
				ret = p_ble_ifc->set_adv_data(data, sizeof(data));
				LOG_HEXDUMP_DBG(data, sizeof(data), "set adv data");
				LOG_INF("set adv status %d", ret);
			} else {
				memset(data_oversize, 0xF0, sizeof(data_oversize));
				ret = p_ble_ifc->set_adv_data(data_oversize, sizeof(data_oversize));
				LOG_HEXDUMP_DBG(data_oversize, sizeof(data_oversize), "set adv data: ");
				LOG_INF("set adv status %d", ret);
			}
			break;
		case BTN_SEND:
			delta_time = k_uptime_get() - btn_press_time;
			if (delta_time < BTN_LONG_PRESS) {
				data[DATA_SIZE - 1]++;
				ret = p_ble_ifc->send(AMA_SERVICE, data, sizeof(data));
				LOG_HEXDUMP_DBG(data, sizeof(data), "send notify data");
				LOG_INF("send status %d", ret);
			} else {
				ret = p_ble_ifc->disconnect();
				LOG_INF("disconnect status %d", ret);
			}
			break;
		case BTN_INIT:
			ret = p_ble_ifc->init(&ble_cfg);
			LOG_INF("init status %d", ret);
			break;
		default:
			LOG_DBG("no action");
			break;
		}
	}
}

static void app_data_callback(sid_ble_cfg_service_identifier_t id, uint8_t *data, uint16_t length)
{
	LOG_DBG("%s service id: %d", __func__, id);
	LOG_HEXDUMP_DBG(data, length, "data");
}

static void app_notify_callback(sid_ble_cfg_service_identifier_t id, bool state)
{
	LOG_DBG("%s service id: %d", __func__, id);
	LOG_DBG("state %s", state ? "true" : "false");
}

static void app_connection_callback(bool state, uint8_t *addr)
{
	LOG_DBG("%s", __func__);
	LOG_DBG("state %s", state ? "true" : "false");
	LOG_DBG("addr %02x:%02x:%02x:%02x:%02x:%02x", addr[5], addr[4], addr[3], addr[2], addr[1], addr[0]);

	dk_set_led(LED_CONN, state ? 1U : 0U);
}

static void app_indication_callback(bool status)
{
	LOG_DBG("%s", __func__);
	LOG_DBG("status %s", status ? "true" : "false");
}

static void app_mtu_callback(uint16_t size)
{
	LOG_DBG("%s size %d", __func__, size);
}

static void app_adv_start_callback(void)
{
	LOG_DBG("%s", __func__);
}

void main(void)
{
	LOG_INF("> Test Bluetooth");
	sid_pal_ble_adapter_create(&p_ble_ifc);

	sid_pal_ble_adapter_callbacks_t ble_cbs = {
		.data_callback = app_data_callback,
		.notify_callback = app_notify_callback,
		.conn_callback = app_connection_callback,
		.ind_callback = app_indication_callback,
		.mtu_callback = app_mtu_callback,
		.adv_start_callback = app_adv_start_callback,
	};

	p_ble_ifc->init(&ble_cfg);
	p_ble_ifc->start_adv();
	p_ble_ifc->set_callback(&ble_cbs);
	LOG_INF("Bluetooth name %s", CONFIG_BT_DEVICE_NAME);
	LOG_INF("Advertising Started");

	dk_leds_init();
	int err = dk_buttons_init(app_button_handler);

	if (err) {
		LOG_ERR("button init error %d", err);
	}

	for (;;) {
		k_sleep(K_MSEC(500));
	}
}
