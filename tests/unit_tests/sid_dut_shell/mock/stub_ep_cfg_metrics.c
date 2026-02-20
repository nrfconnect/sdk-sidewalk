/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 *
 * Stub implementations for sid_ep_cfg_* and sid_metrics_core_cli_* symbols
 * used by app_shell.c. Required for unit_testing where the full Sidewalk stack
 * is not linked.
 */

#include <stdbool.h>
#include <stdint.h>
#include <sid_api.h>

/* Mirror type definitions from app_shell.c (no public API for these). */
struct sid_ep_cap {
	uint8_t version;
	uint8_t links_enabled;
	uint8_t traffic_threshold_id;
	uint8_t metrics_periodicity;
	uint16_t sdk_version;
	uint16_t max_tx_power;
	uint16_t qualification_id;
	uint32_t features_support;
};

struct sid_ep_cfg_traffic_thresholds {
	uint8_t table_id;
	uint8_t lora_static_normal_rate;
	uint8_t lora_static_burst_rate;
	uint8_t lora_mobile_normal_rate;
	uint8_t lora_mobile_burst_rate;
	uint16_t lora_static_max_packets_per_day;
	uint16_t lora_mobile_max_packets_per_day;
	uint16_t fsk_min_packets_per_minute;
	uint16_t ble_min_packets_per_minute;
};

struct sid_ep_cfg {
	uint8_t metrics_enabled;
	uint8_t metrics_periodicity;
	uint8_t traffic_throttling_enabled;
	uint8_t current_traffic_threshold_table_id;
	uint32_t tag_enabled_mask;
	struct sid_ep_cfg_traffic_thresholds traffic_thresholds_table;
};

enum sid_metrics_category_ids {
	SID_METRICS_CAT_CONFIG = 0x00,
	SID_METRICS_CAT_LOCATION = 0x12,
	SID_METRICS_CAT_ALL = 0x36,
};

enum sid_metrics_core_actions {
	SID_METRICS_ACTION_NONE = 0xff,
	SID_METRICS_ACTION_CLEAR = 0,
};

void sid_ep_cfg_get_active_cap(struct sid_ep_cap *cap, bool clear)
{
	(void)clear;
	if (cap) {
		cap->version = 0;
		cap->links_enabled = 0;
		cap->traffic_threshold_id = 0;
		cap->metrics_periodicity = 0;
		cap->sdk_version = 0;
		cap->max_tx_power = 0;
		cap->qualification_id = 0;
		cap->features_support = 0;
	}
}

void sid_ep_cfg_get_active_cfg(struct sid_ep_cfg *cfg, bool clear)
{
	(void)clear;
	if (cfg) {
		cfg->metrics_enabled = 0;
		cfg->metrics_periodicity = 0;
		cfg->traffic_throttling_enabled = 0;
		cfg->current_traffic_threshold_table_id = 0;
		cfg->tag_enabled_mask = 0;
		cfg->traffic_thresholds_table.table_id = 0;
		cfg->traffic_thresholds_table.lora_static_normal_rate = 0;
		cfg->traffic_thresholds_table.lora_static_burst_rate = 0;
		cfg->traffic_thresholds_table.lora_mobile_normal_rate = 0;
		cfg->traffic_thresholds_table.lora_mobile_burst_rate = 0;
		cfg->traffic_thresholds_table.lora_static_max_packets_per_day = 0;
		cfg->traffic_thresholds_table.lora_mobile_max_packets_per_day = 0;
		cfg->traffic_thresholds_table.fsk_min_packets_per_minute = 0;
		cfg->traffic_thresholds_table.ble_min_packets_per_minute = 0;
	}
}

sid_error_t sid_metrics_core_cli_print_cat_by_priority(enum sid_metrics_category_ids category)
{
	(void)category;
	return SID_ERROR_NONE;
}

sid_error_t sid_metrics_core_cli_execute_action(enum sid_metrics_core_actions action,
					       enum sid_metrics_category_ids category,
					       uint32_t raw_bitmask)
{
	(void)action;
	(void)category;
	(void)raw_bitmask;
	return SID_ERROR_NONE;
}
