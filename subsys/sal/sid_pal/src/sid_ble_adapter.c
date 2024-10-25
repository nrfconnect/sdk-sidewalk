/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/** @file sid_ble_adapter.c
 *  @brief Bluetooth low energy adapter implementation.
 */

#include <sid_error.h>
#include <sid_pal_ble_adapter_ifc.h>
#include <sid_ble_service.h>
#include <sid_ble_ama_service.h>
#if defined(CONFIG_SIDEWALK_VENDOR_SERVICE)
#include <sid_ble_vnd_service.h>
#endif /* CONFIG_SIDEWALK_VENDOR_SERVICE */
#if defined(CONFIG_SIDEWALK_LOGGING_SERVICE)
#include <sid_ble_log_service.h>
#endif /* CONFIG_SIDEWALK_LOGGING_SERVICE */
#include <sid_ble_adapter_callbacks.h>
#include <sid_ble_advert.h>
#include <sid_ble_connection.h>

#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/bluetooth.h>

#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/settings/settings.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/bluetooth/hci_vs.h>

LOG_MODULE_REGISTER(sid_ble, CONFIG_SIDEWALK_BLE_ADAPTER_LOG_LEVEL);

static sid_error_t ble_adapter_init(const sid_ble_config_t *cfg);
static sid_error_t ble_adapter_start_service(void);
static sid_error_t ble_adapter_set_adv_data(uint8_t *data, uint8_t length);
static sid_error_t ble_adapter_start_advertisement(void);
static sid_error_t ble_adapter_stop_advertisement(void);
static sid_error_t ble_adapter_send_data(sid_ble_cfg_service_identifier_t id, uint8_t *data,
					 uint16_t length);
static sid_error_t ble_adapter_set_callback(const sid_pal_ble_adapter_callbacks_t *cb);
static sid_error_t ble_adapter_disconnect(void);
static sid_error_t ble_adapter_deinit(void);
static sid_error_t ble_adapter_get_rssi(int8_t *rssi);
static sid_error_t ble_adapter_get_tx_pwr(int8_t *tx_power);
static sid_error_t ble_adapter_set_tx_pwr(int8_t tx_power);

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
	.get_rssi = ble_adapter_get_rssi,
	.get_tx_pwr = ble_adapter_get_tx_pwr,
	.set_tx_pwr = ble_adapter_set_tx_pwr,

};

static void read_conn_rssi(uint16_t handle, int8_t *rssi)
{
	struct net_buf *buf, *rsp = NULL;
	struct bt_hci_cp_read_rssi *cp;
	struct bt_hci_rp_read_rssi *rp;

	int err;

	buf = bt_hci_cmd_create(BT_HCI_OP_READ_RSSI, sizeof(*cp));
	if (!buf) {
		LOG_ERR("Unable to allocate command buffer\n");
		return;
	}

	cp = net_buf_add(buf, sizeof(*cp));
	cp->handle = sys_cpu_to_le16(handle);

	err = bt_hci_cmd_send_sync(BT_HCI_OP_READ_RSSI, buf, &rsp);
	if (err) {
		uint8_t reason = rsp ? ((struct bt_hci_rp_read_rssi *)rsp->data)->status : 0;
		LOG_ERR("Read RSSI err: %d reason 0x%02x\n", err, reason);
		return;
	}

	rp = (void *)rsp->data;
	*rssi = rp->rssi;

	net_buf_unref(rsp);
}
static void set_tx_power(uint8_t handle_type, uint16_t handle, int8_t tx_pwr_lvl)
{
	struct bt_hci_cp_vs_write_tx_power_level *cp;
	struct bt_hci_rp_vs_write_tx_power_level *rp;
	struct net_buf *buf, *rsp = NULL;
	int err;

	buf = bt_hci_cmd_create(BT_HCI_OP_VS_WRITE_TX_POWER_LEVEL, sizeof(*cp));
	if (!buf) {
		LOG_ERR("Unable to allocate command buffer\n");
		return;
	}

	cp = net_buf_add(buf, sizeof(*cp));
	cp->handle = sys_cpu_to_le16(handle);
	cp->handle_type = handle_type;
	cp->tx_power_level = tx_pwr_lvl;

	err = bt_hci_cmd_send_sync(BT_HCI_OP_VS_WRITE_TX_POWER_LEVEL, buf, &rsp);
	if (err) {
		uint8_t reason =
			rsp ? ((struct bt_hci_rp_vs_write_tx_power_level *)rsp->data)->status : 0;
		LOG_ERR("Set Tx power err: %d reason 0x%02x\n", err, reason);
		return;
	}

	rp = (void *)rsp->data;
	LOG_ERR("Actual Tx Power: %d\n", rp->selected_tx_power);

	net_buf_unref(rsp);
}

static void get_tx_power(uint8_t handle_type, uint16_t handle, int8_t *tx_pwr_lvl)
{
	struct bt_hci_cp_vs_read_tx_power_level *cp;
	struct bt_hci_rp_vs_read_tx_power_level *rp;
	struct net_buf *buf, *rsp = NULL;
	int err;

	*tx_pwr_lvl = 0xFF;
	buf = bt_hci_cmd_create(BT_HCI_OP_VS_READ_TX_POWER_LEVEL, sizeof(*cp));
	if (!buf) {
		LOG_ERR("Unable to allocate command buffer\n");
		return;
	}

	cp = net_buf_add(buf, sizeof(*cp));
	cp->handle = sys_cpu_to_le16(handle);
	cp->handle_type = handle_type;

	err = bt_hci_cmd_send_sync(BT_HCI_OP_VS_READ_TX_POWER_LEVEL, buf, &rsp);
	if (err) {
		uint8_t reason =
			rsp ? ((struct bt_hci_rp_vs_read_tx_power_level *)rsp->data)->status : 0;
		LOG_ERR("Read Tx power err: %d reason 0x%02x\n", err, reason);
		return;
	}

	rp = (void *)rsp->data;
	*tx_pwr_lvl = rp->tx_power_level;

	net_buf_unref(rsp);
}

static sid_error_t ble_adapter_get_rssi(int8_t *rssi)
{
	const sid_ble_conn_params_t *params = sid_ble_conn_params_get();
	if (params == NULL || params->conn == NULL) {
		return SID_ERROR_GENERIC;
	}

	uint16_t conn_handle = 0;
	int e = bt_hci_get_conn_handle(params->conn, &conn_handle);
	if (e != 0) {
		LOG_ERR("Can not get conn_handle error %d", e);
		return SID_ERROR_GENERIC;
	}
	read_conn_rssi(conn_handle, rssi);

	LOG_DBG("BLE RSSI = %d", *rssi);
	return SID_ERROR_NONE;
}

static sid_error_t ble_adapter_get_tx_pwr(int8_t *tx_power)
{
	uint16_t conn_handle = 0;

	const sid_ble_conn_params_t *params = sid_ble_conn_params_get();
	if (params == NULL || params->conn == NULL) {
		return SID_ERROR_GENERIC;
	}

	int e = bt_hci_get_conn_handle(params->conn, &conn_handle);
	if (e != 0) {
		LOG_ERR("Can not get conn_handle error %d", e);
		return SID_ERROR_GENERIC;
	}
	get_tx_power(BT_HCI_VS_LL_HANDLE_TYPE_CONN, conn_handle, tx_power);
	LOG_DBG("BLE get tx pwr: %d", *tx_power);
	return SID_ERROR_NONE;
}

static sid_error_t ble_adapter_set_tx_pwr(int8_t tx_power)
{
	uint16_t conn_handle = 0;

	const sid_ble_conn_params_t *params = sid_ble_conn_params_get();
	if (params == NULL || params->conn == NULL) {
		return SID_ERROR_GENERIC;
	}
	int e = bt_hci_get_conn_handle(params->conn, &conn_handle);
	if (e != 0) {
		LOG_ERR("Can not get conn_handle error %d", e);
		return SID_ERROR_GENERIC;
	}
	set_tx_power(BT_HCI_VS_LL_HANDLE_TYPE_CONN, conn_handle, tx_power);
	LOG_DBG("BLE set tx pwr: %d", tx_power);
	return SID_ERROR_NONE;
}

static sid_error_t ble_adapter_init(const sid_ble_config_t *cfg)
{
	LOG_DBG("Sidewalk -> BLE");
	ARG_UNUSED(cfg);

	LOG_INF("Enable BT");
	int err_code;
	err_code = bt_enable(NULL);
	switch (err_code) {
	case -EALREADY:
	case 0:
		LOG_INF("BT initialized");
		break;
	default:
		LOG_ERR("BT init failed (err %d)", err_code);
		return SID_ERROR_GENERIC;
	}

	sid_ble_conn_init();

	return SID_ERROR_NONE;
}

static sid_error_t ble_adapter_start_service(void)
{
	LOG_DBG("Sidewalk -> BLE");
	return SID_ERROR_NONE;
}

static sid_error_t ble_adapter_set_adv_data(uint8_t *data, uint8_t length)
{
	LOG_DBG("Sidewalk -> BLE");
	if (!data || 0 == length) {
		return SID_ERROR_INVALID_ARGS;
	}

	int err = sid_ble_advert_update(data, length);

	if (err) {
		LOG_ERR("Advertising failed to update (err %d)", err);
		return SID_ERROR_GENERIC;
	}

	return SID_ERROR_NONE;
}

static sid_error_t ble_adapter_start_advertisement(void)
{
	LOG_DBG("Sidewalk -> BLE");
	int err = sid_ble_advert_start();

	if (err) {
		LOG_ERR("Advertising failed to start (err %d)", err);
		return SID_ERROR_GENERIC;
	}

	return SID_ERROR_NONE;
}

static sid_error_t ble_adapter_stop_advertisement(void)
{
	LOG_DBG("Sidewalk -> BLE");
	int err = sid_ble_advert_stop();

	if (err) {
		LOG_ERR("Advertising failed to stop (err %d)", err);
		return SID_ERROR_GENERIC;
	}

	return SID_ERROR_NONE;
}

/* The static literal has lifetime of its context, so as a global its lifetime is static.
   When it was used in a context of a function, it had been stored on the stack of that function, 
   and was not valid outside of this function.*/
static const struct bt_uuid *uuid_ama_service = AMA_SID_BT_CHARACTERISTIC_NOTIFY;
#if defined(CONFIG_SIDEWALK_VENDOR_SERVICE)
static const struct bt_uuid *uuid_vnd_service = VND_SID_BT_CHARACTERISTIC_NOTIFY;
#endif /* CONFIG_SIDEWALK_VENDOR_SERVICE */
#if defined(CONFIG_SIDEWALK_LOGGING_SERVICE)
static const struct bt_uuid *uuid_log_service = LOG_SID_BT_CHARACTERISTIC_NOTIFY;
#endif /* CONFIG_SIDEWALK_VENDOR_SERVICE */

static sid_ble_srv_params_t get_srv_params(sid_ble_cfg_service_identifier_t id)
{
	switch (id) {
	case AMA_SERVICE:
		return (sid_ble_srv_params_t){
			.conn = sid_ble_conn_params_get()->conn,
			.service = (struct bt_gatt_service_static *)sid_ble_get_ama_service(),
			.uuid = uuid_ama_service
		};

#if defined(CONFIG_SIDEWALK_VENDOR_SERVICE)
	case VENDOR_SERVICE:
		return (sid_ble_srv_params_t){
			.conn = sid_ble_conn_params_get()->conn,
			.service = (struct bt_gatt_service_static *)sid_ble_get_vnd_service(),
			.uuid = uuid_vnd_service
		};
#endif /* CONFIG_SIDEWALK_VENDOR_SERVICE */
#if defined(CONFIG_SIDEWALK_LOGGING_SERVICE)
	case LOGGING_SERVICE:
		return (sid_ble_srv_params_t){
			.conn = sid_ble_conn_params_get()->conn,
			.service = (struct bt_gatt_service_static *)sid_ble_get_log_service(),
			.uuid = uuid_log_service
		};
#endif /* CONFIG_SIDEWALK_LOGGING_SERVICE */
	default:
		return (sid_ble_srv_params_t){ .conn = NULL, .uuid = NULL, .service = NULL };
	}
}

static sid_error_t ble_adapter_send_data(sid_ble_cfg_service_identifier_t id, uint8_t *data,
					 uint16_t length)
{
	LOG_DBG("Sidewalk -> BLE");
	sid_ble_srv_params_t srv_params = get_srv_params(id);
	if (srv_params.service == NULL || srv_params.uuid == NULL) {
		return SID_ERROR_NOSUPPORT;
	}

	int err_code = sid_ble_send_data(&srv_params, data, length);
	if (-EINVAL == err_code) {
		return SID_ERROR_INVALID_ARGS;
	} else if (0 > err_code) {
		return SID_ERROR_GENERIC;
	}
	return SID_ERROR_NONE;
}

static sid_error_t ble_adapter_set_callback(const sid_pal_ble_adapter_callbacks_t *cb)
{
	LOG_DBG("Sidewalk -> BLE");
	sid_error_t err_code;

	if (!cb) {
		return SID_ERROR_NULL_POINTER;
	}

	err_code = sid_ble_adapter_notification_cb_set(cb->ind_callback);
	if (SID_ERROR_NONE != err_code) {
		return err_code;
	}

	err_code = sid_ble_adapter_data_cb_set(cb->data_callback);
	if (SID_ERROR_NONE != err_code) {
		return err_code;
	}

	err_code = sid_ble_adapter_notification_changed_cb_set(cb->notify_callback);
	if (SID_ERROR_NONE != err_code) {
		return err_code;
	}

	err_code = sid_ble_adapter_conn_cb_set(cb->conn_callback);
	if (SID_ERROR_NONE != err_code) {
		return err_code;
	}

	err_code = sid_ble_adapter_mtu_cb_set(cb->mtu_callback);
	if (SID_ERROR_NONE != err_code) {
		return err_code;
	}

	err_code = sid_ble_adapter_adv_start_cb_set(cb->adv_start_callback);
	if (SID_ERROR_NONE != err_code) {
		return err_code;
	}

	return SID_ERROR_NONE;
}

static sid_error_t ble_adapter_disconnect(void)
{
	LOG_DBG("Sidewalk -> BLE");
	int err = sid_ble_conn_disconnect();

	if (err) {
		LOG_ERR("Disconnection failed (err %d)", err);
		return SID_ERROR_GENERIC;
	}

	return SID_ERROR_NONE;
}

static sid_error_t ble_adapter_deinit(void)
{
	LOG_DBG("Sidewalk -> BLE");
	sid_ble_conn_deinit();

	int err = bt_disable();

	if (err) {
		LOG_ERR("BT disable failed (error %d)", err);
		return SID_ERROR_GENERIC;
	}

	return SID_ERROR_NONE;
}

sid_error_t sid_pal_ble_adapter_create(sid_pal_ble_adapter_interface_t *handle)
{
	if (!handle) {
		return SID_ERROR_INVALID_ARGS;
	}

	*handle = &ble_ifc;

	return SID_ERROR_NONE;
}
