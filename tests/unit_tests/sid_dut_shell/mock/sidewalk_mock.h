#ifndef SIDEWALK_MOCK_H
#define SIDEWALK_MOCK_H

#include <zephyr/fff.h>
#include <sidewalk.h>

DEFINE_FFF_GLOBALS;

FAKE_VOID_FUNC(sidewalk_start, sidewalk_ctx_t *);
FAKE_VALUE_FUNC(void *, sidewalk_data_alloc, size_t);
FAKE_VOID_FUNC(sidewalk_data_free, void *);
FAKE_VALUE_FUNC(int, sidewalk_event_send, sidewalk_event_t, void *);

#define SIDEWALK_FAKES_LIST(FAKE)                                                                  \
	FAKE(sidewalk_start)                                                                       \
	FAKE(sidewalk_data_alloc)                                                                  \
	FAKE(sidewalk_data_free)                                                                   \
	FAKE(sidewalk_event_send)

#endif /* SIDEWALK_MOCK_H */
