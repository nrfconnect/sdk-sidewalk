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

struct direct_immediate_value {
	size_t len;
	void *dest;
	uint8_t fetched;
};

static int direct_loader_immediate_value(const char *name, size_t len,
					 settings_read_cb read_cb, void *cb_arg,
					 void *param)
{
	const char *next;
	size_t name_len;
	int rc;
	struct direct_immediate_value *one_value =
		(struct direct_immediate_value *)param;

	name_len = settings_name_next(name, &next);

	if (name_len == 0) {
		if (len == one_value->len) {
			rc = read_cb(cb_arg, one_value->dest, len);
			if (rc >= 0) {
				one_value->fetched = 1;
				LOG_INF("immediate load: OK.\n");
				return 0;
			}

			LOG_ERR("fail (err %d)\n", rc);
			return rc;
		}
		return -EINVAL;
	}

	/* Other keys aren't served by the callback.
	 * Return success in order to skip them
	 * and keep storage processing.
	 */
	return 0;
}

int load_immediate_value(const char *name, void *dest, size_t len)
{
	int rc;
	struct direct_immediate_value dov;

	dov.fetched = 0;
	dov.len = len;
	dov.dest = dest;

	rc = settings_load_subtree_direct(name, direct_loader_immediate_value,
					  (void *)&dov);
	if (rc == 0) {
		if (!dov.fetched) {
			rc = -ENOENT;
		}
	}

	return rc;
}

app_start_t application_to_start()
{
	settings_subsys_init();
	settings_load();

	#if defined(CONFIG_SIDEWALK_DFU_SERVICE_BLE)
	bool dfu_mode = false;

	(void) load_immediate_value(CONFIG_DFU_FLAG_SETTINGS_KEY, &dfu_mode, sizeof(dfu_mode));

	if (dfu_mode) {
		dfu_mode = false;
		int rc = settings_save_one(CONFIG_DFU_FLAG_SETTINGS_KEY, (const void *)&dfu_mode,
					   sizeof(dfu_mode));
		if (rc) {
			LOG_ERR("Failed to erase DFU flag from persistant storage, Err = %d", rc);
		}
		return DFU_APPLICATION;
	}
	#endif /* CONFIG_SIDEWALK_DFU_SERVICE_BLE */
	return SIDEWALK_APPLICATION;
}
