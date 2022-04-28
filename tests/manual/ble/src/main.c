/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#include <sid_pal_ble_adapter_ifc.h>

#include <zephyr.h>
#include <logging/log.h>

LOG_MODULE_REGISTER(ble_test, LOG_LEVEL_DBG);

static sid_pal_ble_adapter_interface_t p_ble_ifc;
static const sid_ble_config_t ble_cfg;

static uint8_t adv_data[] = { 0x01, 0x02, 0xde, 0xad, 0xbe, 0xef, 0x00 };
static uint8_t adv_data_oversize[64];

void my_work_handler(struct k_work *work)
{
	uint8_t adv_data_lb = sizeof(adv_data) - 1;

	adv_data[adv_data_lb]++;
	LOG_HEXDUMP_DBG(adv_data, sizeof(adv_data), "set adc data: ");
	p_ble_ifc->set_adv_data(adv_data, sizeof(adv_data));
}

K_WORK_DEFINE(my_work, my_work_handler);

void my_timer_handler(struct k_timer *dummy)
{
	k_work_submit(&my_work);
}

K_TIMER_DEFINE(my_timer, my_timer_handler, NULL);

void main(void)
{
	LOG_INF("> Test Bluetooth");
	sid_pal_ble_adapter_create(&p_ble_ifc);

	p_ble_ifc->init(&ble_cfg);
	p_ble_ifc->start_adv();
	LOG_INF("Bluetooth name %s", CONFIG_BT_DEVICE_NAME);
	LOG_INF("Advertising Started");

	k_sleep(K_SECONDS(5));
	p_ble_ifc->stop_adv();
	LOG_INF("Advertising Stopped");

	k_sleep(K_SECONDS(5));
	p_ble_ifc->set_adv_data(adv_data, sizeof(adv_data));
	LOG_HEXDUMP_DBG(adv_data, sizeof(adv_data), "set adc data: ");
	p_ble_ifc->start_adv();
	LOG_INF("Advertising Started");
	memset(adv_data_oversize, 0xF0, sizeof(adv_data_oversize));
	p_ble_ifc->set_adv_data(adv_data_oversize, sizeof(adv_data_oversize));
	LOG_HEXDUMP_DBG(adv_data_oversize, sizeof(adv_data_oversize), "set adc data: ");

	/* start periodic timer that expires once every second */
	k_timer_start(&my_timer, K_SECONDS(5), K_SECONDS(1));
}
