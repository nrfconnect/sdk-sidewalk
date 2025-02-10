/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>
#include <sid_ble_advert.h>
#include <sid_ble_uuid.h>

#include <sid_ble_uuid.h>
#include <bt_app_callbacks.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/logging/log.h>
#include <string.h>

LOG_MODULE_REGISTER(sid_ble_advert, CONFIG_SIDEWALK_BLE_ADAPTER_LOG_LEVEL);

#define MS_TO_INTERVAL_VAL(ms) (uint16_t)((ms) / 0.625f)

#define AMA_ADV_OPTIONS (BT_LE_ADV_OPT_CONN)

#if 10240 < (CONFIG_SIDEWALK_BLE_ADV_INT_FAST + CONFIG_SIDEWALK_BLE_ADV_INT_PRECISION)
#error "Invalid value for CONFIG_SIDEWALK_BLE_ADV_INT_FAST or CONFIG_SIDEWALK_BLE_ADV_INT_PRECISION, sum of those values have to be smaller than 10240"
#endif

#if 10240 < (CONFIG_SIDEWALK_BLE_ADV_INT_SLOW + CONFIG_SIDEWALK_BLE_ADV_INT_PRECISION)
#error "Invalid value for CONFIG_SIDEWALK_BLE_ADV_INT_SLOW or CONFIG_SIDEWALK_BLE_ADV_INT_PRECISION, sum of those values have to be smaller than 10240"
#endif
#if CONFIG_SIDEWALK_BLE_ADV_INT_FAST > CONFIG_SIDEWALK_BLE_ADV_INT_SLOW
#error "CONFIG_SIDEWALK_BLE_ADV_INT_FAST should be smaller than CONFIG_SIDEWALK_BLE_ADV_INT_SLOW"
#endif

/* Advertising parameters. */
static struct bt_le_adv_param adv_param_fast = {
	.id = BT_ID_SIDEWALK,
	.options = (AMA_ADV_OPTIONS),
	.interval_min = MS_TO_INTERVAL_VAL(CONFIG_SIDEWALK_BLE_ADV_INT_FAST),
	.interval_max = MS_TO_INTERVAL_VAL(CONFIG_SIDEWALK_BLE_ADV_INT_FAST +
					   CONFIG_SIDEWALK_BLE_ADV_INT_PRECISION),
};

static struct bt_le_adv_param adv_param_slow = {
	.id = BT_ID_SIDEWALK,
	.options = (AMA_ADV_OPTIONS),
	.interval_min = MS_TO_INTERVAL_VAL(CONFIG_SIDEWALK_BLE_ADV_INT_SLOW),
	.interval_max = MS_TO_INTERVAL_VAL(CONFIG_SIDEWALK_BLE_ADV_INT_SLOW +
					   CONFIG_SIDEWALK_BLE_ADV_INT_PRECISION),
};

static struct bt_le_ext_adv *adv_set = NULL;

/**
 * @brief Advertising data items values size in bytes.
 * @note Need to be up to be keep up to date manually.
 */
#define AD_FLAGS_LEN 1
#define AD_SERVICES_LEN 2
#define AD_NAME_SHORT_LEN 2
#define AD_TLV_TYPE_AND_LENGTH 2
#define AD_TLV_LEN(x) (x + AD_TLV_TYPE_AND_LENGTH)
#define AD_MANUF_DATA_LEN_MAX                                                                      \
	(BT_GAP_ADV_MAX_ADV_DATA_LEN - AD_TLV_TYPE_AND_LENGTH - AD_TLV_LEN(AD_FLAGS_LEN) -         \
	 AD_TLV_LEN(AD_SERVICES_LEN) - AD_TLV_LEN(AD_NAME_SHORT_LEN))

enum adv_data_items { ADV_DATA_FLAGS, ADV_DATA_SERVICES, ADV_DATA_MANUF_DATA, ADV_DATA_NAME };

typedef enum { BLE_ADV_DISABLE, BLE_ADV_FAST, BLE_ADV_SLOW } sid_ble_adv_state_t;

static void change_advertisement_interval(struct k_work *);
K_WORK_DELAYABLE_DEFINE(change_adv_work, change_advertisement_interval);

static atomic_t adv_state = ATOMIC_INIT(BLE_ADV_DISABLE);

static uint8_t bt_adv_manuf_data[AD_MANUF_DATA_LEN_MAX];

static struct bt_data adv_data[] = {
	[ADV_DATA_FLAGS] = BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	[ADV_DATA_SERVICES] =
		BT_DATA_BYTES(BT_DATA_UUID16_ALL, BT_UUID_16_ENCODE(AMA_SERVICE_UUID_VAL)),
	[ADV_DATA_MANUF_DATA] =
		BT_DATA(BT_DATA_MANUFACTURER_DATA, &bt_adv_manuf_data, AD_MANUF_DATA_LEN_MAX),
	[ADV_DATA_NAME] = BT_DATA(BT_DATA_NAME_COMPLETE, CONFIG_SIDEWALK_BLE_NAME,
				  sizeof(CONFIG_SIDEWALK_BLE_NAME) - 1),
};

static const struct bt_data sd[] = {
	BT_DATA(BT_DATA_NAME_COMPLETE, CONFIG_SIDEWALK_BLE_NAME,
		sizeof(CONFIG_SIDEWALK_BLE_NAME) - 1),
};

/**
 * @brief The function copy manufacturing data to static buffer used in BLE avertising.
 *
 * @param data buffer with data to copy.
 * @param data_len number of bytes to copy.
 *
 * @return number of bytes written to manufacuring data in avertising.
 */
static uint8_t advert_manuf_data_copy(uint8_t *data, uint8_t data_len)
{
	uint16_t ama_id = sys_cpu_to_le16(BT_COMP_ID_AMA);
	uint8_t ama_id_len = sizeof(ama_id);
	uint8_t new_data_len = MIN(data_len, AD_MANUF_DATA_LEN_MAX - ama_id_len);

	memcpy(bt_adv_manuf_data, &ama_id, ama_id_len);
	memcpy(&bt_adv_manuf_data[ama_id_len], data, new_data_len);

	return new_data_len + ama_id_len;
}

static void change_advertisement_interval(struct k_work *work)
{
	ARG_UNUSED(work);
	LOG_INF("Change advertisement interval");

	struct bt_le_ext_adv_start_param ext_adv_start_param = { 0 };

	if (BLE_ADV_FAST == atomic_get(&adv_state)) {
		int err = 0;
		err = bt_le_ext_adv_stop(adv_set);
		if (err) {
			atomic_set(&adv_state, BLE_ADV_DISABLE);
			LOG_ERR("Failed to stop fast adv errno %d (%s)", err, strerror(err));
			return;
		}
		err = bt_le_ext_adv_delete(adv_set);
		if (err) {
			atomic_set(&adv_state, BLE_ADV_DISABLE);
			LOG_ERR("Failed to delete adv set");
			return;
		}
		adv_set = NULL;
		err = bt_le_ext_adv_create(&adv_param_slow, NULL, &adv_set);
		if (err) {
			atomic_set(&adv_state, BLE_ADV_DISABLE);
			LOG_ERR("Failed to create slow adv set");
			return;
		}
		err = bt_le_ext_adv_set_data(adv_set, adv_data, ARRAY_SIZE(adv_data), sd,
					     ARRAY_SIZE(sd));
		if (err) {
			atomic_set(&adv_state, BLE_ADV_DISABLE);
			LOG_ERR("Failed to set adv data to slow adv errno %d (%s)", err,
				strerror(err));
			return;
		}
		err = bt_le_ext_adv_start(adv_set, &ext_adv_start_param);
		if (err) {
			atomic_set(&adv_state, BLE_ADV_DISABLE);
			LOG_ERR("Failed to start slow adv errno %d (%s)", err, strerror(err));
			return;
		}

		atomic_set(&adv_state, BLE_ADV_SLOW);
		LOG_DBG("Change succesful");
	}
}

int sid_ble_advert_init(void)
{
	int ret;
	if (adv_set == NULL) {
		ret = bt_le_ext_adv_create(&adv_param_fast, NULL, &adv_set);
		if (ret) {
			LOG_ERR("Failed to create fast advertising set errno %d (%s)", ret,
				strerror(ret));
			return ret;
		}
	}

	return 0;
}

int sid_ble_advert_deinit(void)
{
	if (adv_set != NULL) {
		int err = bt_le_ext_adv_delete(adv_set);
		if (err) {
			LOG_ERR("Failed to delete adv_set_fast errno %d (%s)", err, strerror(err));
			return err;
		}
		adv_set = NULL;
	}

	return 0;
}

void sid_ble_advert_notify_connection(void)
{
	LOG_DBG("Conneciton has been made, cancel change adv");
	k_work_cancel_delayable(&change_adv_work);
}

int sid_ble_advert_start(void)
{
	struct bt_le_ext_adv_start_param ext_adv_start_param = { 0 };
	int err = 0;
	// make sure to always start with fast advertising set
	sid_ble_advert_deinit();
	sid_ble_advert_init();
	err = bt_le_ext_adv_set_data(adv_set, adv_data, ARRAY_SIZE(adv_data), sd, ARRAY_SIZE(sd));
	if (err) {
		atomic_set(&adv_state, BLE_ADV_DISABLE);
		LOG_ERR("Failed to set fast adv data errno: %d (%s)", err, strerror(err));
		return err;
	}

	err = bt_le_ext_adv_start(adv_set, &ext_adv_start_param);
	if (err) {
		atomic_set(&adv_state, BLE_ADV_DISABLE);
		LOG_ERR("Failed to start fast adv errno: %d (%s)", err, strerror(err));
		return err;
	}

	atomic_set(&adv_state, BLE_ADV_FAST);
	k_work_reschedule(&change_adv_work, K_SECONDS(CONFIG_SIDEWALK_BLE_ADV_INT_TRANSITION));

	return err;
}

int sid_ble_advert_stop(void)
{
	k_work_cancel_delayable(&change_adv_work);
	int err = bt_le_ext_adv_stop(adv_set);

	if (0 == err) {
		atomic_set(&adv_state, BLE_ADV_DISABLE);
	}

	return err;
}

int sid_ble_advert_update(uint8_t *data, uint8_t data_len)
{
	sid_ble_adv_state_t state = atomic_get(&adv_state);

	if (!data || 0 == data_len) {
		return -EINVAL;
	}

	adv_data[ADV_DATA_MANUF_DATA].data_len = advert_manuf_data_copy(data, data_len);

	int err = 0;

	if (BLE_ADV_DISABLE != state) {
		/* Update currently advertised set, the other one will be set on start/transition */
		err = bt_le_ext_adv_set_data(adv_set, adv_data, ARRAY_SIZE(adv_data), sd,
					     ARRAY_SIZE(sd));
	}

	return err;
}
