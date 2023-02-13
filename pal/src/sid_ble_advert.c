/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>
#include <sid_ble_advert.h>
#include <sid_ble_uuid.h>

#include <sid_ble_uuid.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/sys/byteorder.h>

#define MS_TO_INTERVAL_VAL(ms) (uint16_t)((ms) / 0.625f)

#if defined(CONFIG_MAC_ADDRESS_TYPE_RANDOM_PRIVATE_NON_RESOLVABLE)
#define AMA_ADV_OPTIONS     (BT_LE_ADV_OPT_USE_NAME | \
			     BT_LE_ADV_OPT_FORCE_NAME_IN_AD)
#else
#define AMA_ADV_OPTIONS     (BT_LE_ADV_OPT_CONNECTABLE | \
			     BT_LE_ADV_OPT_USE_NAME |	 \
			     BT_LE_ADV_OPT_FORCE_NAME_IN_AD)
#endif

#if 10240 < (CONFIG_SIDEWALK_BLE_ADV_INT_FAST + CONFIG_SIDEWALK_BLE_ADV_INT_PRECISION)
#error \
	"Invalid value for CONFIG_SIDEWALK_BLE_ADV_INT_FAST or CONFIG_SIDEWALK_BLE_ADV_INT_PRECISION, sum of those values have to be smaller than 10240"
#endif

#if 10240 < (CONFIG_SIDEWALK_BLE_ADV_INT_SLOW + CONFIG_SIDEWALK_BLE_ADV_INT_PRECISION)
#error \
	"Invalid value for CONFIG_SIDEWALK_BLE_ADV_INT_SLOW or CONFIG_SIDEWALK_BLE_ADV_INT_PRECISION, sum of those values have to be smaller than 10240"
#endif
#if CONFIG_SIDEWALK_BLE_ADV_INT_FAST > CONFIG_SIDEWALK_BLE_ADV_INT_SLOW
#error "CONFIG_SIDEWALK_BLE_ADV_INT_FAST should be smaller than CONFIG_SIDEWALK_BLE_ADV_INT_SLOW"
#endif

/* Advertising parameters. */
#define AMA_ADV_PARAM_FAST											   \
	BT_LE_ADV_PARAM(											   \
		AMA_ADV_OPTIONS,										   \
		MS_TO_INTERVAL_VAL(CONFIG_SIDEWALK_BLE_ADV_INT_FAST),						   \
		MS_TO_INTERVAL_VAL(CONFIG_SIDEWALK_BLE_ADV_INT_FAST + CONFIG_SIDEWALK_BLE_ADV_INT_PRECISION), NULL \
		)

#define AMA_ADV_PARAM_SLOW											   \
	BT_LE_ADV_PARAM(											   \
		AMA_ADV_OPTIONS,										   \
		MS_TO_INTERVAL_VAL(CONFIG_SIDEWALK_BLE_ADV_INT_SLOW),						   \
		MS_TO_INTERVAL_VAL(CONFIG_SIDEWALK_BLE_ADV_INT_SLOW + CONFIG_SIDEWALK_BLE_ADV_INT_PRECISION), NULL \
		)

/**
 * @brief Advertising data items values size in bytes.
 * @note Need to be up to be keep up to date manually.
 */
#define AD_FLAGS_LEN 1
#define AD_SERVICES_LEN 2
#define AD_NAME_SHORT_LEN 2
#define AD_TLV_TYPE_AND_LENGTH 2
#define AD_TLV_LEN(x) (x + AD_TLV_TYPE_AND_LENGTH)
#define AD_MANUF_DATA_LEN_MAX (BT_GAP_ADV_MAX_ADV_DATA_LEN - \
			       AD_TLV_TYPE_AND_LENGTH  -     \
			       AD_TLV_LEN(AD_FLAGS_LEN) -    \
			       AD_TLV_LEN(AD_SERVICES_LEN) - \
			       AD_TLV_LEN(AD_NAME_SHORT_LEN))

enum adv_data_items {
	ADV_DATA_FLAGS,
	ADV_DATA_SERVICES,
	ADV_DATA_MANUF_DATA
};

typedef enum {
	BLE_ADV_DISABLE,
	BLE_ADV_ENABLE
} sid_ble_adv_state_t;

static void change_adv(struct k_work *);
K_WORK_DELAYABLE_DEFINE(change_adv_work, change_adv);

static atomic_t adv_state = ATOMIC_INIT(BLE_ADV_DISABLE);

static uint8_t bt_adv_manuf_data[AD_MANUF_DATA_LEN_MAX];

static struct bt_data adv_data[] = {
	[ADV_DATA_FLAGS] = BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	[ADV_DATA_SERVICES] = BT_DATA_BYTES(BT_DATA_UUID16_ALL, BT_UUID_16_ENCODE(AMA_SERVICE_UUID_VAL)),
	[ADV_DATA_MANUF_DATA] = BT_DATA(BT_DATA_MANUFACTURER_DATA, &bt_adv_manuf_data, AD_MANUF_DATA_LEN_MAX)
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

static void change_adv(struct k_work *work)
{
	ARG_UNUSED(work);
	if (BLE_ADV_ENABLE == atomic_get(&adv_state)) {

		if (bt_le_adv_stop()) {
			atomic_set(&adv_state, BLE_ADV_DISABLE);
			return;
		}
		if (bt_le_adv_start(AMA_ADV_PARAM_SLOW, adv_data, ARRAY_SIZE(adv_data), NULL, 0)) {
			atomic_set(&adv_state, BLE_ADV_DISABLE);
			return;
		}
	}
}

int sid_ble_advert_start(void)
{
	k_work_reschedule(&change_adv_work, K_SECONDS(CONFIG_SIDEWALK_BLE_ADV_INT_TRANSITION));
	int err = bt_le_adv_start(AMA_ADV_PARAM_FAST, adv_data, ARRAY_SIZE(adv_data), NULL, 0);

	if (err) {
		return err;
	}
	atomic_set(&adv_state, BLE_ADV_ENABLE);

#if defined(CONFIG_MAC_ADDRESS_TYPE_RANDOM_PRIVATE_NON_RESOLVABLE)
	static struct bt_le_oob oob;
	(void)bt_le_oob_get_local(BT_ID_DEFAULT, &oob);
#endif

	return err;
}

int sid_ble_advert_stop(void)
{
	k_work_cancel_delayable(&change_adv_work);
	int err = bt_le_adv_stop();

	if (0 == err) {
		atomic_set(&adv_state, BLE_ADV_DISABLE);
	}

	return err;
}

int sid_ble_advert_update(uint8_t *data, uint8_t data_len)
{
	if (!data || 0 == data_len) {
		return -EINVAL;
	}

	adv_data[ADV_DATA_MANUF_DATA].data_len = advert_manuf_data_copy(data, data_len);

	int err = 0;

	if (BLE_ADV_ENABLE == atomic_get(&adv_state)) {
		err = bt_le_adv_update_data(adv_data, ARRAY_SIZE(adv_data), NULL, 0);
	}

	return err;
}
