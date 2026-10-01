/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/errno.h>

#include <zephyr/shell/shell.h>

#include <sid_error.h>
#include <sid_sdk_version.h>
#include <sid_pal_mfg_store_ifc.h>
#include <sid_pal_storage_kv_ifc.h>
#include <sid_pal_storage_kv_internal_group_ids.h>
#include <sid_clock_ifc.h>

#include <cli/print_shell.h>
#include <cli/ep_cfg_print.h>
#include <cli/sid_metrics_cli.h>
#include <sidewalk_version.h>

#define CHECK_ARGUMENT_COUNT(argc, required, optional)                                             \
	if ((argc < required) || (argc > (required + optional))) {                                 \
		return -EINVAL;                                                                    \
	}

enum storage_kv_protocol_keys {
	STORAGE_KV_SIDEWALK_ID = 43,
};

#define SID_SEC_DEV_ID_SZ 5

typedef enum {
	SID_SEC_TX_UUID_TIME = 0,
	SID_SEC_TX_UUID_COUNTER,
} sid_sec_tx_uuid_type_t;

typedef struct {
	uint8_t const *dev_id;
	size_t id_size;
	sid_sec_tx_uuid_type_t type;
	size_t tx_uuid_size;
} sid_sec_tx_uuid_params_t;

sid_error_t sid_sec_get_tx_uuid(sid_sec_tx_uuid_params_t *params, uint8_t *tx_uuid, uint32_t *reference,
				uint32_t *intvl_secs);

static void bytes_to_hex_str(char *out, size_t out_len, const uint8_t *data, size_t data_len)
{
	static const char hex[] = "0123456789ABCDEF";

	if (out_len < (data_len * 2 + 1)) {
		return;
	}

	for (size_t i = 0; i < data_len; i++) {
		out[i * 2] = hex[data[i] >> 4];
		out[i * 2 + 1] = hex[data[i] & 0x0f];
	}
	out[data_len * 2] = '\0';
}

static void print_mfg_vals(const struct shell *shell)
{
	uint8_t pr_buf[32];
	uint32_t mfg_version = sid_pal_mfg_store_get_version();

	shell_info(shell, "VERSION: %d", mfg_version);

	uint8_t dev_id[SID_PAL_MFG_STORE_DEVID_SIZE] = {};

	if (sid_pal_mfg_store_dev_id_get(dev_id)) {
		bytes_to_hex_str(pr_buf, sizeof(pr_buf), dev_id, SID_PAL_MFG_STORE_DEVID_SIZE);
		shell_info(shell, "DEVID: 0x%s", pr_buf);
	}

	uint8_t smsn[32] = {};

	sid_pal_mfg_store_read(SID_PAL_MFG_STORE_SMSN, smsn, SID_PAL_MFG_STORE_SMSN_SIZE);
	shell_info(shell, "SMSN");
	shell_hexdump(shell, smsn, sizeof(smsn));
}

SHELL_STATIC_SUBCMD_SET_CREATE(
	sub_print_services,
	SHELL_CMD_ARG(txid, NULL, CMD_PRINT_TXID_DESCRIPTION, cmd_print_txid, CMD_PRINT_TXID_ARG_REQUIRED,
		      CMD_PRINT_TXID_ARG_OPTIONAL),
	SHELL_CMD_ARG(get_cap, NULL, CMD_PRINT_GET_CAP_DESCRIPTION, cmd_print_get_cap,
		      CMD_PRINT_GET_CAP_ARG_REQUIRED, CMD_PRINT_GET_CAP_ARG_OPTIONAL),
	SHELL_CMD_ARG(clear_cap, NULL, CMD_PRINT_CLEAR_CAP_DESCRIPTION, cmd_print_clear_cap,
		      CMD_PRINT_CLEAR_CAP_ARG_REQUIRED, CMD_PRINT_CLEAR_CAP_ARG_OPTIONAL),
	SHELL_CMD_ARG(metrics, NULL, CMD_PRINT_METRICS_DESCRIPTION, cmd_print_metrics,
		      CMD_PRINT_METRICS_ARG_REQUIRED, CMD_PRINT_METRICS_ARG_OPTIONAL),
	SHELL_CMD_ARG(clear_metrics, NULL, CMD_PRINT_CLEAR_METRICS_DESCRIPTION, cmd_print_clear_metrics,
		      CMD_PRINT_CLEAR_METRICS_ARG_REQUIRED, CMD_PRINT_CLEAR_METRICS_ARG_OPTIONAL),
	SHELL_CMD_ARG(mfg, NULL, CMD_PRINT_MFG_DESCRIPTION, cmd_print_mfg, CMD_PRINT_MFG_ARG_REQUIRED,
		      CMD_PRINT_MFG_ARG_OPTIONAL),
	SHELL_CMD_ARG(fwversion, NULL, CMD_PRINT_FWVERSION_DESCRIPTION, cmd_print_fwversion,
		      CMD_PRINT_FWVERSION_ARG_REQUIRED, CMD_PRINT_FWVERSION_ARG_OPTIONAL),
	SHELL_SUBCMD_SET_END);

SHELL_CMD_REGISTER(print, &sub_print_services, "Sidewalk print CLI (QA)", NULL);

int cmd_print_mfg(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_ARGUMENT_COUNT(argc, CMD_PRINT_MFG_ARG_REQUIRED, CMD_PRINT_MFG_ARG_OPTIONAL);
	print_mfg_vals(shell);
	return 0;
}

int cmd_print_fwversion(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_ARGUMENT_COUNT(argc, CMD_PRINT_FWVERSION_ARG_REQUIRED, CMD_PRINT_FWVERSION_ARG_OPTIONAL);

	shell_info(shell, "fw ver. %d.%d.%d-%d (rev. %2x), uasl: %d", SID_SDK_MAJOR_VERSION,
		   SID_SDK_MINOR_VERSION, SID_SDK_PATCH_VERSION, SID_SDK_BUILD_VERSION,
		   (uint8_t)(SID_SDK_BUILD_VERSION & 0xff), 0);
	shell_info(shell, "mac protocol en %d", 1);
	shell_info(shell, "Nordic version: %s", STRINGIFY(APP_BUILD_VERSION));
	return 0;
}

int cmd_print_txid(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_ARGUMENT_COUNT(argc, CMD_PRINT_TXID_ARG_REQUIRED, CMD_PRINT_TXID_ARG_OPTIONAL);

	uint8_t dev_id[SID_PAL_MFG_STORE_DEVID_SIZE] = {};
	sid_error_t ret;

	if (!sid_pal_mfg_store_dev_id_get(dev_id)) {
		ret = sid_pal_storage_kv_record_get(SID_PAL_STORAGE_KV_INTERNAL_PROTOCOL_GROUP_ID,
						      STORAGE_KV_SIDEWALK_ID, dev_id, sizeof(dev_id));
		if (ret != SID_ERROR_NONE) {
			goto error;
		}
	}

	ret = sid_clock_now(SID_CLOCK_SOURCE_NETWORK, NULL, NULL);

	uint8_t tx_id[SID_SEC_DEV_ID_SZ] = {};
	sid_sec_tx_uuid_params_t txid_params = {
		.type = (ret == SID_ERROR_NONE) ? SID_SEC_TX_UUID_TIME : SID_SEC_TX_UUID_COUNTER,
		.dev_id = dev_id,
		.id_size = SID_PAL_MFG_STORE_DEVID_SIZE,
		.tx_uuid_size = SID_PAL_MFG_STORE_DEVID_SIZE,
	};

	ret = sid_sec_get_tx_uuid(&txid_params, tx_id, NULL, NULL);
	if (ret != SID_ERROR_NONE) {
		goto error;
	}

	shell_info(shell, "TXID: 0x%02X%02X%02X%02X%02X", tx_id[0], tx_id[1], tx_id[2], tx_id[3],
		   tx_id[4]);
	return 0;

error:
	shell_error(shell, "TXID print failed: %d", ret);
	return -EIO;
}

int cmd_print_get_cap(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_ARGUMENT_COUNT(argc, CMD_PRINT_GET_CAP_ARG_REQUIRED, CMD_PRINT_GET_CAP_ARG_OPTIONAL);

	uint32_t cap_cfg = strtoul(argv[1], NULL, 0);

	return sid_ep_cfg_print(shell, cap_cfg, false);
}

int cmd_print_clear_cap(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_ARGUMENT_COUNT(argc, CMD_PRINT_CLEAR_CAP_ARG_REQUIRED, CMD_PRINT_CLEAR_CAP_ARG_OPTIONAL);

	uint32_t cap_cfg = strtoul(argv[1], NULL, 0);

	return sid_ep_cfg_print(shell, cap_cfg, true);
}

int cmd_print_metrics(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_ARGUMENT_COUNT(argc, CMD_PRINT_METRICS_ARG_REQUIRED, CMD_PRINT_METRICS_ARG_OPTIONAL);

	enum sid_metrics_category_ids category = (enum sid_metrics_category_ids)strtoul(argv[1], NULL, 0);

	if (sid_metrics_core_cli_print_cat_by_priority(category) != SID_ERROR_NONE) {
		shell_error(shell, "Wrong category ID %d", category);
		return -EINVAL;
	}

	return 0;
}

int cmd_print_clear_metrics(const struct shell *shell, int32_t argc, const char **argv)
{
	CHECK_ARGUMENT_COUNT(argc, CMD_PRINT_CLEAR_METRICS_ARG_REQUIRED, CMD_PRINT_CLEAR_METRICS_ARG_OPTIONAL);

	uint32_t raw_bitmask = UINT32_MAX;

	if (argc == 3) {
		uint32_t metrics_id = strtoul(argv[2], NULL, 0);

		raw_bitmask = (uint32_t)(1 << metrics_id);
	}

	enum sid_metrics_category_ids category = (enum sid_metrics_category_ids)strtoul(argv[1], NULL, 0);

	if (sid_metrics_core_cli_execute_action(SID_METRICS_ACTION_CLEAR, category, raw_bitmask) !=
	    SID_ERROR_NONE) {
		return -EINVAL;
	}

	return 0;
}
