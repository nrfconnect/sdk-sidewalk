/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/*
 * Registration over Wire (ROW) shell commands.
 *
 * The host tool drives the factory registration with the register commands.
 * All Sidewalk serial registration calls run in the Sidewalk thread.
 * Each command answers with one {REGISTRATION ...} line.
 */

#include <cli/app_dut.h>
#include <cli/row_shell.h>
#include <sidewalk.h>

#include <sid_error.h>
#include <sid_mfg_credentials.h>
#include <sid_mfg_storage.h>
#include <sid_row_uart.h>
#include <sid_serial_registration.h>
#include <sid_state_lite.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/shell/shell.h>
#include <zephyr/sys/atomic.h>

LOG_MODULE_REGISTER(row_shell, CONFIG_SIDEWALK_LOG_LEVEL);

#define REGISTER_H "Serial device registration commands"
#define REGISTER_START_H "Start serial device registration"
#define REGISTER_GET_STATE_H "Print the device registration state"
#define REGISTER_COMMIT_KEYS_H "Persist registration credentials from KV into the MFG page"
#define REGISTER_FINALIZE_H "Finalize registration: tear down the registration stack and wired link"

#define REGISTRATION_MSG_STARTED "{REGISTRATION STARTED}"
#define REGISTRATION_MSG_STATE "{REGISTRATION STATE=%d}"
#define REGISTRATION_MSG_KEYS_COMMITTED "{REGISTRATION KEYS_COMMITTED}"
#define REGISTRATION_MSG_FINALIZED "{REGISTRATION FINALIZED}"
#define REGISTRATION_MSG_ERROR "{REGISTRATION ERROR %d}"

#define ROW_EVENT_TIMEOUT K_SECONDS(5)

struct row_cmd_result {
	sid_error_t err;
	uint8_t state;
};

static atomic_t row_active;
static atomic_t row_process_pending;
static struct row_cmd_result row_result;

K_SEM_DEFINE(row_event_done, 0, 1);

static void row_event_process(sidewalk_ctx_t *sid, void *ctx)
{
	ARG_UNUSED(sid);
	ARG_UNUSED(ctx);

	atomic_clear(&row_process_pending);
	if (atomic_get(&row_active)) {
		sid_serial_registration_process();
	}
}

static void row_request_process(void)
{
	if (!atomic_get(&row_active) || atomic_set(&row_process_pending, 1)) {
		return;
	}
	if (sidewalk_event_send(row_event_process, NULL, NULL)) {
		atomic_clear(&row_process_pending);
	}
}

static void row_poll_timer_handler(struct k_timer *timer)
{
	ARG_UNUSED(timer);
	row_request_process();
}

K_TIMER_DEFINE(row_poll_timer, row_poll_timer_handler, NULL);

static void row_event_done_set(sid_error_t err)
{
	row_result.err = err;
	k_sem_give(&row_event_done);
}

static void row_event_init(sidewalk_ctx_t *sid, void *ctx)
{
	ARG_UNUSED(sid);
	ARG_UNUSED(ctx);

	if (dut_sid_is_initialized()) {
		LOG_ERR("Sidewalk stack is initialized, deinitialize it first");
		row_event_done_set(SID_ERROR_INVALID_STATE);
		return;
	}

	struct serial_ifc_context *ifc = sid_row_uart_get_ifc();
	const struct sid_serial_registration_config cfg = {
		.ifc = ifc,
		.on_registration_done = NULL,
		.ctx = NULL,
	};

	ifc->serial_init();
	sid_error_t err = sid_serial_registration_init(&cfg);

	if (!err) {
		atomic_set(&row_active, 1);
	}
	row_event_done_set(err);
}

static void row_event_start(sidewalk_ctx_t *sid, void *ctx)
{
	ARG_UNUSED(sid);
	ARG_UNUSED(ctx);

	/* A failed start leaves the stack initialized. Only finalize or reset recovers. */
	sid_row_uart_set_frame_ready_cb(row_request_process);
	k_timer_start(&row_poll_timer, K_MSEC(CONFIG_SIDEWALK_ROW_POLL_INTERVAL_MS),
		      K_MSEC(CONFIG_SIDEWALK_ROW_POLL_INTERVAL_MS));

	row_event_done_set(sid_serial_registration_start());
}

static void row_event_get_state(sidewalk_ctx_t *sid, void *ctx)
{
	ARG_UNUSED(sid);
	ARG_UNUSED(ctx);

	row_result.state = sid_state_get_device_state();
	row_event_done_set(SID_ERROR_NONE);
}

static void row_event_commit_keys(sidewalk_ctx_t *sid, void *ctx)
{
	ARG_UNUSED(sid);
	ARG_UNUSED(ctx);

	sid_error_t err = sid_mfg_credentials_commit();

	/* The commit only stages the MFG records in RAM. Write them to flash. */
	if (!err && sid_mfg_storage_flush()) {
		err = SID_ERROR_STORAGE_WRITE_FAIL;
	}
	row_event_done_set(err);
}

static void row_event_finalize(sidewalk_ctx_t *sid, void *ctx)
{
	ARG_UNUSED(sid);
	ARG_UNUSED(ctx);

	sid_error_t err = sid_serial_registration_deinit();

	if (!err) {
		k_timer_stop(&row_poll_timer);
		sid_row_uart_set_frame_ready_cb(NULL);
		atomic_clear(&row_active);
	}
	row_event_done_set(err);
}

static sid_error_t row_run(event_handler_t handler)
{
	k_sem_reset(&row_event_done);

	if (sidewalk_event_send(handler, NULL, NULL)) {
		return SID_ERROR_BUSY;
	}
	if (k_sem_take(&row_event_done, ROW_EVENT_TIMEOUT)) {
		return SID_ERROR_TIMEOUT;
	}

	return row_result.err;
}

static void row_reply(const struct shell *sh, sid_error_t err, const char *done_msg)
{
	if (err) {
		shell_print(sh, REGISTRATION_MSG_ERROR, (int)err);
	} else {
		shell_print(sh, "%s", done_msg);
	}
}

static int cmd_register_start(const struct shell *sh, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	/*
	 * The reply goes out before the first frame: the host reads {REGISTRATION STARTED}
	 * first, then the 0xAA frames that sid_serial_registration_start() sends.
	 */
	sid_error_t err = row_run(row_event_init);

	row_reply(sh, err, REGISTRATION_MSG_STARTED);
	if (err) {
		return 0;
	}

	err = row_run(row_event_start);
	if (err) {
		shell_print(sh, REGISTRATION_MSG_ERROR, (int)err);
	}

	return 0;
}

static int cmd_register_get_state(const struct shell *sh, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	sid_error_t err = row_run(row_event_get_state);

	if (err) {
		shell_print(sh, REGISTRATION_MSG_ERROR, (int)err);
	} else {
		shell_print(sh, REGISTRATION_MSG_STATE, row_result.state);
	}

	return 0;
}

static int cmd_register_commit_keys(const struct shell *sh, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	row_reply(sh, row_run(row_event_commit_keys), REGISTRATION_MSG_KEYS_COMMITTED);

	return 0;
}

static int cmd_register_finalize(const struct shell *sh, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	row_reply(sh, row_run(row_event_finalize), REGISTRATION_MSG_FINALIZED);

	return 0;
}

bool row_shell_is_active(void)
{
	return atomic_get(&row_active);
}

SHELL_STATIC_SUBCMD_SET_CREATE(
	sub_register,
	SHELL_CMD_ARG(start, NULL, REGISTER_START_H, cmd_register_start, 1, 0),
	SHELL_CMD_ARG(get_state, NULL, REGISTER_GET_STATE_H, cmd_register_get_state, 1, 0),
	SHELL_CMD_ARG(commit_keys, NULL, REGISTER_COMMIT_KEYS_H, cmd_register_commit_keys, 1, 0),
	SHELL_CMD_ARG(finalize, NULL, REGISTER_FINALIZE_H, cmd_register_finalize, 1, 0),
	SHELL_SUBCMD_SET_END);

SHELL_CMD_REGISTER(register, &sub_register, REGISTER_H, NULL);
