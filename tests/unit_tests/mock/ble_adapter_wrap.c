/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <mock/ble_adapter_wrap.h>
#include <zephyr/settings/settings.h>
#include <sid_ble_adapter_callbacks.h>

int wrap_settings_load_call_count;
int wrap_sid_ble_conn_init_call_count;
int wrap_sid_ble_advert_init_call_count;
int wrap_sid_ble_send_data_call_count;

static int wrap_sid_ble_advert_deinit_ret;
static int wrap_sid_ble_advert_update_ret;
static int wrap_sid_ble_advert_start_ret;
static int wrap_sid_ble_advert_stop_ret;
static int wrap_sid_ble_conn_disconnect_ret;
static int wrap_settings_load_ret;
static const sid_ble_conn_data_t *wrap_conn_data;

void ble_adapter_wrap_reset(void)
{
	wrap_settings_load_call_count = 0;
	wrap_sid_ble_conn_init_call_count = 0;
	wrap_sid_ble_advert_init_call_count = 0;
	wrap_sid_ble_send_data_call_count = 0;
	wrap_settings_load_ret = 0;
	wrap_sid_ble_advert_deinit_ret = 0;
	wrap_sid_ble_advert_update_ret = 0;
	wrap_sid_ble_advert_start_ret = 0;
	wrap_sid_ble_advert_stop_ret = 0;
	wrap_sid_ble_conn_disconnect_ret = 0;
	wrap_conn_data = NULL;
}

int __wrap_settings_load(void)
{
	wrap_settings_load_call_count++;
	return wrap_settings_load_ret;
}

void __wrap_sid_ble_conn_init(void)
{
	wrap_sid_ble_conn_init_call_count++;
}

void __wrap_sid_ble_conn_deinit(void)
{
}

const sid_ble_conn_data_t *__wrap_sid_ble_conn_data_get(void)
{
	return wrap_conn_data;
}

int __wrap_sid_ble_conn_disconnect(void)
{
	return wrap_sid_ble_conn_disconnect_ret;
}

int __wrap_sid_ble_advert_init(void)
{
	wrap_sid_ble_advert_init_call_count++;
	return 0;
}

int __wrap_sid_ble_advert_deinit(void)
{
	return wrap_sid_ble_advert_deinit_ret;
}

int __wrap_sid_ble_advert_update(uint8_t *data, uint8_t len)
{
	ARG_UNUSED(data);
	ARG_UNUSED(len);
	return wrap_sid_ble_advert_update_ret;
}

int __wrap_sid_ble_advert_start(void)
{
	return wrap_sid_ble_advert_start_ret;
}

int __wrap_sid_ble_advert_stop(void)
{
	return wrap_sid_ble_advert_stop_ret;
}

int __wrap_sid_ble_send_data(sid_ble_srv_params_t *params, uint8_t *data, uint16_t len)
{
	ARG_UNUSED(params);
	ARG_UNUSED(data);
	ARG_UNUSED(len);
	wrap_sid_ble_send_data_call_count++;
	return 0;
}
