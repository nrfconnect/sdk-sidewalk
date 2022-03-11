#include "sid_pal_log_ifc.h"
#include <stddef.h>

#include <sys/printk.h>
#include <logging/log.h>
#include <logging/log_ctrl.h>
LOG_MODULE_DECLARE(sid_log, LOG_LEVEL_DBG);

#define MSG_LENGTH_MAX (CONFIG_SIDEWALK_LOG_MSG_LENGTH_MAX)
#define LOG_FLUSH_SLEEP_PERIOD K_MSEC(5)

void sid_pal_log(sid_pal_log_severity_t severity, uint32_t num_args, const char *fmt, ...)
{
	ARG_UNUSED(num_args);

	va_list args;
	int print_len = 0;
	char msg[MSG_LENGTH_MAX] = "";

	va_start(args, fmt);
	print_len = vsnprintk(msg, sizeof(msg), fmt, args);
	va_end(args);

	switch (severity) {
	case SID_PAL_LOG_SEVERITY_ERROR:
		LOG_ERR("%s", msg);
		break;
	case SID_PAL_LOG_SEVERITY_WARNING:
		LOG_WRN("%s", msg);
		break;
	case SID_PAL_LOG_SEVERITY_INFO:
		LOG_INF("%s", msg);
		break;
	case SID_PAL_LOG_SEVERITY_DEBUG:
		LOG_DBG("%s", msg);
		break;
	default:
		LOG_DBG("%s", msg);
		LOG_WRN("sid pal log unknow severity %d", severity);
		break;
	}

	if (print_len > MSG_LENGTH_MAX) {
		LOG_WRN("sid pal log dropped %d bytes", (print_len - MSG_LENGTH_MAX));
	}
}

void sid_pal_log_flush(void)
{
	while (log_buffered_cnt()) {
		k_sleep(LOG_FLUSH_SLEEP_PERIOD);
	}
}

char const *sid_pal_log_push_str(char *string)
{
	LOG_WRN("%s - not implemented (optional).", __func__);

	return string;
}

bool sid_pal_log_get_log_buffer(struct sid_pal_log_buffer *const log_buffer)
{
	ARG_UNUSED(log_buffer);
	LOG_WRN("%s - not implemented (optional).", __func__);

	return false;
}
