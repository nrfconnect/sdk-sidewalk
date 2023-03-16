/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <unity.h>
#include <osal_alloc.h>
#include <ace/ace_status.h>

typedef struct {
	uint32_t field32b;
	uint8_t field8b;
} test_struct_t;

static void *mem[CONFIG_SIDEWALK_HEAP_SIZE];

void test_ace_alloc_init_deinit(void)
{
	TEST_ASSERT_EQUAL(ACE_STATUS_OK, aceAlloc_init());
	TEST_ASSERT_EQUAL(ACE_STATUS_OK, aceAlloc_deInit());
}

void test_ace_alloc_init_with_allocator_not_supported(void)
{
	aceAlloc_allocator_t *p_allocators = NULL;
	size_t count = 0;

	TEST_ASSERT_EQUAL(ACE_STATUS_NOT_SUPPORTED, aceAlloc_initWithAllocator(p_allocators, count));
}

void test_ace_alloc_and_free(void)
{
	TEST_ASSERT_EQUAL(ACE_STATUS_OK, aceAlloc_init());

	size_t chunk_size = 16;
	void *p = NULL;

	for (int i = 0; i < 2 * CONFIG_SIDEWALK_HEAP_SIZE; i++) {
		p = aceAlloc_alloc(ACE_MODULE_GROUP, ACE_ALLOC_BUFFER_GENERIC, chunk_size);
		TEST_ASSERT_NOT_NULL(p);
		aceAlloc_free(ACE_MODULE_GROUP, ACE_ALLOC_BUFFER_GENERIC, p);
	}

	TEST_ASSERT_EQUAL(ACE_STATUS_OK, aceAlloc_deInit());
}

void test_ace_alloc_free_negative(void)
{
	TEST_ASSERT_EQUAL(ACE_STATUS_OK, aceAlloc_init());

	aceAlloc_free(ACE_MODULE_GROUP, ACE_ALLOC_BUFFER_GENERIC, NULL);

	TEST_ASSERT_NULL(aceAlloc_alloc(ACE_MODULE_GROUP, ACE_ALLOC_BUFFER_GENERIC, 0));

	TEST_ASSERT_EQUAL(ACE_STATUS_OK, aceAlloc_deInit());
}

void test_ace_alloc_full(void)
{
	TEST_ASSERT_EQUAL(ACE_STATUS_OK, aceAlloc_init());

	size_t chunk_size = 4;
	int mem_i_max = 0;

	for (mem_i_max = 0; mem_i_max < (CONFIG_SIDEWALK_HEAP_SIZE / chunk_size); mem_i_max++) {
		mem[mem_i_max] = aceAlloc_alloc(ACE_MODULE_GROUP, ACE_ALLOC_BUFFER_GENERIC, chunk_size);
		/* Memory heap may end earlier
		 * due to alignment or metadata gaps. */
		if (!mem[mem_i_max]) {
			break;
		}
	}

	TEST_ASSERT_NULL(aceAlloc_alloc(ACE_MODULE_GROUP, ACE_ALLOC_BUFFER_GENERIC, chunk_size));

	for (int i = 0; i < mem_i_max; i++) {
		for (int j = 0; j <= i; j++) {
			aceAlloc_free(ACE_MODULE_GROUP, ACE_ALLOC_BUFFER_GENERIC, mem[j]);
		}

		for (int j = 0; j <= i; j++) {
			mem[j] = aceAlloc_alloc(ACE_MODULE_GROUP, ACE_ALLOC_BUFFER_GENERIC, chunk_size);
			TEST_ASSERT_NOT_NULL(mem[j]);
		}
	}

	for (int i = mem_i_max - 1; i >= 0; i--) {
		aceAlloc_free(ACE_MODULE_GROUP, ACE_ALLOC_BUFFER_GENERIC, mem[i]);
	}

	TEST_ASSERT_EQUAL(ACE_STATUS_OK, aceAlloc_deInit());
}

void test_ace_calloc_initialize_with_zeros(void)
{
	TEST_ASSERT_EQUAL(ACE_STATUS_OK, aceAlloc_init());

	size_t chunks_no = 10;
	size_t chunk_size = sizeof(test_struct_t);
	test_struct_t *chunks = NULL;

	chunks = aceAlloc_calloc(ACE_MODULE_GROUP, ACE_ALLOC_BUFFER_GENERIC, chunks_no, chunk_size);
	TEST_ASSERT_NOT_NULL(chunks);

	for (int i = 0; i < chunks_no; i++) {
		TEST_ASSERT_NOT_NULL(&chunks[i]);
		TEST_ASSERT_EQUAL_UINT32(0, chunks[i].field32b);
		TEST_ASSERT_EQUAL_UINT8(0, chunks[i].field8b);
	}

	for (int i = 0; i < chunks_no; i++) {
		chunks[i].field32b = 0xffffffff;
		chunks[i].field8b = 0xff;
	}

	aceAlloc_free(ACE_MODULE_GROUP, ACE_ALLOC_BUFFER_GENERIC, chunks);

	chunks = aceAlloc_calloc(ACE_MODULE_GROUP, ACE_ALLOC_BUFFER_GENERIC, chunks_no, chunk_size);
	TEST_ASSERT_NOT_NULL(chunks);

	for (int i = 0; i < chunks_no; i++) {
		TEST_ASSERT_NOT_NULL(&chunks[i]);
		TEST_ASSERT_EQUAL_UINT32(0, chunks[i].field32b);
		TEST_ASSERT_EQUAL_UINT8(0, chunks[i].field8b);
	}

	aceAlloc_free(ACE_MODULE_GROUP, ACE_ALLOC_BUFFER_GENERIC, chunks);

	TEST_ASSERT_EQUAL(ACE_STATUS_OK, aceAlloc_deInit());
}

void test_ace_calloc_negative(void)
{
	size_t chunks_no = 1, chunk_size = 1;

	TEST_ASSERT_NULL(aceAlloc_calloc(ACE_MODULE_GROUP, ACE_ALLOC_BUFFER_GENERIC, 0, chunk_size));
	TEST_ASSERT_NULL(aceAlloc_calloc(ACE_MODULE_GROUP, ACE_ALLOC_BUFFER_GENERIC, chunks_no, 0));
	TEST_ASSERT_NULL(aceAlloc_calloc(ACE_MODULE_GROUP, ACE_ALLOC_BUFFER_GENERIC, 0, 0));

	chunk_size = __SIZE_MAX__ - 1;
	chunks_no = 2;
	TEST_ASSERT_NULL(aceAlloc_calloc(ACE_MODULE_GROUP, ACE_ALLOC_BUFFER_GENERIC, chunk_size, chunks_no));
}

void test_ace_calloc_full(void)
{
	TEST_ASSERT_EQUAL(ACE_STATUS_OK, aceAlloc_init());

	size_t chunks_no = 0;
	size_t chunk_size = sizeof(test_struct_t);
	test_struct_t *chunks = NULL;

	chunks_no = ((CONFIG_SIDEWALK_HEAP_SIZE / 2) / sizeof(test_struct_t)) + 1;
	chunks = aceAlloc_calloc(ACE_MODULE_GROUP, ACE_ALLOC_BUFFER_GENERIC, chunks_no, chunk_size);
	TEST_ASSERT_NOT_NULL(chunks);

	TEST_ASSERT_NULL(aceAlloc_calloc(ACE_MODULE_GROUP, ACE_ALLOC_BUFFER_GENERIC, chunks_no, chunk_size));

	aceAlloc_free(ACE_MODULE_GROUP, ACE_ALLOC_BUFFER_GENERIC, chunks);

	chunks = aceAlloc_calloc(ACE_MODULE_GROUP, ACE_ALLOC_BUFFER_GENERIC, chunks_no, chunk_size);
	TEST_ASSERT_NOT_NULL(chunks);

	aceAlloc_free(ACE_MODULE_GROUP, ACE_ALLOC_BUFFER_GENERIC, chunks);

	TEST_ASSERT_EQUAL(ACE_STATUS_OK, aceAlloc_deInit());
}

/* It is required to be added to each test. That is because unity is using
 * different main signature (returns int) and zephyr expects main which does
 * not return value.
 */
extern int unity_main(void);

void main(void)
{
	(void)unity_main();
}
