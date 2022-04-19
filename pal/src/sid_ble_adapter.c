/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

/** @file sid_ble_adapter.c
 *  @brief Bluetooth low energy adapter implementation.
 */

#include <sid_pal_ble_adapter_ifc.h>

static sid_error_t ble_adapter_init(const sid_ble_config_t *cfg);
static sid_error_t ble_adapter_start_service(void);
static sid_error_t ble_adapter_set_adv_data(uint8_t *data, uint8_t length);
static sid_error_t ble_adapter_start_advertisement(void);
static sid_error_t ble_adapter_stop_advertisement(void);
static sid_error_t ble_adapter_send_data(sid_ble_cfg_service_identifier_t id, uint8_t *data, uint16_t length);
static sid_error_t ble_adapter_set_callback(const sid_pal_ble_adapter_callbacks_t *cb);
static sid_error_t ble_adapter_disconnect(void);
static sid_error_t ble_adapter_deinit(void);

static struct sid_pal_ble_adapter_interface ble_ifc = {
	.init = ble_adapter_init,
	.start_service = ble_adapter_start_service,
	.set_adv_data = ble_adapter_set_adv_data,
	.start_adv = ble_adapter_start_advertisement,
	.stop_adv = ble_adapter_stop_advertisement,
	.send = ble_adapter_send_data,
	.set_callback = ble_adapter_set_callback,
	.disconnect = ble_adapter_disconnect,
	.deinit = ble_adapter_deinit,
};

static sid_error_t ble_adapter_init(const sid_ble_config_t *cfg)
{
	return SID_ERROR_NOSUPPORT;
}
static sid_error_t ble_adapter_start_service(void)
{
	return SID_ERROR_NOSUPPORT;
}
static sid_error_t ble_adapter_set_adv_data(uint8_t *data, uint8_t length)
{
	return SID_ERROR_NOSUPPORT;
}
static sid_error_t ble_adapter_start_advertisement(void)
{
	return SID_ERROR_NOSUPPORT;
}
static sid_error_t ble_adapter_stop_advertisement(void)
{
	return SID_ERROR_NOSUPPORT;
}
static sid_error_t ble_adapter_send_data(sid_ble_cfg_service_identifier_t id, uint8_t *data, uint16_t length)
{
	return SID_ERROR_NOSUPPORT;
}
static sid_error_t ble_adapter_set_callback(const sid_pal_ble_adapter_callbacks_t *cb)
{
	return SID_ERROR_NOSUPPORT;
}
static sid_error_t ble_adapter_disconnect(void)
{
	return SID_ERROR_NOSUPPORT;
}
static sid_error_t ble_adapter_deinit(void)
{
	return SID_ERROR_NOSUPPORT;
}

sid_error_t sid_pal_ble_adapter_create(sid_pal_ble_adapter_interface_t *handle)
{
	if (!handle) {
		return SID_ERROR_INVALID_ARGS;
	}

	*handle = &ble_ifc;

	return SID_ERROR_NONE;
}
