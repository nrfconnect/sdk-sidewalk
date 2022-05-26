#include <stddef.h>
#include <stdlib.h>
#include <zephyr/kernel.h>

typedef int aceModules_moduleId_t;
typedef int aceAlloc_bufferType_t;

void *aceAlloc_alloc(aceModules_moduleId_t module_id,
		     aceAlloc_bufferType_t buf_type, size_t size)
{
	ARG_UNUSED(module_id);
	ARG_UNUSED(buf_type);

	return malloc(size);
}

void aceAlloc_free(aceModules_moduleId_t module_id,
		   aceAlloc_bufferType_t buf_type, void *p)
{
	ARG_UNUSED(module_id);
	ARG_UNUSED(buf_type);

	free(p);
}
