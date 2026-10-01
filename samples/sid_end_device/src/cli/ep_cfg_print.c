/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <cli/ep_cfg_print.h>

#include <zephyr/shell/shell.h>

static void print_ep_cap(const struct shell *shell, const struct sid_ep_cap *cap)
{
	shell_info(shell, "Capability version %d", cap->version);
	shell_info(shell, "SDK version %d.%d.%d", cap->sdk_version >> 10,
		   ((cap->sdk_version & 0x03F0) >> 4), cap->sdk_version & 0x000F);
	shell_info(shell, "links enabled 0x%02x", cap->links_enabled);
	shell_info(shell, "Links enabled BLE %d FSK %d LoRa %d",
		   ((cap->links_enabled & 0x01) == 0x01), ((cap->links_enabled & 0x02) == 0x02),
		   ((cap->links_enabled & 0x04) == 0x04));
	shell_info(shell, "features_support 0x%04x", cap->features_support);
	shell_info(
		shell,
		"Features support Static device %d Mobile Device %d Battery powered %d Line Powered %d ffs over fsk %d metrics enabled %d",
		((cap->features_support & 0x1) == 0x01), ((cap->features_support & 0x2) == 0x02),
		((cap->features_support & 0x4) == 0x04), ((cap->features_support & 0x8) == 0x08),
		((cap->features_support & 0x10) == 0x10), ((cap->features_support & 0x20) == 0x20));
	shell_info(
		shell,
		"Features support Coverage test %d Lora low latency %d Auto connect %d MLM %d SBDT %d Traffic throttling enabled %d",
		((cap->features_support & 0x40) == 0x40), ((cap->features_support & 0x80) == 0x80),
		((cap->features_support & 0x100) == 0x100), ((cap->features_support & 0x200) == 0x200),
		((cap->features_support & 0x400) == 0x400), ((cap->features_support & 0x800)) == 0x800);
	shell_info(shell,
		   "Features support Capabilities lite enabled %d Metrics lite enabled %d DULT %d",
		   ((cap->features_support & 0x1000) == 0x1000),
		   ((cap->features_support & 0x2000) == 0x2000),
		   ((cap->features_support & 0x4000) == 0x4000));
	shell_info(shell, "Qualification id 0x%04x", cap->qualification_id);
	shell_info(shell, "Traffic threshold table id %d", cap->traffic_threshold_id);
	shell_info(shell, "metrics periodicity %d hours", (6 * cap->metrics_periodicity));
	shell_info(shell, "BLE Tx power %d FSK Tx power %d LoRa Tx power %d",
		   ((cap->max_tx_power >> 10) & 0x1F), ((cap->max_tx_power >> 5) & 0x1F),
		   (cap->max_tx_power & 0x1F));
}

static void print_ep_cfg(const struct shell *shell, const struct sid_ep_cfg *cfg)
{
	shell_info(shell, "threshold table id %d", cfg->traffic_thresholds_table.table_id);

	shell_info(shell, "LoRa static normal rate 1 message every %d %s",
		   (cfg->traffic_thresholds_table.lora_static_normal_rate & 0x7F),
		   (cfg->traffic_thresholds_table.lora_static_normal_rate & 0x80) ? "seconds" :
										   "minutes");
	shell_info(shell, "LoRa mobile normal rate 1 message every %d %s",
		   (cfg->traffic_thresholds_table.lora_mobile_normal_rate & 0x7F),
		   (cfg->traffic_thresholds_table.lora_mobile_normal_rate & 0x80) ? "seconds" :
										   "minutes");
	shell_info(shell, "LoRa static burst rate 1 message every %d %s",
		   (cfg->traffic_thresholds_table.lora_static_burst_rate & 0x7F),
		   (cfg->traffic_thresholds_table.lora_static_burst_rate & 0x80) ? "seconds" :
										  "minutes");
	shell_info(shell, "LoRa mobile burst rate 1 message every %d %s",
		   (cfg->traffic_thresholds_table.lora_mobile_burst_rate & 0x7F),
		   (cfg->traffic_thresholds_table.lora_mobile_burst_rate & 0x80) ? "seconds" :
										  "minutes");
	shell_info(shell, "LoRa static max packets per day %d",
		   cfg->traffic_thresholds_table.lora_static_max_packets_per_day);
	shell_info(shell, "LoRa mobile max packets per day %d",
		   cfg->traffic_thresholds_table.lora_mobile_max_packets_per_day);
	shell_info(shell, "FSK max packets per min %d",
		   cfg->traffic_thresholds_table.fsk_min_packets_per_minute);
	shell_info(shell, "FSK max packets per day %d",
		   cfg->traffic_thresholds_table.fsk_min_packets_per_minute * 60 * 24);
	shell_info(shell, "BLE max packets per min %d",
		   cfg->traffic_thresholds_table.ble_min_packets_per_minute);
	shell_info(shell, "BLE max packets per day %d",
		   cfg->traffic_thresholds_table.ble_min_packets_per_minute * 60 * 24);
}

int sid_ep_cfg_print(const struct shell *shell, uint32_t cap_cfg, bool clear)
{
	if (!cap_cfg || cap_cfg > 3) {
		return -EINVAL;
	}

	if (cap_cfg & 0x1) {
		struct sid_ep_cap cap = {};

		sid_ep_cfg_get_active_cap(&cap, clear);
		print_ep_cap(shell, &cap);
	}

	if (cap_cfg & 0x2) {
		struct sid_ep_cfg cfg = {};

		sid_ep_cfg_get_active_cfg(&cfg, clear);
		print_ep_cfg(shell, &cfg);
	}

	return 0;
}
