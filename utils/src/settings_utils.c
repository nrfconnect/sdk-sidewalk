/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <settings_utils.h>

#include <stdlib.h>
#include <zephyr/settings/settings.h>
#include <errno.h>
#include <zephyr/kernel.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(settings_utils, CONFIG_SIDEWALK_LOG_LEVEL);

/**
 * Structure for immediate request from settings
 * 
 */
struct direct_immediate_value {
	size_t dest_capacity; /**< IN */
	void *dest; /**< IN, OUT */

	size_t len; /**< OUT*/
	uint8_t fetched; /**< OUT*/
};

static int direct_loader_immediate_value(const char *name, size_t len, settings_read_cb read_cb,
					 void *cb_arg, void *param)
{
	const char *next;
	size_t name_len;
	int rc;
	struct direct_immediate_value *one_value = (struct direct_immediate_value *)param;

	name_len = settings_name_next(name, &next);

	if (name_len == 0) {
		if (one_value->dest == NULL) {
			// do not read value, just check its length
			one_value->len = len;
			return 0;
		}

		rc = read_cb(cb_arg, one_value->dest, one_value->dest_capacity);
		if (rc >= 0) {
			one_value->fetched = 1;
			one_value->len = rc;
			return 0;
		}

		LOG_ERR("fail (err %d)\n", rc);
		return rc;
	}

	/* Other keys aren't served by the callback.
	 * Return success in order to skip them
	 * and keep storage processing.
	 */
	return 0;
}

int settings_utils_load_immediate_value(const char *name, void *dest, size_t len)
{
	int rc;
	struct direct_immediate_value dov = { 0 };

	dov.fetched = 0;
	dov.dest_capacity = len;
	dov.dest = dest;

	rc = settings_load_subtree_direct(name, direct_loader_immediate_value, (void *)&dov);
	if (rc == 0) {
		if (!dov.fetched) {
			rc = -ENOENT;
		}
		LOG_DBG("loaded %s key", name);
	}

	return dov.len;
}

int settings_utils_get_value_size(const char *name, size_t *len)
{
	int rc;
	struct direct_immediate_value dov = { 0 };

	rc = settings_load_subtree_direct(name, direct_loader_immediate_value, (void *)&dov);
	if (rc == 0) {
		*len = dov.len;
	}

	return rc;
}

#if defined(DEPRECATED_DFU_FLAG_SETTINGS_KEY)
app_start_t application_to_start(void)
{
	settings_subsys_init();
	settings_load();

#if defined(CONFIG_SIDEWALK_DFU_SERVICE_BLE)
	bool dfu_mode = false;

	(void)settings_utils_load_immediate_value(CONFIG_DEPRECATED_DFU_FLAG_SETTINGS_KEY,
						  &dfu_mode, sizeof(dfu_mode));

	if (dfu_mode) {
		dfu_mode = false;
		int rc = settings_save_one(CONFIG_DEPRECATED_DFU_FLAG_SETTINGS_KEY,
					   (const void *)&dfu_mode, sizeof(dfu_mode));
		if (rc) {
			LOG_ERR("Failed to erase DFU flag from persistant storage, Err = %d", rc);
		}
		return DFU_APPLICATION;
	}
#endif /* CONFIG_SIDEWALK_DFU_SERVICE_BLE */
	return SIDEWALK_APPLICATION;
}
#endif /* DEPRECATED_DFU_FLAG_SETTINGS_KEY */

#ifdef CONFIG_PERSISTENT_LINK_MASK_SETTINGS_KEY

int settings_utils_link_mask_get(uint32_t *link_mask)
{
	settings_subsys_init();
	settings_load();

	return settings_utils_load_immediate_value(CONFIG_PERSISTENT_LINK_MASK_SETTINGS_KEY,
						   link_mask, sizeof(link_mask));
}

int settings_utils_link_mask_set(uint32_t link_mask)
{
	int ret = settings_save_one(CONFIG_PERSISTENT_LINK_MASK_SETTINGS_KEY,
				    (const void *)&link_mask, sizeof(link_mask));
	settings_commit();
	return ret;
}
#endif /* CONFIG_PERSISTENT_LINK_MASK_SETTINGS_KEY */
