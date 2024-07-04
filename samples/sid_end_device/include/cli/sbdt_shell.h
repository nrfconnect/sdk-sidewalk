/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <stdint.h>
#include <zephyr/shell/shell.h>
#include <sid_bulk_data_transfer_api.h>

#define CMD_SBDT_INIT_DESCRIPTION                                                                  \
	"Initialize the sidewalk bulk data stack, this can only be done after sid init is done"
#define CMD_SBDT_INIT_ARG_REQUIRED 1
#define CMD_SBDT_INIT_ARG_OPTIONAL 0

#define CMD_SBDT_DEINIT_DESCRIPTION                                                                \
	"This will deinit bulk data transfer stack, will send cancel for any ongoing bulk data transfers"
#define CMD_SBDT_DEINIT_ARG_REQUIRED 1
#define CMD_SBDT_DEINIT_ARG_OPTIONAL 0

#define CMD_SBDT_CANCEL_DESCRIPTION                                                                \
	"<file_id> <reason>\n"                                                                     \
	"This will send a cancel resp with the given file_id, with the given cancel reason\n"      \
	"       0x0 - None\n"                                                                      \
	"       0x1 - Generic\n"                                                                   \
	"       0x3 - File is too big\n"                                                           \
	"       0x4 - No space for file on the device\n"                                           \
	"       0x5 - Device has low battery\n"                                                    \
	"       0x9 - File verification failed\n"                                                  \
	"       0xB - File already exists\n"                                                       \
	"       0xE - Invalid block size"

#define CMD_SBDT_CANCEL_ARG_REQUIRED 3
#define CMD_SBDT_CANCEL_ARG_OPTIONAL 0

#define CMD_SBDT_CFG_DESCRIPTION                                                                                                                                            \
	" -p -r -fd <finalize resp delay> -fs <finalize response accept> -tr <transfer request accept> -trs <transfer request reject reasons> -br <buffer release delay>\n" \
	"       - This will allow use to manipulate bulk data transfer, by rejecting or accepting transfers or implying successful  verification of file transfer\n"        \
	"        -p will print the existing values for the various cfg variables\n"                                                                                         \
	"        -r will reset the cfg values back to default\n"                                                                                                            \
	"        -fd Will add a delay in responding to file transfer finalize request, it's in seconds\n"                                                                   \
	"        -fs Will allow user set success (0) or failure (1) the file transfer\n"                                                                                    \
	"                0 - Success\n"                                                                                                                                     \
	"                1 - Failure\n"                                                                                                                                     \
	"        -tr Will allow user to accept (0) or reject (1) the upcoming file transfer\n"                                                                              \
	"                0 - Accept\n"                                                                                                                                      \
	"                1 - Reject\n"                                                                                                                                      \
	"        -trs Will allow user to set a reject reason\n"                                                                                                             \
	"                0x0 - None\n"                                                                                                                                      \
	"                0x1 - Generic\n"                                                                                                                                   \
	"                0x3 - File is too big\n"                                                                                                                           \
	"                0x4 - No space for file on the device\n"                                                                                                           \
	"                0x5 - Device has low battery\n"                                                                                                                    \
	"                0x9 - File verification failed\n"                                                                                                                  \
	"                0xB - File already exists\n"                                                                                                                       \
	"                0xE - Invalid block size\n"                                                                                                                        \
	"	 -br Will add a delay to buffer release in milliseconds\n"
#define CMD_SBDT_CFG_ARG_REQUIRED 1
#define CMD_SBDT_CFG_ARG_OPTIONAL 11

#define CMD_SBDT_STATS_DESCRIPTION                                                                 \
	"<file_id>\n"                                                                              \
	"       - Prints the progress of the transfer as a percent and current file offset"
#define CMD_SBDT_STATS_ARG_REQUIRED 2
#define CMD_SBDT_STATS_ARG_OPTIONAL 0

#define CMD_SBDT_PARAMS_DESCRIPTION                                                                \
	"<file_id>\n"                                                                              \
	"       - Prints the various parameters relevant to file transfer"
#define CMD_SBDT_PARAMS_ARG_REQUIRED 2
#define CMD_SBDT_PARAMS_ARG_OPTIONAL 0

int cmd_sbdt_init(const struct shell *shell, int32_t argc, const char **argv);
int cmd_sbdt_deinit(const struct shell *shell, int32_t argc, const char **argv);
int cmd_sbdt_cancel(const struct shell *shell, int32_t argc, const char **argv);
int cmd_sbdt_cfg(const struct shell *shell, int32_t argc, const char **argv);
int cmd_sbdt_print(const struct shell *shell, int32_t argc, const char **argv);
int cmd_sbdt_stats(const struct shell *shell, int32_t argc, const char **argv);
int cmd_sbdt_params(const struct shell *shell, int32_t argc, const char **argv);

void on_sbdt_transfer_request(const struct sid_bulk_data_transfer_request *const transfer_request,
			      struct sid_bulk_data_transfer_response *const transfer_response,
			      void *context);
void on_sbdt_data_received(const struct sid_bulk_data_transfer_desc *const desc,
			   const struct sid_bulk_data_transfer_buffer *const buffer, void *context);
void on_sbdt_finalize_request(uint32_t file_id, void *context);
void on_sbdt_cancel_request(uint32_t file_id, void *context);
void on_sbdt_error(uint32_t file_id, void *context);
void on_release_scratch_buffer(uint32_t file_id, void *context);

struct sbdt_context {
	struct sid_bulk_data_transfer_event_callbacks ft_callbacks;
	uint32_t finalize_response_delay_s;
	uint32_t release_buffer_delay_ms;
	uint8_t finalize_response_action;
	uint8_t transfer_request_action;
	enum sid_bulk_data_transfer_reject_reason transfer_request_reject_reason;
	bool started_transfer;
};

struct sbdt_file_info {
	bool is_consumed;
	uint16_t current_block_num;
	uint32_t block_size;
	uint32_t crc;
	uint32_t file_id;
	uint32_t file_size;
	uint32_t file_offset;
	size_t minimum_scratch_buffer_size;
	uint32_t file_descriptor_size;
	uint8_t file_descriptor[SID_BULK_DATA_TRANSFER_FILE_DESCRIPTOR_MAX_SIZE];
	uint8_t *scratch;
};

struct sbdt_file_info *get_file_info_by_id(uint32_t file_id);
