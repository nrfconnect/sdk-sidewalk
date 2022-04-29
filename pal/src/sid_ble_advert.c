/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#include <sid_ble_advert.h>
#include <sid_ble_uuid.h>

#include <sid_ble_uuid.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/uuid.h>

typedef enum {
	BLE_ADV_DISABLE,
	BLE_ADV_ENABLE
} sid_ble_adv_state_t;

static sid_ble_adv_state_t adv_state;

/* Advertising parameters. */
#define AMA_ADV_PARAM				\
	BT_LE_ADV_PARAM(			\
		BT_LE_ADV_OPT_CONNECTABLE |	\
		BT_LE_ADV_OPT_USE_NAME |	\
		BT_LE_ADV_OPT_FORCE_NAME_IN_AD,	\
		BT_GAP_ADV_FAST_INT_MIN_2,	\
		BT_GAP_ADV_FAST_INT_MAX_2, NULL	\
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
#define AD_MANUF_DATA_LEN_MAX (BT_GAP_ADV_MAX_ADV_DATA_LEN   \
			       - AD_TLV_TYPE_AND_LENGTH	     \
			       - AD_TLV_LEN(AD_FLAGS_LEN)    \
			       - AD_TLV_LEN(AD_SERVICES_LEN) \
			       - AD_TLV_LEN(AD_NAME_SHORT_LEN))

static uint8_t bt_adv_manuf_data[AD_MANUF_DATA_LEN_MAX];

enum adv_data_items {
	ADV_DATA_FLAGS,
	ADV_DATA_SERVICES,
	ADV_DATA_MANUF_DATA
};

static struct bt_data adv_data[] = {
	[ADV_DATA_FLAGS] = BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	[ADV_DATA_SERVICES] = BT_DATA_BYTES(BT_DATA_UUID16_ALL, BT_UUID_16_ENCODE(AMA_SERVICE_UUID_VAL)),
	[ADV_DATA_MANUF_DATA] = BT_DATA(BT_DATA_MANUFACTURER_DATA, &bt_adv_manuf_data, AD_MANUF_DATA_LEN_MAX)
};

/**
 * @brief Function to set manufacturing data in ble advertisment.
 *
 * @note It just updates advertising data structure.
 * It is required to run bluetooth advertising data update function separately.
 *
 * @param data
 * @param data_len
 *
 * @return number of bytes written to manufacuring data in avertising.
 */
static uint8_t sid_ble_advert_manuf_data_update(uint8_t *data, uint8_t data_len)
{
	uint8_t new_data_len = MIN(data_len, AD_MANUF_DATA_LEN_MAX);

	memcpy(bt_adv_manuf_data, data, new_data_len);

	return new_data_len;
}

int sid_ble_advert_start(void)
{
	int err = bt_le_adv_start(AMA_ADV_PARAM, adv_data, ARRAY_SIZE(adv_data), NULL, 0);

	if (0 == err) {
		adv_state = BLE_ADV_ENABLE;
	}

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

	adv_data[ADV_DATA_MANUF_DATA].data_len = sid_ble_advert_manuf_data_update(data, data_len);

	int err = 0;
	if (BLE_ADV_ENABLE == adv_state) {
		err = bt_le_adv_update_data(adv_data, ARRAY_SIZE(adv_data), NULL, 0);
	}

	return err;
}
