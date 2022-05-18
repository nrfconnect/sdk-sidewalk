#include <sys/__assert.h>
#include <zephyr.h>

#include <logging/log.h>
LOG_MODULE_REGISTER(assert, LOG_LEVEL_DBG);

void assert_post_action(const char *file, unsigned int line)
{
	LOG_ERR("assert in file %s, line %d", file, line);
	k_sleep(K_SECONDS(1));
}
