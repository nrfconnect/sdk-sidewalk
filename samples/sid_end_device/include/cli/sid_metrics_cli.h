/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef SID_METRICS_CLI_H
#define SID_METRICS_CLI_H

#include <sid_error.h>
#include <stdint.h>

enum sid_metrics_category_ids {
	SID_METRICS_CAT_CONFIG = 0x00,
	SID_METRICS_CAT_LORA_MAC = 0x01,
	SID_METRICS_CAT_LORA_LINK = 0x02,
	SID_METRICS_CAT_FSK_MAC = 0x03,
	SID_METRICS_CAT_FSK_LINK = 0x04,
	SID_METRICS_CAT_BLE_LINK = 0x05,
	SID_METRICS_CAT_TIME_SYNC = 0x06,
	SID_METRICS_CAT_NWK_SYNC = 0x07,
	SID_METRICS_CAT_LMM = 0x08,
	SID_METRICS_CAT_SSM_SEC = 0x09,
	SID_METRICS_CAT_GWD = 0x0a,
	SID_METRICS_CAT_REG_KR = 0x0b,
	SID_METRICS_CAT_FFN = 0x0c,
	SID_METRICS_CAT_SBDT = 0x0d,
	SID_METRICS_CAT_MLM = 0x0e,
	SID_METRICS_CAT_ACM = 0x0f,
	SID_METRICS_CAT_GWS = 0x10,
	SID_METRICS_CAT_DULT = 0x11,
	SID_METRICS_CAT_LOCATION = 0x12,

	SID_METRICS_AMOUNT_CATEGORIES = SID_METRICS_CAT_LOCATION,

	SID_METRICS_CAT_ALL = 0x36,
	SID_METRICS_CAT_LOW = 0x37,
	SID_METRICS_CAT_MEDIUM = 0x38,
	SID_METRICS_CAT_HIGH = 0x39,
};

enum sid_metrics_core_actions {
	SID_METRICS_ACTION_NONE = 0xff,
	SID_METRICS_ACTION_CLEAR = 0,
	SID_METRICS_ACTION_ENABLE = 1,
	SID_METRICS_ACTION_DISABLE = 2,
	SID_METRICS_ACTION_READ = 3,
	SID_METRICS_ACTIONS_AMOUNT = 4,
};

sid_error_t sid_metrics_core_cli_print_cat_by_priority(enum sid_metrics_category_ids category);
sid_error_t sid_metrics_core_cli_execute_action(enum sid_metrics_core_actions action,
						enum sid_metrics_category_ids category,
						uint32_t raw_bitmask);
sid_error_t sid_metrics_core_cli_trigger_report(void);
sid_error_t sid_metrics_core_cli_set_metrics(enum sid_metrics_category_ids category,
					     uint8_t metrics_id, uint32_t val);

#endif /* SID_METRICS_CLI_H */
