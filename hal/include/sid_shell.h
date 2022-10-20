
#include <sid_api.h>
#include <sidewalk_thread.h>

void CLI_register_message_send();
void CLI_register_message_not_send();
void CLI_register_message_received();
void CLI_init(app_context_t *ctx);
void CLI_register_sid_status(const struct sid_status *status);

// add handler to send message
void sidewalk_send_message(struct sid_msg msg);
