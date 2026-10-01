/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef EP_CFG_PRINT_H
#define EP_CFG_PRINT_H

#include <stdbool.h>
#include <stdint.h>

struct shell;

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

extern void sid_ep_cfg_get_active_cap(struct sid_ep_cap *cap, bool clear);
extern void sid_ep_cfg_get_active_cfg(struct sid_ep_cfg *cfg, bool clear);

/** @param cap_cfg Bitmask: 1 = capabilities, 2 = configuration, 3 = both. */
int sid_ep_cfg_print(const struct shell *shell, uint32_t cap_cfg, bool clear);

#endif /* EP_CFG_PRINT_H */
