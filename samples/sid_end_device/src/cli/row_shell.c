/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/*
 * Registration over Wire (ROW) shell commands.
 *
 * The host tool drives the factory registration with the register commands.
 * All Sidewalk serial registration calls run on a dedicated ROW work queue, so they are
 * serialized with each other. The ROW stack is independent of the Sidewalk stack: do not
 * run the sid commands between register start and register finalize.
 * Each command answers with one {REGISTRATION ...} line.
 */

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
static struct row_cmd_result row_result;

static K_SEM_DEFINE(row_event_done, 0, 1);

static K_THREAD_STACK_DEFINE(row_wq_stack, CONFIG_SIDEWALK_ROW_THREAD_STACK_SIZE);
static struct k_work_q row_wq;

static void row_event_process(struct k_work *work)
{
	ARG_UNUSED(work);

	if (atomic_get(&row_active)) {
		sid_serial_registration_process();
	}
}

static K_WORK_DEFINE(row_process_work, row_event_process);

/* Called from the UART ISR and the poll timer. Coalesced while a process request is queued. */
static void row_request_process(void)
{
	if (atomic_get(&row_active)) {
		(void)k_work_submit_to_queue(&row_wq, &row_process_work);
	}
}

static void row_poll_timer_handler(struct k_timer *timer)
{
	ARG_UNUSED(timer);
	row_request_process();
}

static K_TIMER_DEFINE(row_poll_timer, row_poll_timer_handler, NULL);

static void row_event_done_set(sid_error_t err)
{
	row_result.err = err;
	k_sem_give(&row_event_done);
}

static void row_event_init(struct k_work *work)
{
	ARG_UNUSED(work);

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

static void row_event_start(struct k_work *work)
{
	ARG_UNUSED(work);

	/* A failed start leaves the stack initialized. Only finalize or reset recovers. */
	sid_row_uart_set_frame_ready_cb(row_request_process);
	k_timer_start(&row_poll_timer, K_MSEC(CONFIG_SIDEWALK_ROW_POLL_INTERVAL_MS),
		      K_MSEC(CONFIG_SIDEWALK_ROW_POLL_INTERVAL_MS));

	row_event_done_set(sid_serial_registration_start());
}

static void row_event_get_state(struct k_work *work)
{
	ARG_UNUSED(work);

	row_result.state = sid_state_get_device_state();
	row_event_done_set(SID_ERROR_NONE);
}

static void row_event_commit_keys(struct k_work *work)
{
	ARG_UNUSED(work);

	sid_error_t err = sid_mfg_credentials_commit();

	/* The commit only stages the MFG records in RAM. Write them to flash. */
	if (!err && sid_mfg_storage_flush()) {
		err = SID_ERROR_STORAGE_WRITE_FAIL;
	}
	row_event_done_set(err);
}

static void row_event_finalize(struct k_work *work)
{
	ARG_UNUSED(work);

	sid_error_t err = sid_serial_registration_deinit();

	if (!err) {
		k_timer_stop(&row_poll_timer);
		sid_row_uart_set_frame_ready_cb(NULL);
		atomic_clear(&row_active);
	}
	row_event_done_set(err);
}

static K_WORK_DEFINE(row_init_work, row_event_init);
static K_WORK_DEFINE(row_start_work, row_event_start);
static K_WORK_DEFINE(row_get_state_work, row_event_get_state);
static K_WORK_DEFINE(row_commit_keys_work, row_event_commit_keys);
static K_WORK_DEFINE(row_finalize_work, row_event_finalize);

/* Run the work item on the ROW work queue and wait for its result. */
static sid_error_t row_run(struct k_work *work)
{
	/* A work item still pending after a timeout means the ROW work queue is stuck. */
	if (k_work_busy_get(work)) {
		return SID_ERROR_BUSY;
	}

	k_sem_reset(&row_event_done);

	if (k_work_submit_to_queue(&row_wq, work) < 0) {
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
	sid_error_t err = row_run(&row_init_work);

	row_reply(sh, err, REGISTRATION_MSG_STARTED);
	if (err) {
		return 0;
	}

	err = row_run(&row_start_work);
	if (err) {
		shell_print(sh, REGISTRATION_MSG_ERROR, (int)err);
	}

	return 0;
}

static int cmd_register_get_state(const struct shell *sh, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	sid_error_t err = row_run(&row_get_state_work);

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

	row_reply(sh, row_run(&row_commit_keys_work), REGISTRATION_MSG_KEYS_COMMITTED);

	return 0;
}

static int cmd_register_finalize(const struct shell *sh, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	row_reply(sh, row_run(&row_finalize_work), REGISTRATION_MSG_FINALIZED);

	return 0;
}

static int row_shell_init(void)
{
	const struct k_work_queue_config cfg = {
		.name = "row_wq",
	};

	k_work_queue_start(&row_wq, row_wq_stack, K_THREAD_STACK_SIZEOF(row_wq_stack),
			   CONFIG_SIDEWALK_THREAD_PRIORITY, &cfg);

	return 0;
}

SYS_INIT(row_shell_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);

SHELL_STATIC_SUBCMD_SET_CREATE(
	sub_register,
	SHELL_CMD_ARG(start, NULL, REGISTER_START_H, cmd_register_start, 1, 0),
	SHELL_CMD_ARG(get_state, NULL, REGISTER_GET_STATE_H, cmd_register_get_state, 1, 0),
	SHELL_CMD_ARG(commit_keys, NULL, REGISTER_COMMIT_KEYS_H, cmd_register_commit_keys, 1, 0),
	SHELL_CMD_ARG(finalize, NULL, REGISTER_FINALIZE_H, cmd_register_finalize, 1, 0),
	SHELL_SUBCMD_SET_END);

SHELL_CMD_REGISTER(register, &sub_register, REGISTER_H, NULL);
