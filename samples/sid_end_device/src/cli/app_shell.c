/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <errno.h>
#include <stdbool.h>

#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>
#include <zephyr/sys/atomic.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/util.h>

#include <ace/cli.h>
#include <sid_asd_cli.h>

#include <cli/app_shell.h>

static char cmd_line[CONFIG_SHELL_CMD_BUFF_SIZE];
static ace_status_t cmd_status;
static atomic_t cmd_pending;
static atomic_t cmd_suspended;
static atomic_t cli_ready;

static K_MUTEX_DEFINE(cmd_mutex);
static K_SEM_DEFINE(qa_wake_sem, 0, 1);
static K_SEM_DEFINE(cmd_done_sem, 0, 1);

static void qa_pwr_meas_enter(void)
{
	atomic_set(&cmd_suspended, true);
}

static void qa_pwr_meas_exit(void)
{
	atomic_set(&cmd_suspended, false);

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

int app_shell_init(void)
{
#ifdef CONFIG_SIDEWALK_TRACE_HEAP
	if (SID_CLI_REGISTER_SUB_COMMAND_SET(sid_nordic_cmds) != SID_ERROR_NONE) {
		return -ENOMEM;
	}
#endif /* CONFIG_SIDEWALK_TRACE_HEAP */

	atomic_set(&cli_ready, true);

	return 0;
}

int app_shell_cmd_forward(const struct shell *sh, size_t argc, char **argv)
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
		k_sem_take(&cmd_done_sem, K_FOREVER);
		err = (int)cmd_status;
	}

	k_mutex_unlock(&cmd_mutex);

	return err;
}

APP_SHELL_FORWARD_CMD_REGISTER(sid, "Sidewalk commands, use \"sid ?\" for help");
APP_SHELL_FORWARD_CMD_REGISTER(reboot, "Reboot the device");
APP_SHELL_FORWARD_CMD_REGISTER(mac, "MAC configuration commands");
APP_SHELL_FORWARD_CMD_REGISTER(print, "Print device information and metrics");
APP_SHELL_FORWARD_CMD_REGISTER(time, "Time synchronization commands");
APP_SHELL_FORWARD_CMD_REGISTER(tx, "Radio transmission commands");
APP_SHELL_FORWARD_CMD_REGISTER(metrics, "Metrics commands");
APP_SHELL_FORWARD_CMD_REGISTER(tpc, "Transmit power control commands");
APP_SHELL_FORWARD_CMD_REGISTER(sbdt, "Sidewalk bulk data transfer commands");
APP_SHELL_FORWARD_CMD_REGISTER(location, "Sidewalk location commands");
APP_SHELL_FORWARD_CMD_REGISTER(dult, "Detecting unwanted location trackers commands");
#ifdef CONFIG_SIDEWALK_SUBGHZ_SUPPORT
APP_SHELL_FORWARD_CMD_REGISTER(gwscan, "Gateway scan commands");
#endif /* CONFIG_SIDEWALK_SUBGHZ_SUPPORT */
