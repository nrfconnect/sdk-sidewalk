/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/** @file sid_storage.c
 *  @brief Sidewalk nvm storage.
 */

#include <sid_pal_storage_kv_ifc.h>
#include <stdint.h>
#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/settings/settings.h>
#ifdef CONFIG_SIDEWALK_CRYPTO_PSA_KEY_STORAGE
#include <sid_crypto_keys.h>

#define STORAGE_KV_INTERNAL_PROTOCOL_GROUP_ID 0
#define STORAGE_KV_WAN_MASTER_KEY (28)
#define STORAGE_KV_APP_MASTER_KEY (30)
#define STORAGE_KV_D2D_MASTER_KEY (48)
#define STORAGE_MASTER_KEY_SIZE (16)
#endif /* CONFIG_SIDEWALK_CRYPTO_PSA_KEY_STORAGE */

#include <zephyr/logging/log.h>
#include <settings_utils.h>

LOG_MODULE_REGISTER(sid_storage, CONFIG_SIDEWALK_LOG_LEVEL);

#define STORAGE_SERIAL_SIZE (32)

static void settings_serialize_group(char *serial, size_t serial_size, uint16_t group)
{
	snprintf(serial, serial_size, "sidewalk/storage/%04x", group);
}

static void settings_serialize_group_key(char *serial, size_t serial_size, uint16_t group,
					 uint16_t key)
{
	snprintf(serial, serial_size, "sidewalk/storage/%04x/%04x", group, key);
}

#ifdef CONFIG_SIDEWALK_CRYPTO_PSA_KEY_STORAGE
static psa_key_id_t storage2key_id(uint16_t group, uint16_t key)
{
	if (STORAGE_KV_INTERNAL_PROTOCOL_GROUP_ID == group) {
		switch (key) {
		case STORAGE_KV_WAN_MASTER_KEY:
			return SID_CRYPTO_KV_WAN_MASTER_KEY_ID;
		case STORAGE_KV_APP_MASTER_KEY:
			return SID_CRYPTO_KV_APP_KEY_KEY_ID;
		case STORAGE_KV_D2D_MASTER_KEY:
			return SID_CRYPTO_KV_D2D_KEY_ID;
		}
	}
	return PSA_KEY_ID_NULL;
}

static void storage_key_save_secure(uint16_t group, uint16_t key)
{
	int err = 0;
	char serial[STORAGE_SERIAL_SIZE] = { 0 };
	uint8_t data[STORAGE_MASTER_KEY_SIZE];
	psa_key_id_t key_id = storage2key_id(group, key);

	settings_serialize_group_key(serial, sizeof(serial), group, key);
	err = settings_utils_load_immediate_value(serial, (void *)data, STORAGE_MASTER_KEY_SIZE);
	if (err == -ENOENT) {
		LOG_DBG("not found key %04x", key);
		return;
	}
	if (err < 0) {
		LOG_ERR("load key %04x err %d", key, err);
		return;
	}

	err = sid_crypto_keys_new_import(key_id, (void *)data, STORAGE_MASTER_KEY_SIZE);
	if (err) {
		LOG_ERR("crypto import %d err %d", key_id, err);
		return;
	}

	err = settings_delete(serial);
	if (err) {
		LOG_ERR("delete key %04x err %d", key, err);
		return;
	}
}

static bool storage_key_delete_secure(sid_crypto_key_id_t id)
{
	int err = sid_crypto_keys_delete(id);
	if (-EINVAL == err) {
		LOG_INF("Not found secure key id %d", id);
	} else if (err) {
		LOG_ERR("Failed to delete secure key id %d", id);
		return false;
	}

	return true;
}

#endif /* CONFIG_SIDEWALK_CRYPTO_PSA_KEY_STORAGE */

sid_error_t sid_pal_storage_kv_init()
{
	int rc = settings_subsys_init();
	if (rc != 0) {
		LOG_ERR("settings init failed (err %d)", rc);
		return SID_ERROR_GENERIC;
	}

	rc = settings_load();
	if (rc != 0) {
		LOG_ERR("settings load failed (err %d)", rc);
		return SID_ERROR_GENERIC;
	}

	LOG_DBG("Initialized KV storage");

#ifdef CONFIG_SIDEWALK_CRYPTO_PSA_KEY_STORAGE
	int ret = sid_crypto_keys_init();
	if (ret != 0) {
		LOG_ERR("Failed to initialize crypto_keys_storage returned errno %d", ret);
		return SID_ERROR_GENERIC;
	}
	storage_key_save_secure(STORAGE_KV_INTERNAL_PROTOCOL_GROUP_ID, STORAGE_KV_WAN_MASTER_KEY);
	storage_key_save_secure(STORAGE_KV_INTERNAL_PROTOCOL_GROUP_ID, STORAGE_KV_APP_MASTER_KEY);
	storage_key_save_secure(STORAGE_KV_INTERNAL_PROTOCOL_GROUP_ID, STORAGE_KV_D2D_MASTER_KEY);
#endif /* CONFIG_SIDEWALK_CRYPTO_PSA_KEY_STORAGE */

	return SID_ERROR_NONE;
}

sid_error_t sid_pal_storage_kv_record_get(uint16_t group, uint16_t key, void *p_data, uint32_t len)
{
	if (!p_data) {
		return SID_ERROR_NULL_POINTER;
	}

#ifdef CONFIG_SIDEWALK_CRYPTO_PSA_KEY_STORAGE
	psa_key_id_t key_id = storage2key_id(group, key);
	if (SID_CRYPTO_KEYS_ID_IS_SIDEWALK_KEY(key_id)) {
		int err = sid_crypto_keys_buffer_set(key_id, (uint8_t *)p_data, len);
		if (err) {
			return SID_ERROR_STORAGE_READ_FAIL;
		} else {
			return SID_ERROR_NONE;
		}
	}
#endif /* CONFIG_SIDEWALK_CRYPTO_PSA_KEY_STORAGE */

	char serial[STORAGE_SERIAL_SIZE] = { 0 };
	settings_serialize_group_key(serial, sizeof(serial), group, key);
	int rc = settings_utils_load_immediate_value(serial, p_data, len);
	if (rc <= 0) {
		return SID_ERROR_NOT_FOUND;
	} else
		return SID_ERROR_NONE;
}

sid_error_t sid_pal_storage_kv_record_get_len(uint16_t group, uint16_t key, uint32_t *p_len)
{
	if (!p_len) {
		return SID_ERROR_NULL_POINTER;
	}
	char serial[STORAGE_SERIAL_SIZE] = { 0 };
	settings_serialize_group_key(serial, sizeof(serial), group, key);
	int rc = settings_utils_get_value_size(serial, p_len);
	if (rc < 0 || *p_len == 0)
		return SID_ERROR_NOT_FOUND;
	else
		return SID_ERROR_NONE;
}

sid_error_t sid_pal_storage_kv_record_set(uint16_t group, uint16_t key, void const *p_data,
					  uint32_t len)
{
	if (!p_data) {
		return SID_ERROR_NULL_POINTER;
	}
	if (len == 0) {
		return SID_ERROR_INVALID_ARGS;
	}

#ifdef CONFIG_SIDEWALK_CRYPTO_PSA_KEY_STORAGE
	psa_key_id_t key_id = storage2key_id(group, key);
	if (SID_CRYPTO_KEYS_ID_IS_SIDEWALK_KEY(key_id)) {
		int err = sid_crypto_keys_new_import(key_id, (uint8_t *)p_data, len);
		if (err == -EBADR) {
			LOG_ERR("Key data is key id, aborting");
			return SID_ERROR_NONE;
		} else if (err) {
			LOG_ERR("Failed to write secure key id %d", key_id);
			return SID_ERROR_STORAGE_WRITE_FAIL;
		} else {
			return SID_ERROR_NONE;
		}
	}
#endif /* CONFIG_SIDEWALK_CRYPTO_PSA_KEY_STORAGE */

	char serial[STORAGE_SERIAL_SIZE] = { 0 };
	settings_serialize_group_key(serial, sizeof(serial), group, key);

	int rc = settings_save_one(serial, (const void *)p_data, len);
	if (rc != 0) {
		LOG_ERR("Failed to save record (%s). Returned errno %d", serial, rc);
		return SID_ERROR_STORAGE_WRITE_FAIL;
	}

	rc = settings_commit();
	if (rc != 0) {
		LOG_ERR("Failed to commit changes. Returned errno %d", rc);
		return SID_ERROR_GENERIC;
	}
	return SID_ERROR_NONE;
}

sid_error_t sid_pal_storage_kv_record_delete(uint16_t group, uint16_t key)
{
#ifdef CONFIG_SIDEWALK_CRYPTO_PSA_KEY_STORAGE
	psa_key_id_t key_id = storage2key_id(group, key);
	if (SID_CRYPTO_KEYS_ID_IS_SIDEWALK_KEY(key_id)) {
		return storage_key_delete_secure(key_id) ? SID_ERROR_NONE :
							   SID_ERROR_STORAGE_ERASE_FAIL;
	}
#endif /* CONFIG_SIDEWALK_CRYPTO_PSA_KEY_STORAGE */

	char serial[STORAGE_SERIAL_SIZE] = { 0 };
	settings_serialize_group_key(serial, sizeof(serial), group, key);
	int rc = settings_delete(serial);
	if (rc == 0) {
		return SID_ERROR_NONE;
	}
	LOG_ERR("Failed to delete record (%s). Returned errno %d", serial, rc);
	return SID_ERROR_GENERIC;
}

int delete_subtree_cb(const char *key, size_t len, settings_read_cb read_cb, void *cb_arg,
		      void *param)
{
	char *subtree = (char *)param;
	char serial[STORAGE_SERIAL_SIZE] = { 0 };
	snprintf(serial, sizeof(serial), "%s/%s", subtree, key);
	int rc = settings_delete(serial);
	if (rc != 0) {
		LOG_ERR("Failed to delete record. Returned errno %d", rc);
		return rc;
	}
	return 0;
}

sid_error_t sid_pal_storage_kv_group_delete(uint16_t group)
{
	char serial[STORAGE_SERIAL_SIZE] = { 0 };
	settings_serialize_group(serial, sizeof(serial), group);
	int rc = settings_load_subtree_direct(serial, delete_subtree_cb, (void *)serial);
	if (rc != 0) {
		LOG_ERR("Failed to delete group. Returned errno %d", rc);
		return SID_ERROR_STORAGE_ERASE_FAIL;
	}
	rc = settings_commit();
	if (rc != 0) {
		LOG_ERR("Failed to commit changes. Returned errno %d", rc);
		return SID_ERROR_GENERIC;
	}

#ifdef CONFIG_SIDEWALK_CRYPTO_PSA_KEY_STORAGE
	bool success = true;
	if (STORAGE_KV_INTERNAL_PROTOCOL_GROUP_ID == group) {
		success = success && storage_key_delete_secure(SID_CRYPTO_KV_WAN_MASTER_KEY_ID);
		success = success && storage_key_delete_secure(SID_CRYPTO_KV_APP_KEY_KEY_ID);
		success = success && storage_key_delete_secure(SID_CRYPTO_KV_D2D_KEY_ID);
	}
	if (!success) {
		return SID_ERROR_STORAGE_ERASE_FAIL;
	}
#endif /* CONFIG_SIDEWALK_CRYPTO_PSA_KEY_STORAGE */

	return SID_ERROR_NONE;
}
