/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/ztest.h>
#include <zephyr/fff.h>
#include <zephyr/sys/sys_heap.h>
#include <osal_alloc.h>
#include <ace/ace_status.h>
#include <sid_hal_memory_ifc.h>

DEFINE_FFF_GLOBALS;

FAKE_VALUE_FUNC(void *, sid_hal_malloc, size_t);
FAKE_VOID_FUNC(sid_hal_free, void *);

#ifndef CONFIG_SIDEWALK_HEAP_SIZE
#define CONFIG_SIDEWALK_HEAP_SIZE 1024
#endif

static uint8_t heap_mem[CONFIG_SIDEWALK_HEAP_SIZE];
static struct sys_heap sid_heap;

static void heap_reset(void)
{
	sys_heap_init(&sid_heap, heap_mem, sizeof(heap_mem));
}

static void *fake_sid_hal_malloc(size_t size)
{
	return sys_heap_alloc(&sid_heap, size);
}

static void fake_sid_hal_free(void *ptr)
{
	if (ptr) {
		sys_heap_free(&sid_heap, ptr);
	}
}

static void fakes_reset(void)
{
	RESET_FAKE(sid_hal_malloc);
	RESET_FAKE(sid_hal_free);
	sid_hal_malloc_fake.custom_fake = fake_sid_hal_malloc;
	sid_hal_free_fake.custom_fake = fake_sid_hal_free;
	heap_reset();
}

static void *suite_setup(void)
{
	fakes_reset();
	return NULL;
}

static void before_test(void *fixture)
{
	ARG_UNUSED(fixture);
	fakes_reset();
}

typedef struct {
	uint32_t field32b;
	uint8_t field8b;
} test_struct_t;

static void *mem[CONFIG_SIDEWALK_HEAP_SIZE];

ZTEST(sid_ace_alloc, test_ace_alloc_init_deinit)
{
	zassert_equal(ACE_STATUS_OK, aceAlloc_init());
	zassert_equal(ACE_STATUS_OK, aceAlloc_deInit());
}

ZTEST(sid_ace_alloc, test_ace_alloc_init_with_allocator_not_supported)
{
	aceAlloc_allocator_t *p_allocators = NULL;
	size_t count = 0;

	zassert_equal(ACE_STATUS_NOT_SUPPORTED, aceAlloc_initWithAllocator(p_allocators, count));
}

ZTEST(sid_ace_alloc, test_ace_alloc_and_free)
{
	zassert_equal(ACE_STATUS_OK, aceAlloc_init());

	size_t chunk_size = 16;
	void *p = NULL;

	for (int i = 0; i < 2 * CONFIG_SIDEWALK_HEAP_SIZE; i++) {
		p = aceAlloc_alloc(ACE_MODULE_GROUP, ACE_ALLOC_BUFFER_GENERIC, chunk_size);
		zassert_not_null(p);
		aceAlloc_free(ACE_MODULE_GROUP, ACE_ALLOC_BUFFER_GENERIC, p);
	}

	zassert_equal(ACE_STATUS_OK, aceAlloc_deInit());
}

ZTEST(sid_ace_alloc, test_ace_alloc_free_negative)
{
	zassert_equal(ACE_STATUS_OK, aceAlloc_init());

	aceAlloc_free(ACE_MODULE_GROUP, ACE_ALLOC_BUFFER_GENERIC, NULL);

	zassert_is_null(aceAlloc_alloc(ACE_MODULE_GROUP, ACE_ALLOC_BUFFER_GENERIC, 0));

	zassert_equal(ACE_STATUS_OK, aceAlloc_deInit());
}

ZTEST(sid_ace_alloc, test_ace_alloc_full)
{
	zassert_equal(ACE_STATUS_OK, aceAlloc_init());

	size_t chunk_size = 4;
	int mem_i_max = 0;

	for (mem_i_max = 0; mem_i_max < (CONFIG_SIDEWALK_HEAP_SIZE / chunk_size); mem_i_max++) {
		mem[mem_i_max] =
			aceAlloc_alloc(ACE_MODULE_GROUP, ACE_ALLOC_BUFFER_GENERIC, chunk_size);
		/* Memory heap may end earlier
		 * due to alignment or metadata gaps. */
		if (!mem[mem_i_max]) {
			break;
		}
	}

	zassert_is_null(aceAlloc_alloc(ACE_MODULE_GROUP, ACE_ALLOC_BUFFER_GENERIC, chunk_size));

	for (int i = 0; i < mem_i_max; i++) {
		for (int j = 0; j <= i; j++) {
			aceAlloc_free(ACE_MODULE_GROUP, ACE_ALLOC_BUFFER_GENERIC, mem[j]);
		}

		for (int j = 0; j <= i; j++) {
			mem[j] = aceAlloc_alloc(ACE_MODULE_GROUP, ACE_ALLOC_BUFFER_GENERIC,
						chunk_size);
			zassert_not_null(mem[j]);
		}
	}

	for (int i = mem_i_max - 1; i >= 0; i--) {
		aceAlloc_free(ACE_MODULE_GROUP, ACE_ALLOC_BUFFER_GENERIC, mem[i]);
	}

	zassert_equal(ACE_STATUS_OK, aceAlloc_deInit());
}

ZTEST(sid_ace_alloc, test_ace_calloc_initialize_with_zeros)
{
	zassert_equal(ACE_STATUS_OK, aceAlloc_init());

	size_t chunks_no = 10;
	size_t chunk_size = sizeof(test_struct_t);
	test_struct_t *chunks = NULL;

	chunks = aceAlloc_calloc(ACE_MODULE_GROUP, ACE_ALLOC_BUFFER_GENERIC, chunks_no, chunk_size);
	zassert_not_null(chunks);

	for (int i = 0; i < chunks_no; i++) {
		zassert_not_null(&chunks[i]);
		zassert_equal(0, chunks[i].field32b);
		zassert_equal(0, chunks[i].field8b);
	}

	for (int i = 0; i < chunks_no; i++) {
		chunks[i].field32b = 0xffffffff;
		chunks[i].field8b = 0xff;
	}

	aceAlloc_free(ACE_MODULE_GROUP, ACE_ALLOC_BUFFER_GENERIC, chunks);

	chunks = aceAlloc_calloc(ACE_MODULE_GROUP, ACE_ALLOC_BUFFER_GENERIC, chunks_no, chunk_size);
	zassert_not_null(chunks);

	for (int i = 0; i < chunks_no; i++) {
		zassert_not_null(&chunks[i]);
		zassert_equal(0, chunks[i].field32b);
		zassert_equal(0, chunks[i].field8b);
	}

	aceAlloc_free(ACE_MODULE_GROUP, ACE_ALLOC_BUFFER_GENERIC, chunks);

	zassert_equal(ACE_STATUS_OK, aceAlloc_deInit());
}

ZTEST(sid_ace_alloc, test_ace_calloc_negative)
{
	size_t chunks_no = 1, chunk_size = 1;

	zassert_is_null(aceAlloc_calloc(ACE_MODULE_GROUP, ACE_ALLOC_BUFFER_GENERIC, 0, chunk_size));
	zassert_is_null(aceAlloc_calloc(ACE_MODULE_GROUP, ACE_ALLOC_BUFFER_GENERIC, chunks_no, 0));
	zassert_is_null(aceAlloc_calloc(ACE_MODULE_GROUP, ACE_ALLOC_BUFFER_GENERIC, 0, 0));

	chunk_size = __SIZE_MAX__ - 1;
	chunks_no = 2;
	zassert_is_null(
		aceAlloc_calloc(ACE_MODULE_GROUP, ACE_ALLOC_BUFFER_GENERIC, chunk_size, chunks_no));
}

ZTEST(sid_ace_alloc, test_ace_calloc_full)
{
	zassert_equal(ACE_STATUS_OK, aceAlloc_init());

	size_t chunks_no = 0;
	size_t chunk_size = sizeof(test_struct_t);
	test_struct_t *chunks = NULL;

	chunks_no = ((CONFIG_SIDEWALK_HEAP_SIZE / 2) / sizeof(test_struct_t)) + 1;
	chunks = aceAlloc_calloc(ACE_MODULE_GROUP, ACE_ALLOC_BUFFER_GENERIC, chunks_no, chunk_size);
	zassert_not_null(chunks);

	zassert_is_null(
		aceAlloc_calloc(ACE_MODULE_GROUP, ACE_ALLOC_BUFFER_GENERIC, chunks_no, chunk_size));

	aceAlloc_free(ACE_MODULE_GROUP, ACE_ALLOC_BUFFER_GENERIC, chunks);

	chunks = aceAlloc_calloc(ACE_MODULE_GROUP, ACE_ALLOC_BUFFER_GENERIC, chunks_no, chunk_size);
	zassert_not_null(chunks);

	aceAlloc_free(ACE_MODULE_GROUP, ACE_ALLOC_BUFFER_GENERIC, chunks);

	zassert_equal(ACE_STATUS_OK, aceAlloc_deInit());
}

ZTEST(sid_ace_alloc, test_sanity)
{
	zassert_true(true);
}

ZTEST_SUITE(sid_ace_alloc, NULL, suite_setup, before_test, NULL, NULL);
