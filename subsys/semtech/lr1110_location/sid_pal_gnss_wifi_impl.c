/*
 * Copyright 2025 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.txt file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#include <sid_pal_gnss_ifc.h>
#include <sid_pal_wifi_ifc.h>

#include <geolocation_services/mw_gnss_scan.h>
#include <smtc_modem_geolocation_api.h>
#include <sid_pal_log_ifc.h>
#include <smtc_modem_utilities.h>
#include <smtc_board_ralf.h>
#include <lr11xx_gnss_wifi_config.h>

#include <lr11xx_system.h>
#include <halo_lr11xx_radio.h>
#include <sid_pal_gnss_wifi.h>
#include <smtc_modem.h>

#include <sid_time_ops.h>
#include <sid_clock_ifc.h>

#define LBM_STACK_ID 0
#define NAV3_TLV_HEADER_SIZE 3
#define SEMTECH_GNSS_NG_TAG 0x53

enum LOCATION_EVENT_TYPE {
    EVENT_TYPE_LBM,
};

struct lr11xx_location_configuration {
    struct sid_pal_gnss_config gnss_cfg;
    struct sid_pal_wifi_config wifi_cfg;
    bool is_init;
    bool is_busy;
    lr11xx_gnss_wifi_config_t *init_config;
    smtc_modem_gnss_event_data_scan_done_t gnss_scan_done_data;
    smtc_modem_wifi_event_data_scan_done_t wifi_scan_done_data;
    bool radio_init_on_loc;
};

static struct lr11xx_location_configuration location_state = {0};
static uint8_t grp_token = 0;

void rotate_scan_grp_token() {
    if(grp_token >= 31) {
        grp_token = 2;
        return;
    }
    grp_token++;
}

void update_gnss_ng_header(uint8_t *ref, bool end) {
    *ref = 0;
    if(end) {
        *ref |= (1 << 7); // set bit 7 if not already to indicate the end
    }
    *ref |= (grp_token & 0x1F); // set last 5 bytes
}

void _app_event_lbm_gnss(enum sid_pal_gnss_events event) {
    location_state.gnss_cfg.on_gnss_event(location_state.gnss_cfg.ctx, event, 0);
}

void _app_event_lbm_wifi(enum sid_pal_wifi_events event) {
    location_state.wifi_cfg.on_wifi_event(location_state.wifi_cfg.ctx, event, 0);
}

static sid_error_t schedule_lbm_timer(uint32_t delay_ms) {
    // use whichever is initialized since it's common for lr11xx
    if(location_state.wifi_cfg.on_wifi_event) {
        location_state.wifi_cfg.on_wifi_event(location_state.wifi_cfg.ctx, EVENT_TYPE_LBM, delay_ms);
    } else if(location_state.gnss_cfg.on_gnss_event) {
        location_state.gnss_cfg.on_gnss_event(location_state.gnss_cfg.ctx, EVENT_TYPE_LBM, delay_ms);
    } else {
        return SID_ERROR_UNINITIALIZED;
    }
    return SID_ERROR_NONE;
}

void app_event_run_engine() {
    if(location_state.wifi_cfg.on_wifi_event) {
        location_state.wifi_cfg.on_wifi_event(location_state.wifi_cfg.ctx, EVENT_TYPE_LBM, 0);
    } else if(location_state.gnss_cfg.on_gnss_event) {
        location_state.gnss_cfg.on_gnss_event(location_state.gnss_cfg.ctx, EVENT_TYPE_LBM, 0);
    } else {
        SID_PAL_LOG_WARNING("Location event callback not set");
    }
}

sid_error_t sid_pal_common_process_event() {
    uint32_t sleep_time_ms = 0;
    do {
        sleep_time_ms = smtc_modem_run_engine();
    } while (sleep_time_ms == 0 || smtc_modem_is_irq_flag_pending());
    schedule_lbm_timer(sleep_time_ms);
    return SID_ERROR_NONE;
}

static void modem_event_callback(void) {
    uint8_t event_pending_count;
    smtc_modem_event_t current_event;

    do {
        // Read modem event
        smtc_modem_get_event(&current_event, &event_pending_count);
        switch (current_event.event_type) {
            case SMTC_MODEM_EVENT_RESET:
                SID_PAL_LOG_INFO("ENTERED RESET/INIT");
                smtc_modem_gnss_send_mode(LBM_STACK_ID, SMTC_MODEM_SEND_MODE_BYPASS);
                smtc_modem_wifi_send_mode(LBM_STACK_ID, SMTC_MODEM_SEND_MODE_BYPASS);
                smtc_modem_gnss_set_constellations(LBM_STACK_ID, location_state.init_config->constellation_type);
                smtc_modem_run_engine();
                break;
            case SMTC_MODEM_EVENT_GNSS_ALMANAC_DEMOD_UPDATE:
                SID_PAL_LOG_INFO("Event Received: SMTC_MODEM_EVENT_GNSS_ALMANAC_DEMOD_UPDATE");
                _app_event_lbm_gnss(SID_PAL_GNSS_ALMANAC_DEMOD_UPDATE);
                break;
            case SMTC_MODEM_EVENT_GNSS_SCAN_DONE:
                SID_PAL_LOG_INFO("Event Received: SMTC_MODEM_EVENT_GNSS_SCAN_DONE");
                smtc_modem_gnss_get_event_data_scan_done(LBM_STACK_ID, &location_state.gnss_scan_done_data);
                break;
            case SMTC_MODEM_EVENT_GNSS_TERMINATED:
                SID_PAL_LOG_INFO("Event Received: SMTC_MODEM_EVENT_GNSS_TERMINATED");
                smtc_modem_gnss_get_event_data_scan_done(LBM_STACK_ID, &location_state.gnss_scan_done_data);
                location_state.is_busy = false;
                _app_event_lbm_gnss(SID_PAL_GNSS_SCAN_COMPLETE);
                break;
            case SMTC_MODEM_EVENT_WIFI_SCAN_DONE:
                SID_PAL_LOG_INFO("Event Received: SMTC_MODEM_EVENT_WIFI_SCAN_DONE");
                smtc_modem_wifi_get_event_data_scan_done(LBM_STACK_ID, &location_state.wifi_scan_done_data);
                break;
            case SMTC_MODEM_EVENT_WIFI_TERMINATED:
                SID_PAL_LOG_INFO("Event Received: SMTC_MODEM_EVENT_WIFI_TERMINATED");
                location_state.is_busy = false;
                SID_PAL_LOG_INFO("Calling _app_event_lbm_wifi");
                _app_event_lbm_wifi(SID_PAL_WIFI_SCAN_COMPLETE);
                SID_PAL_LOG_INFO("_app_event_lbm_wifi completed");
                break;
            default:
                break;
        }
    } while (event_pending_count > 0);
}

void set_lr11xx_gnss_wifi_config(lr11xx_gnss_wifi_config_t *config) {
    location_state.init_config = config;
}

static void radio_event_notifier(sid_pal_radio_events_t event){
}

sid_pal_radio_rx_packet_t rx_packet;

static void radio_irq_handler(void) {
    /* with SMTC_MODEM_HAL_IRQ_FROM_SID_PAL defined:
     * here will only be called for radio interrupt when its not in scan mode. *
     * When in scan mode, smtc_modem_hal_radio_irq() is called instead of here */
}

sid_error_t sid_pal_common_location_init() {

    if(location_state.is_init) {
        SID_PAL_LOG_INFO("common initialization for wifi/gnss already complete.");
        return SID_ERROR_NONE;
    }
    // Status/Version check and smtc modem run configuration for testing only
    lr11xx_system_version_t ver = {0};

    halo_drv_semtech_ctx_t *drv_ctx = lr11xx_get_drv_ctx();

    if(!drv_ctx->ver.hw) {
        sid_error_t err;
        SID_PAL_LOG_ERROR("lr1110 not init. initializing for location");
        if ((err = sid_pal_radio_init(radio_event_notifier, radio_irq_handler, &rx_packet)) != RADIO_ERROR_NONE) {
            SID_PAL_LOG_ERROR("%d = sid_pal_radio_init", err);
            return err;
        }
        location_state.radio_init_on_loc = true;
    }

    if( lr11xx_system_get_version(drv_ctx, &ver) != LR11XX_STATUS_OK ) {
        SID_PAL_LOG_INFO( "lr11xx_system_get_version fail" );
    } else {
        SID_PAL_LOG_INFO("get system version HW 0x%.2X FW 0x%.4X", ver.hw, ver.fw);
    }

    grp_token = (uint8_t)((rand() % 30) + 2);

    smtc_board_initialise_and_get_ralf();
    smtc_modem_init(&modem_event_callback, true);

    location_state.is_init = true;
    return SID_ERROR_NONE;
}

sid_error_t sid_pal_common_location_deinit() {
    if(!location_state.is_init) {
        SID_PAL_LOG_INFO("wifi/gnss deinitialized");
        return SID_ERROR_NONE;
    }
    SID_PAL_LOG_INFO("common deinit for wifi/gnss");
    if(location_state.radio_init_on_loc) {
        sid_pal_radio_deinit();
        location_state.radio_init_on_loc = false;
    }
    // LBM/smtc modem does not provide a deinit
    location_state.is_init = false;
    return SID_ERROR_NONE;
}

// GNSS SPECIFIC PAL IMPLEMENTATION
sid_error_t sid_pal_gnss_init(struct sid_pal_gnss_config *config) {
    location_state.gnss_cfg = *config;
    return sid_pal_common_location_init();
}

sid_error_t sid_pal_gnss_process_event(uint8_t event_id)
{
    return sid_pal_common_process_event();
}

sid_error_t sid_pal_wifi_process_event(uint8_t event_id)
{
    return sid_pal_common_process_event();
}

sid_error_t sid_pal_gnss_alm_demod_start() {
    smtc_modem_almanac_demodulation_start(LBM_STACK_ID);
    if (schedule_lbm_timer(5) != SID_ERROR_NONE) {
        SID_PAL_LOG_ERROR("fail schedule_lbm_timer");
        return SID_ERROR_OUT_OF_RESOURCES;
    }

    return SID_ERROR_NONE;
}

sid_error_t sid_pal_gnss_alm_demod_stop() {
    SID_PAL_LOG_INFO("Almanac stop not supported");
    return SID_ERROR_NOSUPPORT;
}

sid_error_t sid_pal_gnss_deinit() {
    sid_pal_common_location_deinit();
    return SID_ERROR_NONE;
}

sid_error_t sid_pal_gnss_schedule_scan(uint32_t scan_delay_s) {
    if(location_state.is_busy) {
        SID_PAL_LOG_INFO("Location modem is busy, cannot schedule scan");
        return SID_ERROR_BUSY;
    }

    if(!location_state.is_init) {
        SID_PAL_LOG_INFO("PAL is uninitialized");
        return SID_ERROR_UNINITIALIZED;
    }

    location_state.is_busy = true;
    smtc_modem_gnss_scan(LBM_STACK_ID, location_state.init_config->scan_mode, scan_delay_s);
    sid_pal_common_process_event();
    return SID_ERROR_NONE;
}

sid_error_t sid_pal_gnss_cancel_scan() {
    return SID_ERROR_NOSUPPORT;
}

sid_error_t sid_pal_gnss_get_scan_payload(struct sid_pal_gnss_payload *gnss_scan_group) {

    if(!location_state.gnss_scan_done_data.is_valid) {
        return SID_ERROR_INSUFFICIENT_RESULTS;
    }

    memset(gnss_scan_group, 0, sizeof(struct sid_pal_gnss_payload));

    uint8_t scan_idx = location_state.gnss_scan_done_data.nb_scans_valid;
    uint8_t nav_size;

    while(scan_idx > 0) {
        scan_idx--;
        nav_size = location_state.gnss_scan_done_data.scans[scan_idx].nav_size;

        if((gnss_scan_group->size + NAV3_TLV_HEADER_SIZE + nav_size) > GNSS_MAX_PAYLOAD_SIZE) {
            SID_PAL_LOG_INFO("GNSS payload is too large. %u", gnss_scan_group->size + NAV3_TLV_HEADER_SIZE + nav_size);
            return SID_ERROR_GENERIC;
        }

        // Semtech TLV Tag
        gnss_scan_group->payload_data[gnss_scan_group->size] = SEMTECH_GNSS_NG_TAG;
        gnss_scan_group->size++;

        // Semtech TLV Length
        gnss_scan_group->payload_data[gnss_scan_group->size] = nav_size + 1; // Add one for the GNSS header
        gnss_scan_group->size++;

        // GNSS_HEADER
        update_gnss_ng_header((gnss_scan_group->payload_data + gnss_scan_group->size), (scan_idx == 0));
        gnss_scan_group->size++;

        memcpy(&gnss_scan_group->payload_data[gnss_scan_group->size], location_state.gnss_scan_done_data.scans[scan_idx].nav, nav_size);
        gnss_scan_group->size += nav_size;
    }

    return SID_ERROR_NONE;
}

// END OF GNSS SPECIFIC PAL IMPLEMENTATION

// WIFI SPECIFIC PAL IMPLEMENTATION

sid_error_t sid_pal_wifi_init(struct sid_pal_wifi_config *config) {
    location_state.wifi_cfg = *config;
    return sid_pal_common_location_init();
}

sid_error_t sid_pal_wifi_deinit(){
    sid_pal_common_location_deinit();
    return SID_ERROR_NONE;
}

sid_error_t sid_pal_wifi_schedule_scan(uint32_t scan_delay_s) {
    if(location_state.is_busy) {
        SID_PAL_LOG_INFO("Location modem is busy, cannot schedule scan");
        return SID_ERROR_BUSY;
    }

    if(!location_state.is_init) {
        SID_PAL_LOG_INFO("PAL is uninitialized");
        return SID_ERROR_UNINITIALIZED;
    }

    location_state.is_busy = true;
    smtc_modem_wifi_scan(LBM_STACK_ID, scan_delay_s);
    sid_pal_common_process_event();

    return SID_ERROR_NONE;
}

sid_error_t sid_pal_wifi_cancel_scan() {
    return SID_ERROR_NOSUPPORT;
}

sid_error_t sid_pal_wifi_get_scan_payload(struct sid_pal_wifi_payload *wifi_scan_payload) {
    uint8_t ap_idx = location_state.wifi_scan_done_data.nbr_results;
    if(ap_idx < 1) {
        return SID_ERROR_INSUFFICIENT_RESULTS;
    }
    for(uint8_t i = 0; i < location_state.wifi_scan_done_data.nbr_results; i++) {
        wifi_scan_payload->results[i].rssi = location_state.wifi_scan_done_data.results[i].rssi;
        memcpy(&(wifi_scan_payload->results[i].mac), location_state.wifi_scan_done_data.results[i].mac_address, SID_WIFI_MAC_ADDRESS_LENGTH);
    }

    wifi_scan_payload->nbr_results = location_state.wifi_scan_done_data.nbr_results;
    return SID_ERROR_NONE;
}
// END OF WIFI PAL IMPLEMENTATION
