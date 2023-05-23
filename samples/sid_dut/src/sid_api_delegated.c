/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>
#include <sid_api.h>
#include <sid_api_delegated.h>

struct sid_init_args {
	struct k_work work;
	struct k_sem completed;
	sid_error_t return_value;

	const struct sid_config *config;
	struct sid_handle **handle;
};

struct sid_deinit_args {
	struct k_work work;
	struct k_sem completed;
	sid_error_t return_value;

	struct sid_handle *handle;
};

struct sid_start_args {
	struct k_work work;
	struct k_sem completed;
	sid_error_t return_value;

	struct sid_handle *handle;
	uint32_t link_mask;
};
struct sid_stop_args {
	struct k_work work;
	struct k_sem completed;
	sid_error_t return_value;

	struct sid_handle *handle;
	uint32_t link_mask;
};

struct sid_put_msg_args {
	struct k_work work;
	struct k_sem completed;
	sid_error_t return_value;

	struct sid_handle *handle;
	const struct sid_msg *msg;
	struct sid_msg_desc *msg_desc;
};

struct sid_set_factory_reset_args {
	struct k_work work;
	struct k_sem completed;
	sid_error_t return_value;

	struct sid_handle *handle;
};

struct sid_get_mtu_args {
	struct k_work work;
	struct k_sem completed;
	sid_error_t return_value;

	struct sid_handle *handle;
	enum sid_link_type link_type;
	size_t *mtu;
};

struct sid_get_status_args {
	struct k_work work;
	struct k_sem completed;
	sid_error_t return_value;

	struct sid_handle *handle;
	struct sid_status *current_status;
};

struct sid_ble_bcn_connection_request_args {
	struct k_work work;
	struct k_sem completed;
	sid_error_t return_value;

	struct sid_handle *handle;
	bool set;
};

struct sid_get_time_args {
	struct k_work work;
	struct k_sem completed;
	sid_error_t return_value;

	struct sid_handle *handle;
	enum sid_time_format format;
	struct sid_timespec *curr_time;
};

struct sid_set_msg_dest_id_args {
	struct k_work work;
	struct k_sem completed;
	sid_error_t return_value;

	struct sid_handle *handle;
	uint32_t id;
};

struct sid_option_args {
	struct k_work work;
	struct k_sem completed;
	sid_error_t return_value;

	struct sid_handle *handle;
	enum sid_option option;
	void *data;
	size_t len;
};

struct sid_api_ctx {
	struct k_work_q *workq;

	struct sid_init_args sid_init_ctx;
	struct sid_deinit_args sid_deinit_ctx;
	struct sid_start_args sid_start_ctx;
	struct sid_stop_args sid_stop_ctx;
	struct sid_put_msg_args sid_put_msg_ctx;
	struct sid_set_factory_reset_args sid_set_factory_reset_ctx;
	struct sid_get_mtu_args sid_get_mtu_ctx;
	struct sid_get_status_args sid_get_status_ctx;
	struct sid_ble_bcn_connection_request_args sid_ble_bcn_connection_request_ctx;
	struct sid_get_time_args sid_get_time_ctx;
	struct sid_set_msg_dest_id_args sid_set_msg_dest_id_ctx;
	struct sid_option_args sid_option_ctx;
};

static struct sid_api_ctx ctx;

static void sid_init_delegated_work(struct k_work *work)
{
	struct sid_init_args *arguments = CONTAINER_OF(work, struct sid_init_args, work);

	arguments->return_value = sid_init(arguments->config, arguments->handle);
	k_sem_give(&arguments->completed);
}

static void sid_deinit_delegated_work(struct k_work *work)
{
	struct sid_deinit_args *arguments = CONTAINER_OF(work, struct sid_deinit_args, work);

	arguments->return_value = sid_deinit(arguments->handle);
	k_sem_give(&arguments->completed);
}

static void sid_start_delegated_work(struct k_work *work)
{
	struct sid_start_args *arguments = CONTAINER_OF(work, struct sid_start_args, work);

	arguments->return_value = sid_start(arguments->handle, arguments->link_mask);
	k_sem_give(&arguments->completed);
}

static void sid_stop_delegated_work(struct k_work *work)
{
	struct sid_stop_args *arguments = CONTAINER_OF(work, struct sid_stop_args, work);

	arguments->return_value = sid_stop(arguments->handle, arguments->link_mask);
	k_sem_give(&arguments->completed);
}

static void sid_put_msg_delegated_work(struct k_work *work)
{
	struct sid_put_msg_args *arguments = CONTAINER_OF(work, struct sid_put_msg_args, work);

	arguments->return_value =
		sid_put_msg(arguments->handle, arguments->msg, arguments->msg_desc);
	k_sem_give(&arguments->completed);
}

static void sid_set_factory_reset_delegated_work(struct k_work *work)
{
	struct sid_set_factory_reset_args *arguments =
		CONTAINER_OF(work, struct sid_set_factory_reset_args, work);

	arguments->return_value = sid_set_factory_reset(arguments->handle);
	k_sem_give(&arguments->completed);
}

static void sid_get_mtu_delegated_work(struct k_work *work)
{
	struct sid_get_mtu_args *arguments = CONTAINER_OF(work, struct sid_get_mtu_args, work);

	arguments->return_value =
		sid_get_mtu(arguments->handle, arguments->link_type, arguments->mtu);
	k_sem_give(&arguments->completed);
}

static void sid_get_status_delegated_work(struct k_work *work)
{
	struct sid_get_status_args *arguments =
		CONTAINER_OF(work, struct sid_get_status_args, work);

	arguments->return_value = sid_get_status(arguments->handle, arguments->current_status);
	k_sem_give(&arguments->completed);
}

static void sid_ble_bcn_connection_request_delegated_work(struct k_work *work)
{
	struct sid_ble_bcn_connection_request_args *arguments =
		CONTAINER_OF(work, struct sid_ble_bcn_connection_request_args, work);

	arguments->return_value = sid_ble_bcn_connection_request(arguments->handle, arguments->set);
	k_sem_give(&arguments->completed);
}

static void sid_get_time_delegated_work(struct k_work *work)
{
	struct sid_get_time_args *arguments = CONTAINER_OF(work, struct sid_get_time_args, work);

	arguments->return_value =
		sid_get_time(arguments->handle, arguments->format, arguments->curr_time);
	k_sem_give(&arguments->completed);
}

static void sid_set_msg_dest_id_delegated_work(struct k_work *work)
{
	struct sid_set_msg_dest_id_args *arguments =
		CONTAINER_OF(work, struct sid_set_msg_dest_id_args, work);

	arguments->return_value = sid_set_msg_dest_id(arguments->handle, arguments->id);
	k_sem_give(&arguments->completed);
}

static void sid_option_delegated_work(struct k_work *work)
{
	struct sid_option_args *arguments = CONTAINER_OF(work, struct sid_option_args, work);

	arguments->return_value =
		sid_option(arguments->handle, arguments->option, arguments->data, arguments->len);
	k_sem_give(&arguments->completed);
}

///////////////////////////////////////////////////////////////////////////////

sid_error_t sid_init_delegated(const struct sid_config *config, struct sid_handle **handle)
{
	while (k_work_busy_get(&ctx.sid_init_ctx.work) != 0) {
		k_sleep(K_MSEC(1));
	}
	ctx.sid_init_ctx.config = config;
	ctx.sid_init_ctx.handle = handle;
	k_work_submit_to_queue(ctx.workq, &ctx.sid_init_ctx.work);
	k_sem_take(&ctx.sid_init_ctx.completed, K_FOREVER);
	return ctx.sid_init_ctx.return_value;
}

sid_error_t sid_deinit_delegated(struct sid_handle *handle)
{
	while (k_work_busy_get(&ctx.sid_deinit_ctx.work) != 0) {
		k_sleep(K_MSEC(1));
	}

	ctx.sid_deinit_ctx.handle = handle;
	k_work_submit_to_queue(ctx.workq, &ctx.sid_deinit_ctx.work);
	k_sem_take(&ctx.sid_deinit_ctx.completed, K_FOREVER);
	return ctx.sid_deinit_ctx.return_value;
}

sid_error_t sid_start_delegated(struct sid_handle *handle, uint32_t link_mask)
{
	while (k_work_busy_get(&ctx.sid_start_ctx.work) != 0) {
		k_sleep(K_MSEC(1));
	}

	ctx.sid_start_ctx.handle = handle;
	ctx.sid_start_ctx.link_mask = link_mask;
	k_work_submit_to_queue(ctx.workq, &ctx.sid_start_ctx.work);
	k_sem_take(&ctx.sid_start_ctx.completed, K_FOREVER);
	return ctx.sid_start_ctx.return_value;
}

sid_error_t sid_stop_delegated(struct sid_handle *handle, uint32_t link_mask)
{
	while (k_work_busy_get(&ctx.sid_stop_ctx.work) != 0) {
		k_sleep(K_MSEC(1));
	}

	ctx.sid_stop_ctx.handle = handle;
	ctx.sid_stop_ctx.link_mask = link_mask;
	k_work_submit_to_queue(ctx.workq, &ctx.sid_stop_ctx.work);
	k_sem_take(&ctx.sid_stop_ctx.completed, K_FOREVER);
	return ctx.sid_stop_ctx.return_value;
}

sid_error_t sid_put_msg_delegated(struct sid_handle *handle, const struct sid_msg *msg,
				  struct sid_msg_desc *msg_desc)
{
	while (k_work_busy_get(&ctx.sid_put_msg_ctx.work) != 0) {
		k_sleep(K_MSEC(1));
	}

	ctx.sid_put_msg_ctx.handle = handle;
	ctx.sid_put_msg_ctx.msg = msg;
	ctx.sid_put_msg_ctx.msg_desc = msg_desc;
	k_work_submit_to_queue(ctx.workq, &ctx.sid_put_msg_ctx.work);
	k_sem_take(&ctx.sid_put_msg_ctx.completed, K_FOREVER);
	return ctx.sid_put_msg_ctx.return_value;
}

sid_error_t sid_set_factory_reset_delegated(struct sid_handle *handle)
{
	while (k_work_busy_get(&ctx.sid_set_factory_reset_ctx.work) != 0) {
		k_sleep(K_MSEC(1));
	}

	ctx.sid_set_factory_reset_ctx.handle = handle;
	k_work_submit_to_queue(ctx.workq, &ctx.sid_set_factory_reset_ctx.work);
	k_sem_take(&ctx.sid_set_factory_reset_ctx.completed, K_FOREVER);
	return ctx.sid_set_factory_reset_ctx.return_value;
}

sid_error_t sid_get_mtu_delegated(struct sid_handle *handle, enum sid_link_type link_type,
				  size_t *mtu)
{
	while (k_work_busy_get(&ctx.sid_get_mtu_ctx.work) != 0) {
		k_sleep(K_MSEC(1));
	}

	ctx.sid_get_mtu_ctx.handle = handle;
	ctx.sid_get_mtu_ctx.link_type = link_type;
	ctx.sid_get_mtu_ctx.mtu = mtu;
	k_work_submit_to_queue(ctx.workq, &ctx.sid_get_mtu_ctx.work);
	k_sem_take(&ctx.sid_get_mtu_ctx.completed, K_FOREVER);
	return ctx.sid_get_mtu_ctx.return_value;
}

sid_error_t sid_get_status_delegated(struct sid_handle *handle, struct sid_status *current_status)
{
	while (k_work_busy_get(&ctx.sid_get_status_ctx.work) != 0) {
		k_sleep(K_MSEC(1));
	}

	ctx.sid_get_status_ctx.handle = handle;
	ctx.sid_get_status_ctx.current_status = current_status;
	k_work_submit_to_queue(ctx.workq, &ctx.sid_get_status_ctx.work);
	k_sem_take(&ctx.sid_get_status_ctx.completed, K_FOREVER);
	return ctx.sid_get_status_ctx.return_value;
}

sid_error_t sid_ble_bcn_connection_request_delegated(struct sid_handle *handle, bool set)
{
	while (k_work_busy_get(&ctx.sid_ble_bcn_connection_request_ctx.work) != 0) {
		k_sleep(K_MSEC(1));
	}

	ctx.sid_ble_bcn_connection_request_ctx.handle = handle;
	ctx.sid_ble_bcn_connection_request_ctx.set = set;
	k_work_submit_to_queue(ctx.workq, &ctx.sid_ble_bcn_connection_request_ctx.work);
	k_sem_take(&ctx.sid_ble_bcn_connection_request_ctx.completed, K_FOREVER);
	return ctx.sid_ble_bcn_connection_request_ctx.return_value;
}

sid_error_t sid_get_time_delegated(struct sid_handle *handle, enum sid_time_format format,
				   struct sid_timespec *curr_time)
{
	while (k_work_busy_get(&ctx.sid_get_time_ctx.work) != 0) {
		k_sleep(K_MSEC(1));
	}

	ctx.sid_get_time_ctx.handle = handle;
	ctx.sid_get_time_ctx.format = format;
	ctx.sid_get_time_ctx.curr_time = curr_time;
	k_work_submit_to_queue(ctx.workq, &ctx.sid_get_time_ctx.work);
	k_sem_take(&ctx.sid_get_time_ctx.completed, K_FOREVER);
	return ctx.sid_get_time_ctx.return_value;
}

sid_error_t sid_set_msg_dest_id_delegated(struct sid_handle *handle, uint32_t id)
{
	while (k_work_busy_get(&ctx.sid_set_msg_dest_id_ctx.work) != 0) {
		k_sleep(K_MSEC(1));
	}

	ctx.sid_set_msg_dest_id_ctx.handle = handle;
	ctx.sid_set_msg_dest_id_ctx.id = id;
	k_work_submit_to_queue(ctx.workq, &ctx.sid_set_msg_dest_id_ctx.work);
	k_sem_take(&ctx.sid_set_msg_dest_id_ctx.completed, K_FOREVER);
	return ctx.sid_set_msg_dest_id_ctx.return_value;
}

sid_error_t sid_option_delegated(struct sid_handle *handle, enum sid_option option, void *data,
				 size_t len)
{
	while (k_work_busy_get(&ctx.sid_option_ctx.work) != 0) {
		k_sleep(K_MSEC(1));
	}

	ctx.sid_option_ctx.handle = handle;
	ctx.sid_option_ctx.option = option;
	ctx.sid_option_ctx.data = data;
	ctx.sid_option_ctx.len = len;
	k_work_submit_to_queue(ctx.workq, &ctx.sid_option_ctx.work);
	k_sem_take(&ctx.sid_option_ctx.completed, K_FOREVER);
	return ctx.sid_option_ctx.return_value;
}

void sid_api_delegated(struct k_work_q *workq)
{
	ctx.workq = workq;

	k_sem_init(&ctx.sid_init_ctx.completed, 0, 1);
	k_work_init(&ctx.sid_init_ctx.work, sid_init_delegated_work);
	k_sem_init(&ctx.sid_deinit_ctx.completed, 0, 1);
	k_work_init(&ctx.sid_deinit_ctx.work, sid_deinit_delegated_work);
	k_sem_init(&ctx.sid_start_ctx.completed, 0, 1);
	k_work_init(&ctx.sid_start_ctx.work, sid_start_delegated_work);
	k_sem_init(&ctx.sid_stop_ctx.completed, 0, 1);
	k_work_init(&ctx.sid_stop_ctx.work, sid_stop_delegated_work);
	k_sem_init(&ctx.sid_put_msg_ctx.completed, 0, 1);
	k_work_init(&ctx.sid_put_msg_ctx.work, sid_put_msg_delegated_work);
	k_sem_init(&ctx.sid_set_factory_reset_ctx.completed, 0, 1);
	k_work_init(&ctx.sid_set_factory_reset_ctx.work, sid_set_factory_reset_delegated_work);
	k_sem_init(&ctx.sid_get_mtu_ctx.completed, 0, 1);
	k_work_init(&ctx.sid_get_mtu_ctx.work, sid_get_mtu_delegated_work);
	k_sem_init(&ctx.sid_get_status_ctx.completed, 0, 1);
	k_work_init(&ctx.sid_get_status_ctx.work, sid_get_status_delegated_work);
	k_sem_init(&ctx.sid_ble_bcn_connection_request_ctx.completed, 0, 1);
	k_work_init(&ctx.sid_ble_bcn_connection_request_ctx.work,
		    sid_ble_bcn_connection_request_delegated_work);
	k_sem_init(&ctx.sid_get_time_ctx.completed, 0, 1);
	k_work_init(&ctx.sid_get_time_ctx.work, sid_get_time_delegated_work);
	k_sem_init(&ctx.sid_set_msg_dest_id_ctx.completed, 0, 1);
	k_work_init(&ctx.sid_set_msg_dest_id_ctx.work, sid_set_msg_dest_id_delegated_work);
	k_sem_init(&ctx.sid_option_ctx.completed, 0, 1);
	k_work_init(&ctx.sid_option_ctx.work, sid_option_delegated_work);
}
