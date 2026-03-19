/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>
#include <sid_ble_advert.h>
#include <sid_ble_uuid.h>

#include <bt_app_callbacks.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/logging/log.h>
#include <string.h>

LOG_MODULE_REGISTER(sid_ble_advert, CONFIG_SIDEWALK_BLE_ADAPTER_LOG_LEVEL);

/* Advertising data */
#define AMA_ADV_OPTIONS (BT_LE_ADV_OPT_CONN)

#define AD_FLAGS_LEN 1
#define AD_SERVICES_LEN 2
#define AD_TLV_TYPE_AND_LENGTH 2
#define AD_TLV_LEN(x) (x + AD_TLV_TYPE_AND_LENGTH)
#define AD_MANUF_DATA_LEN_MAX                                                                  \
	(BT_GAP_ADV_MAX_ADV_DATA_LEN - AD_TLV_TYPE_AND_LENGTH - AD_TLV_LEN(AD_FLAGS_LEN) -         \
	 AD_TLV_LEN(AD_SERVICES_LEN))
static uint8_t bt_adv_manuf_data[AD_MANUF_DATA_LEN_MAX];

enum adv_data_items { ADV_DATA_FLAGS, ADV_DATA_SERVICES, ADV_DATA_MANUF_DATA };
static struct bt_data ad[] = {
	[ADV_DATA_FLAGS] = BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	[ADV_DATA_SERVICES] =
		BT_DATA_BYTES(BT_DATA_UUID16_ALL, BT_UUID_16_ENCODE(AMA_SERVICE_UUID_VAL)),
	[ADV_DATA_MANUF_DATA] =
		BT_DATA(BT_DATA_MANUFACTURER_DATA, &bt_adv_manuf_data, AD_MANUF_DATA_LEN_MAX),
};

static const struct bt_data sd[] = {
	BT_DATA(BT_DATA_NAME_COMPLETE, CONFIG_SIDEWALK_BLE_NAME,
		sizeof(CONFIG_SIDEWALK_BLE_NAME) - 1),
};

/* Advertising interval */
#define MS_TO_INTERVAL_VAL(ms) (uint16_t)(((uint32_t)(ms) * 8U) / 5U)

#if 10240 < (CONFIG_SIDEWALK_BLE_ADV_INT_FAST + CONFIG_SIDEWALK_BLE_ADV_INT_PRECISION)
#error "Invalid value for CONFIG_SIDEWALK_BLE_ADV_INT_FAST or CONFIG_SIDEWALK_BLE_ADV_INT_PRECISION, sum of those values have to be smaller than 10240"
#endif

#if 10240 < (CONFIG_SIDEWALK_BLE_ADV_INT_SLOW + CONFIG_SIDEWALK_BLE_ADV_INT_PRECISION)
#error "Invalid value for CONFIG_SIDEWALK_BLE_ADV_INT_SLOW or CONFIG_SIDEWALK_BLE_ADV_INT_PRECISION, sum of those values have to be smaller than 10240"
#endif
#if CONFIG_SIDEWALK_BLE_ADV_INT_FAST > CONFIG_SIDEWALK_BLE_ADV_INT_SLOW
#error "CONFIG_SIDEWALK_BLE_ADV_INT_FAST should be smaller than CONFIG_SIDEWALK_BLE_ADV_INT_SLOW"
#endif

static struct bt_le_ext_adv *adv_set;

/* Default: same units as sid_ble_cfg_adv_param_t (0.625 ms, 10 ms). */
static sid_ble_advert_params_t advert_params = {
	.fast_enabled = true,
	.slow_enabled = true,
	.fast_interval = MS_TO_INTERVAL_VAL(CONFIG_SIDEWALK_BLE_ADV_INT_FAST),
	.fast_timeout = CONFIG_SIDEWALK_BLE_ADV_INT_TRANSITION * 100U,
	.slow_interval = MS_TO_INTERVAL_VAL(CONFIG_SIDEWALK_BLE_ADV_INT_SLOW),
	.slow_timeout = 0,
};

static struct bt_le_adv_param adv_param_fast = {
	.id = BT_ID_SIDEWALK,
	.options = (AMA_ADV_OPTIONS),
};

static struct bt_le_adv_param adv_param_slow = {
	.id = BT_ID_SIDEWALK,
	.options = (AMA_ADV_OPTIONS),
};

typedef enum { BLE_ADV_DISABLE, BLE_ADV_FAST, BLE_ADV_SLOW } sid_ble_adv_state_t;
static atomic_t adv_state = ATOMIC_INIT(BLE_ADV_DISABLE);

static void adv_interval_change(struct k_work *);
K_WORK_DELAYABLE_DEFINE(change_adv_work, adv_interval_change);

static void adv_param_set_interval(struct bt_le_adv_param *param, uint32_t interval)
{
	param->interval_min = (uint16_t)interval;
	param->interval_max =
		(uint16_t)(interval + MS_TO_INTERVAL_VAL(CONFIG_SIDEWALK_BLE_ADV_INT_PRECISION));
}

static int adv_start_with_param(struct bt_le_adv_param *param, sid_ble_adv_state_t new_state,
				uint32_t timeout_10ms)
{
	struct bt_le_ext_adv_start_param ext_adv_start_param = { 0 };
	int err;

	err = bt_le_ext_adv_create(param, NULL, &adv_set);
	if (err) {
		LOG_ERR("Failed to create adv set errno %d (%s)", err, strerror(err));
		return err;
	}
	err = bt_le_ext_adv_set_data(adv_set, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
	if (err) {
		bt_le_ext_adv_delete(adv_set);
		adv_set = NULL;
		LOG_ERR("Failed to set adv data errno %d (%s)", err, strerror(err));
		return err;
	}
	err = bt_le_ext_adv_start(adv_set, &ext_adv_start_param);
	if (err) {
		bt_le_ext_adv_delete(adv_set);
		adv_set = NULL;
		LOG_ERR("Failed to start adv errno %d (%s)", err, strerror(err));
		return err;
	}
	atomic_set(&adv_state, new_state);
	if (timeout_10ms > 0) {
		k_work_reschedule(&change_adv_work, K_MSEC((uint32_t)timeout_10ms * 10U));
	}
	return 0;
}

static void adv_interval_change(struct k_work *work)
{
	ARG_UNUSED(work);
	int err;

	LOG_INF("Change advertisement interval");

	if (!adv_set) {
		return;
	}
	err = bt_le_ext_adv_stop(adv_set);
	if (err) {
		atomic_set(&adv_state, BLE_ADV_DISABLE);
		LOG_ERR("Failed to stop adv errno %d (%s)", err, strerror(err));
		return;
	}
	err = bt_le_ext_adv_delete(adv_set);
	if (err) {
		atomic_set(&adv_state, BLE_ADV_DISABLE);
		LOG_ERR("Failed to delete adv set errno %d", err);
		return;
	}
	adv_set = NULL;

	sid_ble_adv_state_t prev = atomic_get(&adv_state);
	if (prev == BLE_ADV_FAST && advert_params.slow_enabled) {
		adv_param_set_interval(&adv_param_slow, advert_params.slow_interval);
		err = adv_start_with_param(&adv_param_slow, BLE_ADV_SLOW,
					   advert_params.slow_timeout);
		if (err) {
			atomic_set(&adv_state, BLE_ADV_DISABLE);
			LOG_DBG("Failed to switch to slow advertising, err %d", err);
			return;
		}
		LOG_DBG("Switched to slow advertising, err %d", err);
	} else {
		atomic_set(&adv_state, BLE_ADV_DISABLE);
		if (prev == BLE_ADV_SLOW) {
			LOG_DBG("Slow advertising timeout, stopped");
		}
		LOG_DBG("BLE advertising disabled");
	}
}

static uint8_t adv_manuf_data_copy(uint8_t *data, uint8_t data_len)
{
	uint16_t ama_id = sys_cpu_to_le16(BT_COMP_ID_AMA);
	uint8_t ama_id_len = sizeof(ama_id);
	uint8_t new_data_len = MIN(data_len, AD_MANUF_DATA_LEN_MAX - ama_id_len);

	memcpy(bt_adv_manuf_data, &ama_id, ama_id_len);
	memcpy(&bt_adv_manuf_data[ama_id_len], data, new_data_len);

	return new_data_len + ama_id_len;
}

int sid_ble_advert_init(void)
{
	/* No initialization needed, left for backward compatibility */
	return 0;
}

int sid_ble_advert_start(void)
{
	struct bt_le_adv_param *param;
	sid_ble_adv_state_t state;
	uint32_t timeout;
	int err;

	if (adv_set != NULL) {
		struct k_work_sync sync;

		(void)k_work_cancel_delayable_sync(&change_adv_work, &sync);
		(void)bt_le_ext_adv_stop(adv_set);
		(void)bt_le_ext_adv_delete(adv_set);
		adv_set = NULL;
		atomic_set(&adv_state, BLE_ADV_DISABLE);
	}

	if (advert_params.fast_enabled) {
		adv_param_set_interval(&adv_param_fast, advert_params.fast_interval);
		param = &adv_param_fast;
		state = BLE_ADV_FAST;
		timeout = advert_params.fast_timeout;
	} else if (advert_params.slow_enabled) {
		adv_param_set_interval(&adv_param_slow, advert_params.slow_interval);
		param = &adv_param_slow;
		state = BLE_ADV_SLOW;
		timeout = advert_params.slow_timeout;
	} else {
		LOG_WRN("No advertising enabled");
		return 0;
	}

	err = adv_start_with_param(param, state, timeout);
	return err;
}

int sid_ble_advert_update(uint8_t *data, uint8_t data_len)
{
	sid_ble_adv_state_t state = atomic_get(&adv_state);

	if (!data || 0 == data_len) {
		return -EINVAL;
	}

	ad[ADV_DATA_MANUF_DATA].data_len = adv_manuf_data_copy(data, data_len);

	int err = 0;

	if (BLE_ADV_DISABLE != state) {
		/* Update currently advertised set, the other one will be set on start/transition */
		err = bt_le_ext_adv_set_data(adv_set, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
	}

	return err;
}

void sid_ble_advert_notify_connection(void)
{
	LOG_DBG("Connection has been made, cancel change adv");
	k_work_cancel_delayable(&change_adv_work);
}

int sid_ble_advert_params_set(sid_ble_advert_params_t *params)
{
	if (!params) {
		return -EINVAL;
	}
	LOG_DBG("sid_ble_advert_params_set: fast_enabled=%d, slow_enabled=%d, fast_interval=%d, fast_timeout=%d, slow_interval=%d, slow_timeout=%d",
		params->fast_enabled, params->slow_enabled, params->fast_interval,
		params->fast_timeout, params->slow_interval, params->slow_timeout);

	advert_params = *params;
	return 0;
}

int sid_ble_advert_params_get(sid_ble_advert_params_t *params)
{
	if (!params) {
		return -EINVAL;
	}
	*params = advert_params;
	return 0;
}

int sid_ble_advert_stop(void)
{
	struct k_work_sync sync;

	/* Sync with change_adv_work so it cannot run in parallel and clear adv_set. */
	(void)k_work_cancel_delayable_sync(&change_adv_work, &sync);
	if (adv_set == NULL) {
		atomic_set(&adv_state, BLE_ADV_DISABLE);
		return 0;
	}
	int err = bt_le_ext_adv_stop(adv_set);

	if (0 == err) {
		atomic_set(&adv_state, BLE_ADV_DISABLE);
	}

	return err;
}

int sid_ble_advert_deinit(void)
{
	if (adv_set) {
		int err = bt_le_ext_adv_delete(adv_set);
		if (err) {
			LOG_ERR("Failed to delete adv_set_fast errno %d (%s)", err, strerror(err));
			return err;
		}
		adv_set = NULL;
	}

	return 0;
}
