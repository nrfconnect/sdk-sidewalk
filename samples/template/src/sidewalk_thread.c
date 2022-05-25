/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#include <sidewalk_thread.h>

#include <sid_pal_crypto_ifc.h>
#include <sid_pal_storage_kv_ifc.h>
#include <sid_pal_mfg_store_ifc.h>

#include <zephyr/kernel.h>
#include <storage/flash_map.h>
#include <logging/log.h>
LOG_MODULE_REGISTER(sid_thread, CONFIG_SIDEWALK_LOG_LEVEL);

#if !FLASH_AREA_LABEL_EXISTS(mfg_storage)
	#error "Flash partition is not defined for the Sidewalk manufacturing storage!!"
#endif

/* Kconfig in future */
#define SIDEWALK_THREAD_STACK_SIZE      (512)
#define SIDEWALK_THREAD_PRIORITY        (K_LOWEST_APPLICATION_THREAD_PRIO)
#define SIDEWALK_MSGQ_SIZE              (4)

K_MSGQ_DEFINE(sid_msgq, sizeof(enum event_type), SIDEWALK_MSGQ_SIZE, 4);
K_THREAD_STACK_DEFINE(sid_stack_area, SIDEWALK_THREAD_STACK_SIZE);
static struct k_thread sid_thread;
static k_tid_t sid_tid;

static void sidewalk_thread(void *u1, void *u2, void *u3)
{
	ARG_UNUSED(u1);
	ARG_UNUSED(u2);
	ARG_UNUSED(u3);

	LOG_INF("Starting sidewalk thread ...");

	while (1) {
		enum event_type event;

		if (!k_msgq_get(&sid_msgq, &event, K_FOREVER)) {
			switch (event) {
			case EVENT_TYPE_SIDEWALK:

				break;
			case EVENT_TYPE_SEND_HELLO:

				break;
			case EVENT_TYPE_SET_BATTERY_LEVEL:

				break;
			case EVENT_TYPE_FACTORY_RESET:

				break;
			case EVENT_TYPE_CONNECTION_REQUEST:

				break;
			default: break;
			}
		}
	}
}

static sid_error_t sid_pal_init(void)
{
	sid_error_t ret_code;

	ret_code = sid_pal_storage_kv_init();
	if (SID_ERROR_NONE != ret_code) {
		LOG_ERR("Sidewalk KV store init failed, err: %d", ret_code);
		return ret_code;
	}

	ret_code = sid_pal_crypto_init();
	if (SID_ERROR_NONE != ret_code) {
		LOG_ERR("Sidewalk Init Crypto HAL, err: %d", ret_code);
		return ret_code;
	}

	static const sid_pal_mfg_store_region_t mfg_store_region = {
		.addr_start = (uintptr_t)(FLASH_AREA_OFFSET(mfg_storage)),
		.addr_end = (uintptr_t)(FLASH_AREA_OFFSET(mfg_storage) + FLASH_AREA_SIZE(mfg_storage)),
	};
	sid_pal_mfg_store_init(mfg_store_region);

	return SID_ERROR_NONE;
}

void sidewalk_thread_message_q_write(enum event_type event)
{
	while (0 != k_msgq_put(&sid_msgq, &event, K_NO_WAIT)) {
		k_msgq_purge(&sid_msgq);
	}
}

void sidewalk_thread_enable(void)
{
    if (SID_ERROR_NONE != sid_pal_init()) {
		return;
	}

	sid_tid = k_thread_create(&sid_thread, sid_stack_area,
				  K_THREAD_STACK_SIZEOF(sid_stack_area),
				  sidewalk_thread, NULL, NULL, NULL,
				  SIDEWALK_THREAD_PRIORITY, 0, K_NO_WAIT);
	k_thread_name_set(&sid_thread, "sidewalk");
}
