/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#include <zephyr/kernel.h>
#include <sid_api.h>
#include <sid_api_delegated.h>
#include <window_iterator_macro.h>

static struct k_work_q *get_default_workq(void);

#define CREATE_STRUCT_FIELDS(type, name) type name;
#define X(api_function, ...)						      \
	struct api_function ## _args {					      \
		struct k_work work;					      \
		struct k_sem completed;					      \
		sid_error_t return_value;				      \
									      \
		WINDOW_ITERATOR_SIZE_2(CREATE_STRUCT_FIELDS, (), __VA_ARGS__) \
	};
API_FUNCTIONS
#undef X
#undef CREATE_STRUCT_FIELDS

// create ctx struct
#define X(api_function, ...) \
	struct api_function ## _args api_function ## _ctx;

struct sid_api_ctx {
	struct k_work_q *workq;
	API_FUNCTIONS
};
#undef X
static struct sid_api_ctx ctx;

/**
 * @brief Implement delegated api worker
 *
 * This worker is responsible for calling real api function and passing its arguments to the original caller.
 *
 */
#define WORKER_ARGUMENTS_TO_CALL(type, name) arguments->name
#define X(api_function, ...)											  \
	static void api_function ## _delegated_work(struct k_work *work)					  \
	{													  \
		struct api_function ## _args *arguments = CONTAINER_OF(work, struct api_function ## _args, work); \
		arguments->return_value =									  \
			api_function(WINDOW_ITERATOR_SIZE_2(WORKER_ARGUMENTS_TO_CALL, ( , ), __VA_ARGS__));	  \
		k_sem_give(&arguments->completed);								  \
	}
API_FUNCTIONS
#undef X
#undef WORKER_ARGUMENTS_TO_CALL

/**
 * @brief Implementation of delegated api function call
 * The caller of this function is held by semaphore until the api function is executed
 *
 */
#define ARGUMENT_LIST(type, name) type name
#define SAVE_ARGUMENTS(type, name) call_ctx->name = name;
#define X(api_function, ...)										  \
	sid_error_t api_function ## _delegated(WINDOW_ITERATOR_SIZE_2(ARGUMENT_LIST, ( , ), __VA_ARGS__)) \
	{												  \
		if (ctx.workq == NULL) {								  \
			sid_api_delegated(get_default_workq());						  \
		}											  \
		while (k_work_busy_get(&ctx.api_function ## _ctx.work) != 0) {				  \
			k_sleep(K_MSEC(1));								  \
		}											  \
		struct api_function ## _args *call_ctx = &ctx.api_function ## _ctx;			  \
		WINDOW_ITERATOR_SIZE_2(SAVE_ARGUMENTS, (), __VA_ARGS__)					  \
		k_work_submit_to_queue(ctx.workq, &ctx.api_function ## _ctx.work);			  \
		k_sem_take(&ctx.api_function ## _ctx.completed, K_FOREVER);				  \
		return ctx.api_function ## _ctx.return_value;						  \
	}
API_FUNCTIONS
#undef X
#undef SAVE_ARGUMENTS
#undef ARGUMENT_LIST

/**
 * @brief Register work queue that would run the delegated api calls
 *
 * @param workq work queue to execute the api calls
 */
void sid_api_delegated(struct k_work_q *workq)
{
	ctx.workq = workq;

	#define X(api_function, ...)			       \
	k_sem_init(&ctx.api_function ## _ctx.completed, 0, 1); \
	k_work_init(&ctx.api_function ## _ctx.work, api_function ## _delegated_work);

	API_FUNCTIONS
	#undef X
}

/**
 * @brief Get the default workq object
 *
 * @return struct k_work_q* default work queue for system
 */
static struct k_work_q *get_default_workq(void)
{
	return &k_sys_work_q;
}
