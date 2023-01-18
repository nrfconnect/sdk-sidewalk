/*
 * Copyright 2022 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#include <sid_demo_types.h>

#include <sid_endian.h>
#include <sid_error.h>
#include <sid_pal_log_ifc.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void sid_demo_app_msg_serialize(struct sid_parse_state *const state,
                                struct sid_demo_msg_desc *msg_desc, struct sid_demo_msg *msg)
{
    uint8_t value = 0;
    set_bit_to_field(&value, (uint8_t)msg_desc->status_hdr_ind, SID_DEMO_APP_MSG_DESC_STATUS_HDR_IND_MASK,
                     SID_DEMO_APP_MSG_DESC_STATUS_HDR_IND_OFFSET);
    set_bit_to_field(&value, (uint8_t)msg_desc->opc, SID_DEMP_APP_MSG_DESC_OP_CODE_MASK,
                     SID_DEMO_APP_MSG_DESC_OP_CODE_OFFSET);
    set_bit_to_field(&value, (uint8_t)msg_desc->cmd_class, SID_DEMO_APP_MSG_DESC_CLASS_MASK,
                     SID_DEMO_APP_MSG_DESC_CLASS_OFFSET);
    set_bit_to_field(&value, (uint8_t)msg_desc->cmd_id, SID_DEMP_APP_MSG_DESC_CMD_ID_MASK,
                     SID_DEMO_APP_MSG_DESC_CMD_ID_OFFSET);

    sid_write_entry_uint8(state, value);
    if (msg_desc->status_hdr_ind) {
        sid_write_entry_uint8(state, msg_desc->status_code);
    }

    if (msg && msg->payload_size) {
        sid_write_entry_nbytes(state, msg->payload, msg->payload_size);
    }
}

void sid_demo_app_msg_deserialize(struct sid_parse_state *const state,
                                  struct sid_demo_msg_desc *msg_desc, struct sid_demo_msg *msg)
{
    uint8_t value = 0;
    sid_read_entry_uint8(state, &value);
    get_bit_from_field(&value, (uint8_t *)&msg_desc->status_hdr_ind, SID_DEMO_APP_MSG_DESC_STATUS_HDR_IND_MASK,
                       SID_DEMO_APP_MSG_DESC_STATUS_HDR_IND_OFFSET);
    get_bit_from_field(&value, (uint8_t *)&msg_desc->opc, SID_DEMP_APP_MSG_DESC_OP_CODE_MASK,
                       SID_DEMO_APP_MSG_DESC_OP_CODE_OFFSET);
    get_bit_from_field(&value, &msg_desc->cmd_class, SID_DEMO_APP_MSG_DESC_CLASS_MASK,
                       SID_DEMO_APP_MSG_DESC_CLASS_OFFSET);
    get_bit_from_field(&value, &msg_desc->cmd_id, SID_DEMP_APP_MSG_DESC_CMD_ID_MASK,
                       SID_DEMO_APP_MSG_DESC_CMD_ID_OFFSET);
    if (msg_desc->status_hdr_ind) {
        sid_read_entry_uint8(state, &msg_desc->status_code);
    }
    if (state->offset < state->buffer_len) {
        msg->payload_size = state->buffer_len - state->offset;
        sid_read_entry_nbytes(state, msg->payload, msg->payload_size);
    }
}

void sid_demo_app_capability_discovery_notification_serialize(struct sid_parse_state *const state,
                                                              struct sid_demo_capability_discovery *cap)
{
    struct tl_in_tlv tl = {};
    if (cap->num_buttons) {
        tl.len = cap->num_buttons;
        tl.tag = SID_DEMO_TAG_NUMBER_OF_BUTTONS;
        sid_write_entry_tlv_nbytes(state, &tl, cap->button_id_arr, cap->num_buttons);
    }

    if (cap->num_leds) {
        tl.len = cap->num_leds;
        tl.tag = SID_DEMO_TAG_NUMBER_OF_LEDS;
        sid_write_entry_tlv_nbytes(state, &tl, cap->led_id_arr, cap->num_leds);
    }

    if (cap->temp_sensor > SID_DEMO_TEMPERATURE_SENSOR_NOT_SUPPORTED && cap->temp_sensor < SID_DEMO_TEMPERATURE_SENSOR_LAST) {
        tl.len = sizeof(uint8_t);
        tl.tag = SID_DEMO_TAG_TEMP_SENSOR_AVAILABLE_AND_UNIT_REPRESENTATION;
        sid_write_entry_tlv_uint8(state, &tl, cap->temp_sensor);
    }

    tl.len = sizeof(cap->link_type);
    tl.tag = SID_DEMO_TAG_LINK_TYPE;
    sid_write_entry_tlv_uint8(state, &tl, cap->link_type);
}

void sid_demo_app_capability_discovery_notification_deserialize(struct sid_parse_state *const state,
                                                                struct sid_demo_capability_discovery *cap)
{
    struct tl_in_tlv tl = {};
    while (state->offset < state->buffer_len) {
        sid_read_tl_from_tlv(state, &tl);
        switch (tl.tag) {
            case SID_DEMO_TAG_NUMBER_OF_BUTTONS:
                cap->num_buttons = tl.len;
                sid_read_entry_nbytes(state, cap->button_id_arr, cap->num_buttons);
                break;
            case SID_DEMO_TAG_NUMBER_OF_LEDS:
                cap->num_leds = tl.len;
                sid_read_entry_nbytes(state, cap->led_id_arr, cap->num_leds);
                break;
            case SID_DEMO_TAG_TEMP_SENSOR_AVAILABLE_AND_UNIT_REPRESENTATION:
                sid_read_entry_uint8(state, (uint8_t *)&cap->temp_sensor);
                break;
            case SID_DEMO_TAG_LINK_TYPE:
                sid_read_entry_uint8(state, &cap->link_type);
                break;
            default:
                break;
        }
    }
}

void sid_demo_app_action_req_serialize(struct sid_parse_state *const state,
                                       struct sid_demo_led_action_req *led_action_req)
{
    struct tl_in_tlv tl = {};

    if (!led_action_req->num_leds) {
        return;
    }

    if (led_action_req->action_req == SID_DEMO_ACTION_LED_ON) {
        tl.tag = SID_DEMO_TAG_LED_ON_ACTION_REQ;
    } else if (led_action_req->action_req == SID_DEMO_ACTION_LED_OFF) {
        tl.tag = SID_DEMO_TAG_LED_OFF_ACTION_REQ;
    } else {
        state->ret_code = SID_ERROR_INVALID_ARGS;
        return;
    }

    if (led_action_req->num_leds == 0xFF) {
        tl.len = sizeof(led_action_req->num_leds);
        sid_write_entry_tlv_uint8(state, &tl, led_action_req->num_leds);
    } else {
        tl.len = led_action_req->num_leds;
        sid_write_entry_tlv_nbytes(state, &tl, led_action_req->led_id_arr, led_action_req->num_leds);
    }

    tl.tag = SID_DEMO_TAG_CURRENT_GPS_TIME_IN_SECONDS;
    tl.len = sizeof(led_action_req->gps_time_in_seconds);
    sid_write_entry_tlv_uint32(state, &tl, led_action_req->gps_time_in_seconds);
}

void sid_demo_app_action_req_deserialize(struct sid_parse_state *const state,
                                         struct sid_demo_led_action_req *led_action_req)
{
    struct tl_in_tlv tl = {};
    while (state->offset < state->buffer_len) {
        sid_read_tl_from_tlv(state, &tl);
        switch (tl.tag) {
            case SID_DEMO_TAG_LED_ON_ACTION_REQ:
            case SID_DEMO_TAG_LED_OFF_ACTION_REQ:
                led_action_req->action_req = (tl.tag == SID_DEMO_TAG_LED_ON_ACTION_REQ) ? (SID_DEMO_ACTION_LED_ON) :
                                             (SID_DEMO_ACTION_LED_OFF);
                led_action_req->num_leds = tl.len;
                sid_read_entry_nbytes(state, led_action_req->led_id_arr, led_action_req->num_leds);
                if (tl.len == sizeof(uint8_t) && led_action_req->led_id_arr[0] == 0xFF) {
                    led_action_req->num_leds = 0xFF;
                }
                break;
            case SID_DEMO_TAG_CURRENT_GPS_TIME_IN_SECONDS:
                sid_read_entry_uint32(state, &led_action_req->gps_time_in_seconds);
                break;
            default:
                break;
        }
    }
}

void sid_demo_app_action_resp_serialize(struct sid_parse_state *const state,
                                        struct sid_demo_action_resp *action_resp)
{
    struct tl_in_tlv tl = {};

    if (action_resp->resp_type == SID_DEMO_ACTION_TYPE_NO_ACTION) {
        goto err;
    }

    if  (action_resp->resp_type == SID_DEMO_ACTION_TYPE_LED) {
        if (action_resp->led_action_resp.action_resp == SID_DEMO_ACTION_LED_NO_ACTION) {
            goto err;
        }
        if (action_resp->led_action_resp.action_resp > SID_DEMO_ACTION_LED_NO_ACTION &&
                action_resp->led_action_resp.action_resp < SID_DEMO_ACTION_LED_LAST) {
            tl.tag = (action_resp->led_action_resp.action_resp == SID_DEMO_ACTION_LED_ON) ?
                     (SID_DEMO_TAG_LED_ON_RESP) : (SID_DEMO_TAG_LED_OFF_RESP);
            if (action_resp->led_action_resp.num_leds == 0xFF) {
                tl.len = sizeof(action_resp->led_action_resp.num_leds);
                sid_write_entry_tlv_uint8(state, &tl, action_resp->led_action_resp.num_leds);
            } else {
                tl.len = action_resp->led_action_resp.num_leds;
                sid_write_entry_tlv_nbytes(state, &tl, action_resp->led_action_resp.led_id_arr,
                                           action_resp->led_action_resp.num_leds);
            }
        } else {
            goto err;
        }
    }

    if (action_resp->resp_type == SID_DEMO_ACTION_TYPE_BUTTON) {
        if (action_resp->button_action_resp.action_resp != SID_DEMO_ACTION_BUTTON_PRESSED) {
            goto err;
        }
        tl.tag = SID_DEMO_TAG_BUTTON_PRESSED_RESP;
        if (action_resp->button_action_resp.num_buttons == 0xFF) {
            sid_write_entry_tlv_uint8(state, &tl, action_resp->button_action_resp.num_buttons);
        } else {
            tl.len = action_resp->button_action_resp.num_buttons;
            sid_write_entry_tlv_nbytes(state, &tl, action_resp->button_action_resp.button_id_arr,
                                       action_resp->button_action_resp.num_buttons);
        }
    }

    if (action_resp->gps_time_in_seconds) {
        tl.tag = SID_DEMO_TAG_CURRENT_GPS_TIME_IN_SECONDS;
        tl.len = sizeof(action_resp->gps_time_in_seconds);
        sid_write_entry_tlv_uint32(state, &tl, action_resp->gps_time_in_seconds);
    }

    if (action_resp->down_link_latency_secs) {
        tl.tag = SID_DEMO_TAG_DL_LATENCY_IN_SECONDS;
        tl.len = sizeof(action_resp->down_link_latency_secs);
        sid_write_entry_tlv_uint32(state, &tl, action_resp->down_link_latency_secs);
    }

    if (action_resp->link_type) {
        tl.tag = SID_DEMO_TAG_LINK_TYPE;
        tl.len = sizeof(action_resp->link_type);
        sid_write_entry_tlv_uint8(state, &tl, action_resp->link_type);
    }

    return;

err:
    state->ret_code = SID_ERROR_INVALID_ARGS;
}

void sid_demo_app_action_resp_deserialize(struct sid_parse_state *const state, struct sid_demo_action_resp *action_resp)
{
    struct tl_in_tlv tl = {};

    while (state->offset < state->buffer_len) {
        sid_read_tl_from_tlv(state, &tl);
        switch (tl.tag) {
            case SID_DEMO_TAG_LED_ON_RESP:
            case SID_DEMO_TAG_LED_OFF_RESP:
                action_resp->led_action_resp.action_resp = (tl.tag == SID_DEMO_TAG_LED_ON_RESP) ?
                                                            (SID_DEMO_ACTION_LED_ON) : (SID_DEMO_ACTION_LED_OFF);
                action_resp->led_action_resp.num_leds = tl.len;
                sid_read_entry_nbytes(state, action_resp->led_action_resp.led_id_arr, action_resp->led_action_resp.num_leds);
                if ((tl.len == sizeof(uint8_t)) && (action_resp->led_action_resp.led_id_arr[0] == 0xFF)) {
                    action_resp->led_action_resp.num_leds = 0xFF;
                }
                action_resp->resp_type = SID_DEMO_ACTION_TYPE_LED;
                break;
            case SID_DEMO_TAG_BUTTON_PRESSED_RESP:
                action_resp->button_action_resp.action_resp = SID_DEMO_ACTION_BUTTON_PRESSED;
                action_resp->button_action_resp.num_buttons = tl.len;
                sid_read_entry_nbytes(state, action_resp->button_action_resp.button_id_arr,
                                      action_resp->button_action_resp.num_buttons);
                if ((tl.len == sizeof(uint8_t)) && (action_resp->button_action_resp.button_id_arr[0] == 0xFF)) {
                    action_resp->button_action_resp.num_buttons = 0xFF;
                }
                action_resp->resp_type = SID_DEMO_ACTION_TYPE_BUTTON;
                break;
            case SID_DEMO_TAG_CURRENT_GPS_TIME_IN_SECONDS:
                sid_read_entry_uint32(state, &action_resp->gps_time_in_seconds);
                break;
            case SID_DEMO_TAG_DL_LATENCY_IN_SECONDS:
                sid_read_entry_uint32(state, &action_resp->down_link_latency_secs);
                break;
            case SID_DEMO_TAG_LINK_TYPE:
                sid_read_entry_uint8(state, &action_resp->link_type);
                break;
            default:
                break;
        }
    }
}

void sid_demo_app_action_notification_serialize(struct sid_parse_state *const state,
                                                struct sid_demo_action_notification *action_notify)
{
    struct tl_in_tlv tl = {};

    if (action_notify->button_action_notify.action_resp == SID_DEMO_ACTION_BUTTON_PRESSED) {
        tl.tag = SID_DEMO_TAG_BUTTON_PRESS_ACTION_NOTIFY;
        if (action_notify->button_action_notify.num_buttons == 0xFF) {
            sid_write_entry_tlv_uint8(state, &tl, action_notify->button_action_notify.num_buttons);
        } else {
            tl.len = action_notify->button_action_notify.num_buttons;
            sid_write_entry_tlv_nbytes(state, &tl, action_notify->button_action_notify.button_id_arr,
                                       action_notify->button_action_notify.num_buttons);
        }
    }

    if ((action_notify->temp_sensor > SID_DEMO_TEMPERATURE_SENSOR_NOT_SUPPORTED) &&
            (action_notify->temp_sensor < SID_DEMO_TEMPERATURE_SENSOR_LAST)) {
        tl.tag = SID_DEMO_TAG_TEMPERATURE_SENSOR_DATA_NOTIFY;
        tl.len = sizeof(action_notify->temperature);
        sid_write_entry_tlv_uint16(state, &tl, action_notify->temperature);
    }

    tl.tag = SID_DEMO_TAG_CURRENT_GPS_TIME_IN_SECONDS;
    tl.len = sizeof(action_notify->gps_time_in_seconds);
    sid_write_entry_tlv_uint32(state, &tl, action_notify->gps_time_in_seconds);

    tl.tag = SID_DEMO_TAG_LINK_TYPE;
    tl.len = sizeof(action_notify->link_type);
    sid_write_entry_tlv_uint8(state, &tl, action_notify->link_type);
}

void sid_demo_app_action_notification_deserialize(struct sid_parse_state *const state,
                                          struct sid_demo_action_notification *action_notify)
{
    struct tl_in_tlv tl = {};

    while (state->offset < state->buffer_len) {
        sid_read_tl_from_tlv(state, &tl);
        switch (tl.tag) {
            case SID_DEMO_TAG_BUTTON_PRESS_ACTION_NOTIFY:
                action_notify->button_action_notify.action_resp = SID_DEMO_ACTION_BUTTON_PRESSED;
                action_notify->button_action_notify.num_buttons = tl.len;
                sid_read_entry_nbytes(state, action_notify->button_action_notify.button_id_arr,
                                      action_notify->button_action_notify.num_buttons);
                if (tl.len == sizeof(uint8_t) && action_notify->button_action_notify.button_id_arr[0] == 0xFF) {
                    action_notify->button_action_notify.num_buttons = 0xFF;
                }
                break;
            case SID_DEMO_TAG_TEMPERATURE_SENSOR_DATA_NOTIFY:
                sid_read_entry_uint16(state, (uint16_t *)&action_notify->temperature);
                break;
            case SID_DEMO_TAG_CURRENT_GPS_TIME_IN_SECONDS:
                sid_read_entry_uint32(state, &action_notify->gps_time_in_seconds);
                break;
            case SID_DEMO_TAG_LINK_TYPE:
                sid_read_entry_uint8(state, &action_notify->link_type);
                break;
            default:
                break;
        }
    }
}

