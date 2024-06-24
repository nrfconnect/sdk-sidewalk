/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <sid_on_dev_cert.h>
#include <sid_base64.h>
#include <sid_hal_memory_ifc.h>

#include <zephyr/shell/shell.h>
#include <zephyr/logging/log.h>

#include <string.h>

LOG_MODULE_REGISTER(sid_dev_cert_shell, CONFIG_SIDEWALK_LOG_LEVEL);

#ifdef SYNTAX_ERR
#undef SYNTAX_ERR
#endif
#define SYNTAX_ERR "Syntax err \r\n"

#define CERT_INIT_H "Initialization of On-Device Certificate Generation library"
#define CERT_DEINIT_H "Deinitialization of On-Device Certificate Generation Library"
#define CERT_SMSN_H "Generate Sidewalk Manufacturing Serial Number (SMSN)"
#define CERT_CSR_H "Generate Certificate Signing Request (CSR)"
#define CERT_CHAIN_H "Write Sidewalk Certificate Chain"
#define CERT_CHAIN_START_H "Start writing of Sidewalk Certificate Chain"
#define CERT_CHAIN_WRITE_H "Write data fragment to Sidewalk Certificate Chain"
#define CERT_CHAIN_COMMIT_H "Commit previously writed Sidewalk Certificate Chain"
#define CERT_APPKEY_H "Write application server ED25519 public key"
#define CERT_APPKEY_START_H "Start writing of app server ED25519 public key"
#define CERT_APPKEY_WRITE_H "Write data fragment to app server ED25519 public key"
#define CERT_APPKEY_COMMIT_H "Commit previously writed app server ED25519 public key"
#define CERT_STORE_H "Verify and store Sidewalk certificates"

#define CERT_INIT_CMD "cert init"
#define CERT_DEINIT_CMD "cert deinit"
#define CERT_SMSN_CMD "cert smsn <device_type> <dsn> <apid> [<board_id>]"
#define CERT_CSR_CMD "cert csr <curve>"
#define CERT_CHAIN_START_CMD "cert chain start <curve>"
#define CERT_CHAIN_WRITE_CMD "cert chain write <base64string>"
#define CERT_CHAIN_COMMIT_CMD "cert chain commit"
#define CERT_APPKEY_START_CMD "cert app_key start"
#define CERT_APPKEY_WRITE_CMD "cert app_key write <base64string>"
#define CERT_APPKEY_COMMIT_CMD "cert app_key commit"
#define CERT_STORE_CMD "cert store"

#define CERT_MSG_OK "{CERT OK}"
#define CERT_MSG_ERROR "{CERT ERROR %d}"
#define CERT_MSG_BASE64 "{CERT <%s>}"
#define CERT_MSG_BASE64_MTU "{CERT BASE64_MTU=%d}"
#define CERT_ED25519_STR "ed25519"
#define CERT_P256R1_STR "p256r1"
#define CERT_DEV_TYPE_SUFFIX "-PRODUCTION"

#define CERT_BASE64_FRAGMENT_MAX_SIZE 32

static uint8_t cert_app_key[SID_ODC_ED25519_PUK_SIZE];
static uint8_t cert_chain_buffer[SID_ODC_SCC_MAX_SIZE];
static struct sid_on_dev_cert_chain_params cert_chain_params = {
	.cert_chain = NULL,
	.cert_chain_size = 0,
};

// Persistent state for receiving the encoded certificate chain in multiple fragments
static struct sid_base64_ctx cert_chain_base64_ctx = {
	.next_out = NULL,
	.avail_out = 0,
	.total_out = 0,
	.state = 0,
};

static struct sid_base64_ctx cert_app_key_base64_ctx = {
	.next_out = NULL,
	.avail_out = 0,
	.total_out = 0,
	.state = 0,
};

static sid_error_t sid_on_dev_cert_cli_print_base64(const struct shell *shell, const uint8_t *input,
						    size_t input_size)
{
	struct sid_base64_ctx ctx;
	sid_base64_init(&ctx);
	uint8_t out[CERT_BASE64_FRAGMENT_MAX_SIZE + 1]; // allow extra byte for null terminator

	ctx.next_in = input;
	ctx.avail_in = input_size;
	ctx.next_out = out;
	ctx.avail_out = CERT_BASE64_FRAGMENT_MAX_SIZE;

	while (true) {
		sid_error_t status = sid_base64_encode(&ctx, true);
		if (status == SID_ERROR_NONE || status == SID_ERROR_BUFFER_OVERFLOW) {
			// add a null terminator
			*ctx.next_out = '\0';

			shell_info(shell, CERT_MSG_BASE64, out);

			if (status == SID_ERROR_NONE) {
				break;
			}
			// Reset the output buffer for the next line
			ctx.next_out = out;
			ctx.avail_out = CERT_BASE64_FRAGMENT_MAX_SIZE;
		} else {
			return SID_ERROR_GENERIC;
		}
	}
	return SID_ERROR_NONE;
}

static int sid_on_dev_cert_cli_init_cmd(const struct shell *shell, int32_t argc, const char **argv)
{
	if (argc == 1) {
		sid_error_t ret = sid_on_dev_cert_init();

		if (ret == SID_ERROR_NONE) {
			shell_info(shell, CERT_MSG_BASE64_MTU, CERT_BASE64_FRAGMENT_MAX_SIZE);
			shell_info(shell, CERT_MSG_OK);
		} else {
			shell_info(shell, CERT_MSG_ERROR, ret);
		}

	} else {
		shell_error(shell, SYNTAX_ERR);
		shell_warn(shell, CERT_INIT_CMD);
	}
	return 0;
}

static int sid_on_dev_cert_cli_deinit(const struct shell *shell, int32_t argc, const char **argv)
{
	if (argc == 1) {
		sid_on_dev_cert_deinit();

		shell_info(shell, CERT_MSG_OK);

	} else {
		shell_error(shell, SYNTAX_ERR);
		shell_warn(shell, CERT_DEINIT_CMD);
	}
	return 0;
}

static int sid_on_dev_cert_cli_smsn(const struct shell *shell, int32_t argc, const char **argv)
{
	// command format: cert smsn <device_type> <dsn> <apid> [<board_id>]
	if (argc >= 4 && argc <= 5) {
		sid_error_t ret = SID_ERROR_NONE;
		// Note on device-type. It gets transformed into "Amazon-id" which is <device-type>-PRODUCTION
		char *dev_type_production =
			sid_hal_malloc(strlen(argv[1]) + strlen(CERT_DEV_TYPE_SUFFIX) + 1);
		if (!dev_type_production) {
			ret = SID_ERROR_OOM;
			goto exit;
		}
		dev_type_production[0] = 0;
		strcat(dev_type_production, argv[1]);
		strcat(dev_type_production, CERT_DEV_TYPE_SUFFIX);

		const struct sid_on_dev_cert_info dev_info = {
			.dev_type = dev_type_production,
			.dsn = argv[2],
			.apid = argv[3],
			.board_id = argc == 5 ? argv[4] : NULL,
		};

		uint8_t smsn[SID_ODC_SMSN_SIZE];
		memset(smsn, 0xFF, SID_ODC_SMSN_SIZE);
		if ((ret = sid_on_dev_cert_generate_smsn(&dev_info, smsn)) == SID_ERROR_NONE) {
			ret = sid_on_dev_cert_cli_print_base64(shell, smsn, SID_ODC_SMSN_SIZE);
		}

		sid_hal_free(dev_type_production);
	exit:
		if (ret == SID_ERROR_NONE) {
			shell_info(shell, CERT_MSG_OK);
		} else {
			shell_info(shell, CERT_MSG_ERROR, ret);
		}

	} else {
		shell_error(shell, SYNTAX_ERR);
		shell_warn(shell, CERT_SMSN_CMD);
	}
	return 0;
}

static int sid_on_dev_cert_cli_csr(const struct shell *shell, int32_t argc, const char **argv)
{
	if (argc == 2) {
		sid_error_t ret = SID_ERROR_NONE;
		uint8_t csr[SID_ODC_CSR_MAX_SIZE];
		size_t csr_size = SID_ODC_CSR_MAX_SIZE;
		enum sid_on_dev_cert_algo_type algo;

		if (strcmp(argv[1], CERT_ED25519_STR) == 0) {
			algo = SID_ODC_CRYPT_ALGO_ED25519;
		} else if (strcmp(argv[1], CERT_P256R1_STR) == 0) {
			algo = SID_ODC_CRYPT_ALGO_P256R1;
		} else {
			ret = SID_ERROR_INVALID_ARGS;
		}

		if (ret == SID_ERROR_NONE &&
		    (ret = sid_on_dev_cert_generate_csr(algo, csr, &csr_size)) == SID_ERROR_NONE &&
		    (ret = sid_on_dev_cert_cli_print_base64(shell, csr, csr_size)) ==
			    SID_ERROR_NONE) {
			shell_info(shell, CERT_MSG_OK);
		} else {
			shell_info(shell, CERT_MSG_ERROR, ret);
		}

	} else {
		shell_error(shell, SYNTAX_ERR);
		shell_warn(shell, CERT_CSR_CMD);
	}
	return 0;
}

static int sid_on_dev_cert_cli_chain_start(const struct shell *shell, int32_t argc,
					   const char **argv)
{
	if (argc == 2) {
		sid_error_t ret = SID_ERROR_NONE;
		enum sid_on_dev_cert_algo_type algo;

		if (strcmp(argv[1], CERT_ED25519_STR) == 0) {
			algo = SID_ODC_CRYPT_ALGO_ED25519;
		} else if (strcmp(argv[1], CERT_P256R1_STR) == 0) {
			algo = SID_ODC_CRYPT_ALGO_P256R1;
		} else {
			ret = SID_ERROR_INVALID_ARGS;
		}
		if (ret == SID_ERROR_NONE) {
			sid_base64_init(&cert_chain_base64_ctx);
			cert_chain_base64_ctx.next_out = cert_chain_buffer;
			cert_chain_base64_ctx.avail_out = (algo == SID_ODC_CRYPT_ALGO_ED25519) ?
								  SID_ODC_ED25519_SCC_MAX_SIZE :
								  SID_ODC_P256R1_SCC_MAX_SIZE;
			cert_chain_params.algo = algo;
			cert_chain_params.cert_chain = cert_chain_buffer;
			cert_chain_params.cert_chain_size = 0;
			shell_info(shell, CERT_MSG_OK);
		} else {
			shell_info(shell, CERT_MSG_ERROR, ret);
		}

	} else {
		shell_error(shell, SYNTAX_ERR);
		shell_warn(shell, CERT_CHAIN_START_CMD);
	}
	return 0;
}

static int sid_on_dev_cert_cli_chain_write(const struct shell *shell, int32_t argc,
					   const char **argv)
{
	if (argc == 2) {
		sid_error_t ret = SID_ERROR_NONE;
		size_t base64_fragment_len = strlen(argv[1]);

		if (base64_fragment_len >= 1 &&
		    base64_fragment_len <= CERT_BASE64_FRAGMENT_MAX_SIZE) {
			if (cert_chain_params.cert_chain) {
				cert_chain_base64_ctx.next_in = (const uint8_t *)argv[1];
				cert_chain_base64_ctx.avail_in = base64_fragment_len;
				ret = sid_base64_decode(&cert_chain_base64_ctx);
				/*
                 * If we have a successful result, we should check:
                 *  - the input buffer should be empty
                 *  - if the output buffer is empty and there are 8 bits or more of temporary data, it is an error
                 */
				if (ret != SID_ERROR_NONE || cert_chain_base64_ctx.avail_in != 0 ||
				    (cert_chain_base64_ctx.avail_out == 0 &&
				     cert_chain_base64_ctx.temp_len >= 8)) {
					// Clear chain pointer. This makes chain uninitialized.
					cert_chain_params.cert_chain = NULL;
					// Reset base64 context
					sid_base64_init(&cert_chain_base64_ctx);
					ret = SID_ERROR_INVALID_ARGS;
				}
			} else {
				ret = SID_ERROR_UNINITIALIZED;
			}
		} else {
			ret = SID_ERROR_PARAM_OUT_OF_RANGE;
		}

		if (ret == SID_ERROR_NONE) {
			shell_info(shell, CERT_MSG_OK);
		} else {
			shell_info(shell, CERT_MSG_ERROR, ret);
		}

	} else {
		shell_error(shell, SYNTAX_ERR);
		shell_warn(shell, CERT_CHAIN_WRITE_CMD);
	}
	return 0;
}

static int sid_on_dev_cert_cli_chain_commit(const struct shell *shell, int32_t argc,
					    const char **argv)
{
	if (argc == 1) {
		sid_error_t ret = SID_ERROR_NONE;
		if (cert_chain_params.cert_chain) {
			cert_chain_params.cert_chain_size = cert_chain_base64_ctx.total_out;
			ret = sid_on_dev_cert_write_cert_chain(&cert_chain_params);
			// Clear chain pointer. This makes chain uninitialized.
			cert_chain_params.cert_chain = NULL;
			// Reset base64 context
			sid_base64_init(&cert_chain_base64_ctx);
		} else {
			ret = SID_ERROR_UNINITIALIZED;
		}
		if (ret == SID_ERROR_NONE) {
			shell_info(shell, CERT_MSG_OK);
		} else {
			shell_info(shell, CERT_MSG_ERROR, ret);
		}

	} else {
		shell_error(shell, SYNTAX_ERR);
		shell_warn(shell, CERT_CHAIN_COMMIT_CMD);
	}
	return 0;
}

static int sid_on_dev_cert_cli_app_key_start(const struct shell *shell, int32_t argc,
					     const char **argv)
{
	if (argc == 1) {
		sid_base64_init(&cert_app_key_base64_ctx);
		cert_app_key_base64_ctx.next_out = cert_app_key;
		cert_app_key_base64_ctx.avail_out = SID_ODC_ED25519_PUK_SIZE;
		shell_info(shell, CERT_MSG_OK);

	} else {
		shell_error(shell, SYNTAX_ERR);
		shell_warn(shell, CERT_APPKEY_START_CMD);
	}
	return 0;
}

static int sid_on_dev_cert_cli_app_key_write(const struct shell *shell, int32_t argc,
					     const char **argv)
{
	if (argc == 2) {
		sid_error_t ret = SID_ERROR_NONE;
		size_t base64_fragment_len = strlen(argv[1]);

		if (base64_fragment_len >= 1 &&
		    base64_fragment_len <= CERT_BASE64_FRAGMENT_MAX_SIZE) {
			if (cert_app_key_base64_ctx.next_out) {
				cert_app_key_base64_ctx.next_in = (const uint8_t *)argv[1];
				cert_app_key_base64_ctx.avail_in = base64_fragment_len;

				ret = sid_base64_decode(&cert_app_key_base64_ctx);
				/*
                 * If we have a successful result, we should check:
                 *  - the input buffer should be empty
                 *  - if the output buffer is empty and there are 8 bits or more of temporary data, it is an error
                 */
				if (ret != SID_ERROR_NONE ||
				    cert_app_key_base64_ctx.avail_in != 0 ||
				    (cert_app_key_base64_ctx.avail_out == 0 &&
				     cert_app_key_base64_ctx.temp_len >= 8)) {
					// Reset base64 context
					sid_base64_init(&cert_app_key_base64_ctx);
					ret = SID_ERROR_INVALID_ARGS;
				}
			} else {
				ret = SID_ERROR_UNINITIALIZED;
			}
		} else {
			ret = SID_ERROR_PARAM_OUT_OF_RANGE;
		}

		if (ret == SID_ERROR_NONE) {
			shell_info(shell, CERT_MSG_OK);
		} else {
			shell_info(shell, CERT_MSG_ERROR, ret);
		}

	} else {
		shell_error(shell, SYNTAX_ERR);
		shell_warn(shell, CERT_APPKEY_WRITE_CMD);
	}
	return 0;
}

static int sid_on_dev_cert_cli_app_key_commit(const struct shell *shell, int32_t argc,
					      const char **argv)
{
	if (argc == 1) {
		sid_error_t ret = SID_ERROR_NONE;

		if (cert_app_key_base64_ctx.total_out == SID_ODC_ED25519_PUK_SIZE &&
		    cert_app_key_base64_ctx.avail_out == 0) {
			ret = sid_on_dev_cert_write_app_server_key((uint8_t *)cert_app_key);
		} else {
			ret = SID_ERROR_UNINITIALIZED;
		}
		// Reset base64 context
		sid_base64_init(&cert_app_key_base64_ctx);

		if (ret == SID_ERROR_NONE) {
			shell_info(shell, CERT_MSG_OK);
		} else {
			shell_info(shell, CERT_MSG_ERROR, ret);
		}

	} else {
		shell_error(shell, SYNTAX_ERR);
		shell_warn(shell, CERT_APPKEY_COMMIT_CMD);
	}
	return 0;
}

static int sid_on_dev_cert_cli_store(const struct shell *shell, int32_t argc, const char **argv)
{
	if (argc == 1) {
		sid_error_t ret = sid_on_dev_cert_verify_and_store();

		if (ret == SID_ERROR_NONE) {
			shell_info(shell, CERT_MSG_OK);
		} else {
			shell_info(shell, CERT_MSG_ERROR, ret);
		}

	} else {
		shell_error(shell, SYNTAX_ERR);
		shell_warn(shell, CERT_STORE_CMD);
	}
	return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(
	sub_chain,
	SHELL_CMD_ARG(start, NULL, CERT_CHAIN_START_H, sid_on_dev_cert_cli_chain_start, 2, 0),
	SHELL_CMD_ARG(write, NULL, CERT_CHAIN_WRITE_H, sid_on_dev_cert_cli_chain_write, 2, 0),
	SHELL_CMD_ARG(commit, NULL, CERT_APPKEY_COMMIT_H, sid_on_dev_cert_cli_chain_commit, 1, 0),
	SHELL_SUBCMD_SET_END);

SHELL_STATIC_SUBCMD_SET_CREATE(
	sub_app_key,
	SHELL_CMD_ARG(start, NULL, CERT_APPKEY_START_H, sid_on_dev_cert_cli_app_key_start, 1, 0),
	SHELL_CMD_ARG(write, NULL, CERT_APPKEY_WRITE_H, sid_on_dev_cert_cli_app_key_write, 2, 0),
	SHELL_CMD_ARG(commit, NULL, CERT_APPKEY_COMMIT_H, sid_on_dev_cert_cli_app_key_commit, 1, 0),
	SHELL_SUBCMD_SET_END);

SHELL_STATIC_SUBCMD_SET_CREATE(
	sub_services, SHELL_CMD_ARG(init, NULL, CERT_INIT_H, sid_on_dev_cert_cli_init_cmd, 1, 0),
	SHELL_CMD_ARG(deinit, NULL, CERT_DEINIT_H, sid_on_dev_cert_cli_deinit, 1, 0),
	SHELL_CMD_ARG(smsn, NULL, CERT_SMSN_H, sid_on_dev_cert_cli_smsn, 4, 1),
	SHELL_CMD_ARG(csr, NULL, CERT_CSR_H, sid_on_dev_cert_cli_csr, 2, 0),
	SHELL_CMD_ARG(chain, &sub_chain, CERT_CHAIN_H, NULL, 1, 0),
	SHELL_CMD_ARG(app_key, &sub_app_key, CERT_APPKEY_H, NULL, 1, 0),
	SHELL_CMD_ARG(store, NULL, CERT_STORE_H, sid_on_dev_cert_cli_store, 1, 0),
	SHELL_SUBCMD_SET_END);

// command, subcommands, help, handler
SHELL_CMD_REGISTER(cert, &sub_services, "sidewalk testing CLI", NULL);
