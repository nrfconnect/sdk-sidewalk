
#include <stddef.h>
#include <stdlib.h>

#include <logging/log.h>
LOG_MODULE_REGISTER(ace, LOG_LEVEL_DBG);

typedef int aceModules_moduleId_t;
typedef int aceAlloc_bufferType_t;

void *aceAlloc_alloc(aceModules_moduleId_t module_id,
		     aceAlloc_bufferType_t buf_type, size_t size)
{
	ARG_UNUSED(module_id);
	void *p = malloc(size);

	LOG_INF("Alloc: %p %d", p, size);
	return p;
}

void aceAlloc_free(aceModules_moduleId_t module_id,
		   aceAlloc_bufferType_t buf_type, void *p)
{
	ARG_UNUSED(module_id);
	LOG_INF("Free: %p", p);
	return free(p);
}
