/*
 * Copyright 2025 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 */

#ifndef LR1110_LOCATION_PRIV_H
#define LR1110_LOCATION_PRIV_H

#include <sid_pal_gnss_ifc.h>
#include <sid_pal_wifi_ifc.h>
#include <sid_error.h>
#include <lr11xx_gnss_wifi_config.h>
#include <smtc_modem_geolocation_api.h>

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

extern struct lr11xx_location_configuration location_state;

sid_error_t sid_pal_common_location_init(void);
sid_error_t sid_pal_common_location_deinit(void);
sid_error_t sid_pal_common_process_event(void);

#endif /* LR1110_LOCATION_PRIV_H */
