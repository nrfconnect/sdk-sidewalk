/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <errno.h>
#include <stdbool.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/shell/shell.h>
#include <zephyr/sys/atomic.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/util.h>

#include <ace/cli.h>
#include <sid_asd_cli.h>
#if defined(CONFIG_SIDEWALK_DFU_SERVICE_BLE)
#include <sidewalk_dfu/nordic_dfu.h>
#endif

#include <cli/app_shell.h>

LOG_MODULE_REGISTER(app_shell, CONFIG_SIDEWALK_LOG_LEVEL);

static char cmd_line[CONFIG_SHELL_CMD_BUFF_SIZE];
static ace_status_t cmd_status;
static atomic_t cmd_pending;
static atomic_t cmd_suspended;
static atomic_t pwr_meas_seq;
static atomic_t cli_ready;

static K_MUTEX_DEFINE(cmd_mutex);
static K_SEM_DEFINE(qa_wake_sem, 0, 1);
static K_SEM_DEFINE(cmd_done_sem, 0, 1);

static void qa_pwr_meas_enter(void)
{
	atomic_set(&cmd_suspended, true);
	atomic_inc(&pwr_meas_seq);
}

static void qa_pwr_meas_exit(void)
{
	atomic_set(&cmd_suspended, false);
	atomic_inc(&pwr_meas_seq);

	if (atomic_get(&cmd_pending)) {
		k_sem_give(&qa_wake_sem);
	}
}

static void qa_blocked(void)
{
	k_sem_take(&qa_wake_sem, K_FOREVER);

	/* The power measurement command runs a nested sid_qa_process() loop, so the CLI must not be re-entered. */
	if (!atomic_get(&cmd_suspended) && atomic_cas(&cmd_pending, true, false)) {
		cmd_status = aceCli_processCmd(cmd_line);
		k_sem_give(&cmd_done_sem);
	}
}

static void qa_unblocked(void)
{
	k_sem_give(&qa_wake_sem);
}

static struct sid_qa_pwr_meas_if qa_ifc = {
	.enter_func = qa_pwr_meas_enter,
	.exit_func = qa_pwr_meas_exit,
	.blocked_func = qa_blocked,
	.unblocked_func = qa_unblocked,
};

struct sid_qa_pwr_meas_if *app_shell_qa_ifc(void)
{
	return &qa_ifc;
}

#ifdef CONFIG_SIDEWALK_TRACE_HEAP
void print_open_buffers(void);

static ace_status_t cmd_sid_heap_stat(int32_t argc, const char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	print_open_buffers();

	return ACE_STATUS_OK;
}

static SID_CLI_REGISTER_COMMAND(sid_nordic_sub_cmds){
	SID_CLI_DEFINE_COMMAND(heap_stat, NULL, "Print heap statistics", cmd_sid_heap_stat),
	SID_CLI_SUBCMD_SET_END,
};

/* Must be registered separately from new root commands, as it is merged with the existing "sid" command. */
static SID_CLI_REGISTER_COMMAND(sid_nordic_cmds){
	SID_CLI_DEFINE_SUB_COMMAND_SET(sid, NULL, "Sidewalk commands", sid_nordic_sub_cmds),
	SID_CLI_SUBCMD_SET_END,
};
#endif /* CONFIG_SIDEWALK_TRACE_HEAP */

#if defined(CONFIG_SIDEWALK_DFU_SERVICE_BLE)
#define CMD_NORDIC_DFU_DESCRIPTION                                                                 \
	"Start BLE SMP service for DFU\n"                                                          \
	"Make sure that sidewalk is deinitialized before use, or the DFU may fail during transfer!"

static ace_status_t cmd_nordic_dfu(int32_t argc, const char **argv)
{
	int err;

	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	LOG_INF("Entering into DFU mode");

	err = nordic_dfu_ble_start();
	if (err) {
		LOG_ERR("dfu start err %d", err);
		return ACE_STATUS_CLI_FUNC_ERROR;
	}

	return ACE_STATUS_OK;
}

static SID_CLI_REGISTER_COMMAND(nordic_dfu_cmds){
	SID_CLI_DEFINE_COMMAND(nordic_dfu, NULL, CMD_NORDIC_DFU_DESCRIPTION, cmd_nordic_dfu),
	SID_CLI_SUBCMD_SET_END,
};
#endif /* CONFIG_SIDEWALK_DFU_SERVICE_BLE */

int app_shell_init(void)
{
#ifdef CONFIG_SIDEWALK_TRACE_HEAP
	if (SID_CLI_REGISTER_SUB_COMMAND_SET(sid_nordic_cmds) != SID_ERROR_NONE) {
		return -ENOMEM;
	}
#endif /* CONFIG_SIDEWALK_TRACE_HEAP */

#if defined(CONFIG_SIDEWALK_DFU_SERVICE_BLE)
	if (SID_CLI_REGISTER_SUB_COMMAND_SET(nordic_dfu_cmds) != SID_ERROR_NONE) {
		return -ENOMEM;
	}
#endif /* CONFIG_SIDEWALK_DFU_SERVICE_BLE */

	atomic_set(&cli_ready, true);

	return 0;
}

static void cmd_wait_done(const struct shell *sh)
{
#if CONFIG_SID_END_DEVICE_CLI_CMD_TIMEOUT > 0
	atomic_val_t seq = atomic_get(&pwr_meas_seq);

	while (k_sem_take(&cmd_done_sem, K_SECONDS(CONFIG_SID_END_DEVICE_CLI_CMD_TIMEOUT)) != 0) {
		atomic_val_t new_seq = atomic_get(&pwr_meas_seq);

		/* Restart the timeout if the power measurement mode was active in the meantime. */
		if (atomic_get(&cmd_suspended) || new_seq != seq) {
			seq = new_seq;
			continue;
		}

		shell_error(sh, "Sidewalk command timed out: %s", cmd_line);
		k_oops();
	}
#else
	ARG_UNUSED(sh);
	k_sem_take(&cmd_done_sem, K_FOREVER);
#endif /* CONFIG_SID_END_DEVICE_CLI_CMD_TIMEOUT > 0 */
}

static int cmd_forward(const struct shell *sh, size_t argc, char **argv)
{
	bool has_args = (argc > 1);
	int len;
	int err;

	if (!atomic_get(&cli_ready)) {
		shell_error(sh, "Sidewalk CLI not initialized");
		return -EAGAIN;
	}

	k_mutex_lock(&cmd_mutex, K_FOREVER);

	len = snprintk(cmd_line, sizeof(cmd_line), "%s%s%s", argv[0], has_args ? " " : "",
		       has_args ? argv[1] : "");

	if ((size_t)len >= sizeof(cmd_line)) {
		shell_error(sh, "Command too long");
		err = -ENOBUFS;
	} else {
		atomic_set(&cmd_pending, true);
		k_sem_give(&qa_wake_sem);
		cmd_wait_done(sh);
		err = (int)cmd_status;
	}

	k_mutex_unlock(&cmd_mutex);

	return err;
}

/* Raw arguments, so that the command line is forwarded unchanged. */
#define FORWARD_CMD_REGISTER(name, help)                                                           \
	SHELL_CMD_ARG_REGISTER(name, NULL, help, cmd_forward, 1, SHELL_OPT_ARG_RAW)

FORWARD_CMD_REGISTER(sid, "Sidewalk commands, use \"sid ?\" for help");
FORWARD_CMD_REGISTER(reboot, "Reboot the device");
FORWARD_CMD_REGISTER(mac, "MAC configuration commands");
FORWARD_CMD_REGISTER(print, "Print device information and metrics");
FORWARD_CMD_REGISTER(time, "Time synchronization commands");
FORWARD_CMD_REGISTER(tx, "Radio transmission commands");
FORWARD_CMD_REGISTER(metrics, "Metrics commands");
FORWARD_CMD_REGISTER(tpc, "Transmit power control commands");
FORWARD_CMD_REGISTER(sbdt, "Sidewalk bulk data transfer commands");
FORWARD_CMD_REGISTER(location, "Sidewalk location commands");
FORWARD_CMD_REGISTER(dult, "Detecting unwanted location trackers commands");
#ifdef CONFIG_SIDEWALK_SUBGHZ_SUPPORT
FORWARD_CMD_REGISTER(gwscan, "Gateway scan commands");
#endif /* CONFIG_SIDEWALK_SUBGHZ_SUPPORT */
#if defined(CONFIG_SIDEWALK_DFU_SERVICE_BLE)
FORWARD_CMD_REGISTER(nordic_dfu, CMD_NORDIC_DFU_DESCRIPTION);
#endif /* CONFIG_SIDEWALK_DFU_SERVICE_BLE */
