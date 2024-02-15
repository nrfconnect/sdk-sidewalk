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

#include <zephyr/logging/log.h>
#include <settings_utils.h>

LOG_MODULE_REGISTER(sid_storage, CONFIG_SIDEWALK_LOG_LEVEL);

sid_error_t sid_pal_storage_kv_init()
{
	int rc = settings_subsys_init();
	if (rc != 0) {
		LOG_ERR("Failed to initialize pal_storage. Returned errno %d", rc);
		return SID_ERROR_GENERIC;
	}
	LOG_DBG("Initialized KV storage");
	return SID_ERROR_NONE;
}

static void settings_serialize_group(char *serial, size_t serial_size, uint16_t group)
{
	snprintf(serial, serial_size, "sidewalk/storage/%04x", group);
}

static void settings_serialize_group_key(char *serial, size_t serial_size, uint16_t group,
					 uint16_t key)
{
	snprintf(serial, serial_size, "sidewalk/storage/%04x/%04x", group, key);
}

sid_error_t sid_pal_storage_kv_record_get(uint16_t group, uint16_t key, void *p_data, uint32_t len)
{
	if (!p_data) {
		return SID_ERROR_NULL_POINTER;
	}
	char serial[32] = { 0 };
	settings_serialize_group_key(serial, sizeof(serial), group, key);
	int rc = settings_utils_load_immediate_value(serial, p_data, len);
	if (rc <= 0)
		return SID_ERROR_NOT_FOUND;
	else
		return SID_ERROR_NONE;
}

sid_error_t sid_pal_storage_kv_record_get_len(uint16_t group, uint16_t key, uint32_t *p_len)
{
	if (!p_len) {
		return SID_ERROR_NULL_POINTER;
	}
	char serial[32] = { 0 };
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
	char serial[32] = { 0 };
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
	char serial[32] = { 0 };
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
	char serial[32] = { 0 };
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
	char serial[32] = { 0 };
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
	return SID_ERROR_NONE;
}
