/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef SM_UTILS_H
#define SM_UTILS_H

#include <stddef.h>
#include <stdint.h>
#include <sid_api.h>

#define PAYLOAD_MAX_SIZE                (255)
#define SID_DEMO_APP_TTL_MAX            (60)
#define SID_DEMO_APP_RETRIES_MAX        (3)

#ifdef CONFIG_SIDEWALK_LINK_MASK_BLE
#define BUILT_IN_LM             (uint32_t)(SID_LINK_TYPE_1)
#elif  CONFIG_SIDEWALK_LINK_MASK_FSK
#define BUILT_IN_LM             (uint32_t)(SID_LINK_TYPE_2)
#elif CONFIG_SIDEWALK_LINK_MASK_LORA
#define BUILT_IN_LM             (uint32_t)(SID_LINK_TYPE_3)
#else
#error "Not defined Sidewalk link mask!!"
#endif

#define SID_APP_LM_2_STR(_lm)   (_lm == SID_LINK_TYPE_1 ? "BLE" :			      \
				 (_lm == SID_LINK_TYPE_2) ? "FSK" :			      \
				 (_lm == SID_LINK_TYPE_3) ? "LoRa" :			      \
				 (_lm == (SID_LINK_TYPE_1 | SID_LINK_TYPE_2)) ? "BLE & FSK" : \
				 (_lm == (SID_LINK_TYPE_1 | SID_LINK_TYPE_3)) ? "BLE & LoRa" : "INVALID")

struct app_demo_msg {
	enum sid_link_type link_type;
	enum sid_msg_type msg_type;
	struct sid_demo_msg_desc *msg_desc;
	struct sid_demo_msg *msg;
};

struct app_demo_rx_msg {
	uint16_t msg_id;
	size_t pld_size;
	uint8_t rx_payload[PAYLOAD_MAX_SIZE];
};

/**
 * @brief Add message to message queue.
 *
 * @param event Sidewalk event.
 */
void sm_main_task_msg_q_write(enum event_type event);

/**
 * @brief Add message to message queue.
 *
 * @param rx_msg pointer to message.
 */
void sm_rx_task_msg_q_write(struct app_demo_rx_msg *rx_msg);

/**
 * @brief The function checks is Sidewalk ready.
 *
 * @return true when Sidewalk is ready false otherwise.
 */
bool sm_is_sidewalk_ready();

/**
 * @brief Get application state.
 *
 * @return Application state.
 */
enum demo_app_state sm_app_state_get();

#endif /* SM_UTILS_H */
