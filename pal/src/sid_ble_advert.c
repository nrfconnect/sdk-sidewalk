/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#include <sid_ble_advert.h>
#include <sid_ble_uuid.h>

#include <sid_ble_uuid.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/sys/byteorder.h>

#ifdef CONFIG_BLE_ADV_SLOW_INT_1000_1200_MS
#define ADV_INT_MIN     BT_GAP_ADV_SLOW_INT_MIN
#define ADV_INT_MAX     BT_GAP_ADV_SLOW_INT_MAX
#elif CONFIG_BLE_ADV_FAST_INT_30_60_MS
#define ADV_INT_MIN     BT_GAP_ADV_FAST_INT_MIN_1
#define ADV_INT_MAX     BT_GAP_ADV_FAST_INT_MAX_1
#else
#define ADV_INT_MIN     BT_GAP_ADV_FAST_INT_MIN_2
#define ADV_INT_MAX     BT_GAP_ADV_FAST_INT_MAX_2
#endif

#if defined(CONFIG_MAC_ADDRESS_TYPE_RANDOM_PRIVATE_NON_RESOLVABLE)
#define AMA_ADV_OPTIONS     (BT_LE_ADV_OPT_USE_NAME | \
			     BT_LE_ADV_OPT_FORCE_NAME_IN_AD)
#else
#define AMA_ADV_OPTIONS     (BT_LE_ADV_OPT_CONNECTABLE | \
			     BT_LE_ADV_OPT_USE_NAME |	 \
			     BT_LE_ADV_OPT_FORCE_NAME_IN_AD)
#endif

/* Advertising parameters. */
#define AMA_ADV_PARAM		  \
	BT_LE_ADV_PARAM(	  \
		AMA_ADV_OPTIONS,  \
		ADV_INT_MIN,	  \
		ADV_INT_MAX, NULL \
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

static sid_ble_adv_state_t adv_state;

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

int sid_ble_advert_start(void)
{
	int err = bt_le_adv_start(AMA_ADV_PARAM, adv_data, ARRAY_SIZE(adv_data), NULL, 0);

	if (!err) {
		adv_state = BLE_ADV_ENABLE;
	}

#if defined(CONFIG_MAC_ADDRESS_TYPE_RANDOM_PRIVATE_NON_RESOLVABLE)
	static struct bt_le_oob oob;
	(void)bt_le_oob_get_local(BT_ID_DEFAULT, &oob);
#endif

	return err;
}

int sid_ble_advert_stop(void)
{
	int err = bt_le_adv_stop();

	if (0 == err) {
		adv_state = BLE_ADV_DISABLE;
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

	if (BLE_ADV_ENABLE == adv_state) {
		err = bt_le_adv_update_data(adv_data, ARRAY_SIZE(adv_data), NULL, 0);
	}

	return err;
}
