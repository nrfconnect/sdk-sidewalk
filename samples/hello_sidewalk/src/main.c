/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#include <sid_pal_storage_kv_ifc.h>
#include <sid_pal_log_ifc.h>
#include <sid_pal_mfg_store_ifc.h>
#include <storage/flash_map.h>
#include <sid_pal_assert_ifc.h>
#include <sid_pal_timer_ifc.h>

#include <sid_pal_uptime_ifc.h>
#include <logging/log.h>
#include <zephyr.h>
LOG_MODULE_REGISTER(app, LOG_LEVEL_DBG);

#define MFG_EMPTY_VERSION       0xFFFFFFFFUL

#if !FLASH_AREA_LABEL_EXISTS(mfg_storage)
	#error "Flash partition is not defined for the Sidewalk manufacturing storage!!"
#endif

static const sid_pal_mfg_store_region_t mfg_store_region = {
	.addr_start = (uintptr_t)(FLASH_AREA_OFFSET(mfg_storage)),
	.addr_end = (uintptr_t)(FLASH_AREA_OFFSET(mfg_storage) + FLASH_AREA_SIZE(mfg_storage)),
};

sid_pal_timer_t sidewalk_timer;
char *sidewalk_timer_str = "Hello Sidewalk timer!";

struct sid_timespec sidewalk_timer_duration = { .tv_sec = 5 };
struct sid_timespec sidewalk_timer_period = { .tv_sec = 10 };

void sidewalk_timer_cb(void *arg, sid_pal_timer_t *originator)
{
	LOG_INF("sidewalk_timer_cb says \"%s\"\n", (char *)arg);
}

void main(void)
{
	uint32_t mfg_ver;
	sid_error_t erc;

	LOG_INF("Hello Sidewalk World! %s\n", CONFIG_BOARD);

	if (SID_ERROR_NONE == sid_pal_storage_kv_init()) {
		LOG_INF("NVM memory is ready!\n");
	}

	SID_PAL_LOG_INFO("Hello Sidewalk Log! %d", 2022);

	sid_pal_mfg_store_init(mfg_store_region);
	mfg_ver = sid_pal_mfg_store_get_version();
	if (MFG_EMPTY_VERSION == mfg_ver) {
		SID_PAL_LOG_INFO("Mfg storage is not provisioned!");
	} else {
		uint8_t dev_id[SID_PAL_MFG_STORE_DEVID_SIZE];
		SID_PAL_LOG_INFO("Mfg storage ver: 0x%04x", mfg_ver);
		sid_pal_mfg_store_dev_id_get(dev_id);
		SID_PAL_LOG_INFO("Mfg dev ID:");
		SID_PAL_HEXDUMP(SID_PAL_LOG_SEVERITY_INFO, dev_id, sizeof(dev_id));
	}

	erc = sid_pal_timer_init(&sidewalk_timer, sidewalk_timer_cb, sidewalk_timer_str);
	if (SID_ERROR_NONE == erc) {
		erc = sid_pal_timer_arm(&sidewalk_timer, SID_PAL_TIMER_PRIO_CLASS_LOWPOWER,
					&sidewalk_timer_duration, &sidewalk_timer_period);
		if (SID_ERROR_NONE == erc) {
			SID_PAL_LOG_INFO("Timer is started and %sarmed!", sid_pal_timer_is_armed(&sidewalk_timer) ? "" : "dis");
		} else {
			SID_PAL_LOG_WARNING("Timer cannot be armed [erc=%d].", erc);
		}
	}else{
		SID_PAL_LOG_WARNING("Timer initialization error=%d.", erc);
	}

	k_sleep(K_SECONDS(3));
	struct sid_timespec sid_time;
	sid_error_t rc = sid_pal_uptime_now(&sid_time);
	if (SID_ERROR_NONE == rc) {
		SID_PAL_LOG_INFO("Uptime sec: %u nsec:%u", sid_time.tv_sec, sid_time.tv_nsec);
	} else {
		SID_PAL_LOG_ERROR("Uptime fail, error code: %d", rc);
	}

	SID_PAL_ASSERT(true);
}
