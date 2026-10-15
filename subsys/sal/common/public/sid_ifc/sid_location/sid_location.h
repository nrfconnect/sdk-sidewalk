/*
 * Copyright 2024-2026 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef SID_LOCATION_IFC_H
#define SID_LOCATION_IFC_H
/// @cond sid_ifc_ep_en
/** @file
 *
 * @defgroup SIDEWALK_API Sidewalk API
 * @brief API to request and send location information.
 * @{
 * @ingroup  SIDEWALK_API
 */

#include <sid_error.h>

#include <stdint.h>
#include <stdbool.h>
#include <sid_api.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Forward declare
 */
struct sid_handle;

/**
 * Bitmask of the location methods a device is allowed to use. Used as a set of
 * flags in struct sid_location_config::sid_location_type_mask and, internally,
 * to select the WiFi vs GNSS send path. This is NOT a result discriminant — for
 * that, see enum sid_location_result_type.
 */
enum sid_location_method_type {
    SID_LOCATION_METHOD_GATEWAY = (1u << 0),
    SID_LOCATION_METHOD_GNSS = (1u << 1),
    SID_LOCATION_METHOD_WIFI = (1u << 2),
    SID_LOCATION_METHOD_FSK_GATEWAY = (1u << 3),
    SID_LOCATION_METHOD_LORA_MLAT = (1u << 4),
    SID_LOCATION_METHOD_ALL = SID_LOCATION_METHOD_GATEWAY | SID_LOCATION_METHOD_WIFI | SID_LOCATION_METHOD_GNSS
                              | SID_LOCATION_METHOD_FSK_GATEWAY | SID_LOCATION_METHOD_LORA_MLAT
};

/**
 * Discriminant for struct sid_location_result::parsed. Tells the on_update
 * consumer which union member (if any) carries a device-parseable payload.
 * Gateway-trigger methods (BLE/FSK L1/L2, LoRa MLAT L5) resolve cloud-side and
 * report SID_LOCATION_RESULT_NONE.
 */
enum sid_location_result_type {
    SID_LOCATION_RESULT_NONE = 0, /**< no device-parseable payload (gateway-trigger/cloud-resolved) */
    SID_LOCATION_RESULT_WIFI,     /**< parsed.wifi is valid */
    SID_LOCATION_RESULT_GNSS,     /**< parsed.gnss is valid */
};

/**
 * Enum to represent the location effort modes on device.
 *
 * L1: BLE gateway trigger (cloud resolves gateway position)
 * L2: FSK gateway trigger (cloud resolves gateway position)
 * L3: WiFi BSSID scan (cloud resolves via AP database)
 * L4: GNSS scan via LR1110 (cloud resolves via LSKF)
 * L5: LoRa MLAT (connectionless broadcast, cloud trilaterates from >=3 GWs)
 * L6: GPS-assist (trigger requests a cloud COMPACT_EPHEMERIS downlink so the
 *     device can produce pseudoranges; requires SID_SDK_CONFIG_ENABLE_GPS_ASSIST)
 */
enum sid_location_effort_mode {
    SID_LOCATION_EFFORT_L1 = 1,
    SID_LOCATION_EFFORT_L2,
    SID_LOCATION_EFFORT_L3,
    SID_LOCATION_EFFORT_L4,
    SID_LOCATION_EFFORT_L5,
    SID_LOCATION_EFFORT_L6,
    SID_LOCATION_EFFORT_INVALID,
    SID_LOCATION_EFFORT_DEFAULT
};

/**
 * Event types of the sid location library
 */
enum sid_location_status {
    SID_LOCATION_LVL1_READY,                 // To be used with timesync connections
    SID_LOCATION_LVL1_UNAVAILABLE,           // To be used with timesync connections
    SID_LOCATION_SEND_ONLY_DONE,             // Send-only operation completed
    SID_LOCATION_SCAN_ONLY_DONE,             // Scan-only operation completed
    SID_LOCATION_SCAN_AND_SEND_DONE,         // Scan-and-send operation completed
    SID_LOCATION_RESOLVE_MSG_SENT,           // resolve_location message sent to gateway
    SID_LOCATION_RESOLVE_MSG_ACK_RECEIVED,   // resolve_location message ack received from cloud
    SID_LOCATION_RESOLVE_MSG_TIMEOUT,        // resolve_location message send failed or timed out
};

enum sid_location_run_type {
    SID_LOCATION_SEND_ONLY = (1u << 0),
    SID_LOCATION_SCAN_ONLY = (1u << 1),
    SID_LOCATION_SCAN_AND_SEND = SID_LOCATION_SEND_ONLY | SID_LOCATION_SCAN_ONLY,
};

#define SID_LOCATION_MAX_PAYLOAD_SIZE 106

/**
 * Maximum number of effort levels a caller can specify in
 * struct sid_location_run_config::levels. Sized to hold every effort mode
 * (L1..L6) once.
 */
#define SID_LOCATION_MAX_LEVELS 6

/**
 * Typed view of a WiFi result. As a result (out), `aps` points into a
 * library-owned buffer valid for the on_update callback; as a send payload
 * (in, see struct sid_location_run_config), `aps` points into caller-owned
 * memory valid for the sid_location_run() call. Access points are packed as
 * [1B rssi][6B bssid] (stride 7); use sid_location_parse_wifi() to decode
 * without hardcoding the packing.
 */
struct sid_location_wifi_result {
    uint8_t num_aps;    /**< number of access points in `aps` */
    const uint8_t *aps; /**< packed AP array: [1B rssi][6B bssid], stride 7 */
    size_t aps_size;    /**< size of `aps` in bytes (num_aps * 7) */
};

/**
 * Typed view of a GNSS result. Used both as a result (out) and as a send
 * payload (in) — see struct sid_location_wifi_result for the buffer-ownership
 * rules.
 */
struct sid_location_gnss_result {
    const uint8_t *nav; /**< compressed NAV message (cloud-bound) */
    size_t nav_size;    /**< size of `nav` in bytes */
    const uint8_t *raw; /**< raw pseudorange/SV data, if any (may be NULL) */
    size_t raw_size;    /**< size of `raw` in bytes (0 if none) */
};

/**
 * Reusable typed location payload, shared by the outbound result
 * (struct sid_location_result) and the inbound send payload
 * (struct sid_location_run_config). The active member is selected by the
 * accompanying enum sid_location_result_type discriminant.
 */
union sid_location_data {
    struct sid_location_wifi_result wifi;
    struct sid_location_gnss_result gnss;
};

struct sid_location_run_config {
    enum sid_location_effort_mode mode;
    enum sid_location_run_type type;
    /**
     * Payload to send for a run that includes SID_LOCATION_SEND_ONLY. Selected
     * by `payload_type`: SID_LOCATION_RESULT_WIFI -> payload.wifi,
     * SID_LOCATION_RESULT_GNSS -> payload.gnss. The library copies the bytes
     * during sid_location_run(), so caller memory need only stay valid for the
     * call. Ignored (leave payload_type = SID_LOCATION_RESULT_NONE) for
     * scan-only runs, where the library produces the payload itself.
     */
    enum sid_location_result_type payload_type;
    union sid_location_data payload;
    /**
     * Optional caller-supplied effort ordering. When num_levels > 0, the
     * library attempts these effort modes in exactly this order, escalating to
     * the next entry when a level is unsupported or returns insufficient
     * results. This overrides the default L1->L6 climb and the single `mode`
     * field. When num_levels == 0, the legacy behavior applies (single `mode`
     * if specified, otherwise the managed default climb).
     *
     * GPS-assist is requested by including SID_LOCATION_EFFORT_L6 as an entry
     * (or as `mode`), not via a separate flag.
     */
    enum sid_location_effort_mode levels[SID_LOCATION_MAX_LEVELS];
    /** Number of valid entries in `levels` (0 = use legacy behavior). */
    uint8_t num_levels;
};

/**
 * Struct with return information on location uplink attempt.
 *
 * The scan/result bytes are exposed through the typed `parsed` union, selected
 * by `result_type`: `parsed.wifi` for WiFi APs, `parsed.gnss` for GNSS NAV/raw
 * data. Gateway-trigger methods report SID_LOCATION_RESULT_NONE and leave
 * `parsed` unset. The union members carry pointers into library-owned buffers
 * that remain valid for the duration of the `on_update` callback. Use
 * sid_location_parse_wifi() / sid_location_parse_gnss() to decode without
 * touching the wire layout.
 */
struct sid_location_result {
    enum sid_location_status status;
    sid_error_t err;
    enum sid_location_effort_mode mode;
    enum sid_link_type link;
    /** Which union member (if any) carries a device-parseable result. */
    enum sid_location_result_type result_type;
    /** Typed view of the result bytes, selected by `result_type`. */
    union sid_location_data parsed;
};

/**
 * One decoded WiFi access point (output of sid_location_parse_wifi()).
 */
struct sid_location_wifi_ap {
    int8_t rssi;      /**< signed dBm (packed as uint8_t on the wire) */
    uint8_t bssid[6]; /**< access point BSSID / MAC */
};

/**
 * One decoded GNSS message (output of sid_location_parse_gnss()).
 */
struct sid_location_gnss_info {
    const uint8_t *nav; /**< compressed NAV message (cloud-bound) */
    size_t nav_size;    /**< size of `nav` in bytes */
    const uint8_t *raw; /**< raw pseudorange/SV data (may be NULL) */
    size_t raw_size;    /**< size of `raw` in bytes (0 if none) */
};

/** Max satellites in the Compact24B pseudorange uplink: 10 x 19-bit slots fit
 *  the fixed 24-byte NDB8 container (190 bits + 2 pad). */
#define SID_LOCATION_GNSS_MAX_PSEUDORANGES 10

/** One decoded satellite from the Compact24B pseudorange payload. */
struct sid_location_gnss_pseudorange {
    /** Fractional-ms code phase, ms * 2^21 (0..2^21-1) as the PAL reports it.
     *  On the wire it is carried as a 19-bit slot (CO >> 2, 1 LSB ~= 0.572 m);
     *  parse_pseudoranges() returns the 19-bit value here. The integer-ms part
     *  is held cloud-side from the aiding and is not carried. */
    uint32_t code_phase_21b;
};

/**
 * Decoded view of the Compact24B NotifyPseudorange payload (header-less, N x
 * 19-bit positional slots). Output of sid_location_parse_pseudoranges().
 */
struct sid_location_gnss_pseudoranges {
    uint8_t format;  /**< always 0 (Compact24B carries no format field). */
    uint8_t num_svs; /**< number of valid entries in `svs`. */
    struct sid_location_gnss_pseudorange svs[SID_LOCATION_GNSS_MAX_PSEUDORANGES];
};

/**
 * The set of callback(s) a user can register through sid_location_init().
 */
struct sid_location_event_callback {
    /**
     * Callback invoked when location start/send is complete.
     *
     * @param[in] handle A pointer to the handle returned by sid_init().
     * @param[in] result struct containing info on the attempted uplink.
     */
    void (*on_update)(const struct sid_location_result *const result, void *context);
    /**
     * Context that user sid_location API can use
     */
    void *context;
};

/**
 * Struct to configure the timeout (ms) for stepping down location levels.
 * In the case of L4->L3 or L3->L2, the timeout will start after the scan completes.
 */
struct sid_location_stepdown_timeouts {
    uint32_t l4_to_l3;
    uint32_t l3_to_l2;
    uint32_t l2_to_l1;
};

/**
 * Struct to configure the timeout (ms) for total time to retry and total attempts
 * when fragmentation is used to uplink scan data (GNSS or WiFi) over LoRa.
 */
struct sid_location_fragmentation_retries {
    uint32_t timeout_ms;
    uint8_t max_retries;
};

/**
 * Struct representing location initialization configurations.
 */
struct sid_location_config {
    enum sid_location_method_type sid_location_type_mask;
    struct sid_location_event_callback callbacks;
    enum sid_location_effort_mode max_effort;
    bool manage_effort;
    struct sid_location_stepdown_timeouts stepdowns;
    struct sid_location_fragmentation_retries fragmentation;
    bool auto_link_pause_resume;
    /* Opaque config forwarded to the GPS-assist PAL backend
     * (struct sid_pal_gps_assist_config::pal_config) when
     * SID_SDK_CONFIG_ENABLE_GPS_ASSIST is set; e.g. the Airoha backend's
     * serial-bus factory. NULL if GPS-assist is not used. */
    void *gps_assist_pal_config;
};

/**
 * Initialize sid location library
 *
 * This API will configure and initialize one or both of the supported
 * location libraries available on device.
 *
 * @param[in] handle A pointer to the handle returned by sid_init()
 * @param[in] config Location configuration structure
 *
 * @returns #SID_ERROR_NONE in case of success
 **/
sid_error_t sid_location_init(struct sid_handle *handle, struct sid_location_config *config);

/**
 * Deinitialize sid location library
 *
 * This API will deinitialize the enabled location libraries.
 *
 * @param[in] handle A pointer to the handle returned by sid_init()
 *
 * @returns #SID_ERROR_NONE in case of success
 **/
sid_error_t sid_location_deinit(struct sid_handle *handle);

/**
 * API to scan and send available location information to the cloud to resolve
 * coordinates. If effort mode is NOT managed, SID_LOCATION_EFFORT_DEFAULT cannot
 * be used.
 *
 * @param[in] handle A pointer to the handle returned by sid_init()
 * @param[in] config struct containing the configuration for the location request
 * @param[in] delay_s delay to start scan (if applicable) and uplink data
 *
 * @returns #SID_ERROR_NONE in case of success
 **/
sid_error_t sid_location_run(struct sid_handle *handle, struct sid_location_run_config *config, uint32_t delay_s);

/**
 * API to set the maximum effort mode allowed (ex. if the device is in a low power mode we only want to
 * send messages via mode 1)
 *
 * @param[in] handle A pointer to the handle returned by sid_init()
 * @param[in] mode maximum effort mode allowed.
 *
 **/

sid_error_t sid_location_set_max_mode(struct sid_handle *handle, enum sid_location_effort_mode mode);

/**
 * API to retrieve the location mode maintained by the location library
 *
 * @param[in] handle A pointer to the handle returned by sid_init()
 *
 * @returns the current sid location effort mode.
 **/
enum sid_location_effort_mode sid_location_get_effort_mode(struct sid_handle *handle);

/**
 * Decode a WiFi location result into an array of access points.
 *
 * Centralizes the packed wire layout ([1B rssi][6B bssid], stride 7) and the
 * signed-RSSI-in-uint8_t quirk so callers never hardcode it. Safe to call from
 * within the on_update callback (the result buffers are valid for the call).
 *
 * @param[in]  in       WiFi view from struct sid_location_result::parsed.wifi
 * @param[out] out      caller-provided array to fill
 * @param[in]  max_aps  capacity of `out` in elements
 * @param[out] num_out  number of access points written (<= max_aps)
 *
 * @returns #SID_ERROR_NONE on success, #SID_ERROR_NULL_POINTER or
 *          #SID_ERROR_INVALID_ARGS on bad input.
 **/
sid_error_t sid_location_parse_wifi(const struct sid_location_wifi_result *in,
                                    struct sid_location_wifi_ap *out,
                                    size_t max_aps,
                                    size_t *num_out);

/**
 * Decode a GNSS location result into a structured view.
 *
 * GNSS payloads are cloud-resolved, so this exposes the NAV and raw buffers
 * without further interpretation, decoupling callers from struct field names.
 * Safe to call from within the on_update callback.
 *
 * @param[in]  in   GNSS view from struct sid_location_result::parsed.gnss
 * @param[out] out  caller-provided struct to fill
 *
 * @returns #SID_ERROR_NONE on success, #SID_ERROR_NULL_POINTER on bad input.
 **/
sid_error_t sid_location_parse_gnss(const struct sid_location_gnss_result *in, struct sid_location_gnss_info *out);

/**
 * Decode the compact-pseudorange (Format 0) NotifyPseudorange payload.
 *
 * This is the inverse of the packing the library uses for the L6 GPS-assist
 * uplink: a 1-byte header ([3b num_svs][2b format][3b reserved]) followed by
 * num_svs x 21-bit fractional code phases, MSB-first, zero-padded to a byte
 * boundary. Decouples callers from the bit layout. Safe to call from within the
 * on_update callback.
 *
 * @param[in]  nav       packed payload (struct sid_location_gnss_result::nav).
 * @param[in]  nav_size  size of @p nav in bytes.
 * @param[out] out       caller-provided struct to fill.
 *
 * @retval SID_ERROR_NONE           on success.
 * @retval SID_ERROR_NULL_POINTER   if @p nav or @p out is NULL.
 * @retval SID_ERROR_INVALID_ARGS   if the payload is malformed, an unsupported
 *                                  format, or too short for its num_svs.
 **/
sid_error_t
sid_location_parse_pseudoranges(const uint8_t *nav, size_t nav_size, struct sid_location_gnss_pseudoranges *out);

/**
 * Send a set of compact-pseudorange (Format 0) GPS-assist measurements.
 *
 * Packs the caller-supplied 21-bit code phases into the NotifyPseudorange wire
 * layout and uplinks them as a single GNSS notify to the SLS service, over the
 * first available link (BLE -> FSK -> LoRa). This is the manual counterpart to
 * the automatic uplink the library performs on an L6 fix: it lets an app with
 * its own pseudorange source (or replaying a captured set) drive the compact
 * uplink directly. Requires an initialized location library and network
 * timesync.
 *
 * @param[in] handle   Handle from sid_init().
 * @param[in] svs      Per-SV compact code phases (svs[i].code_phase_21b).
 * @param[in] num_svs  Number of entries (1..SID_LOCATION_GNSS_MAX_PSEUDORANGES).
 *
 * @retval SID_ERROR_NONE               the payload was handed to the link.
 * @retval SID_ERROR_NULL_POINTER       @p handle or @p svs is NULL.
 * @retval SID_ERROR_INVALID_ARGS       @p num_svs is 0 or too large.
 * @retval SID_ERROR_UNINITIALIZED      location library not initialized.
 * @retval SID_ERROR_INVALID_STATE      no network timesync.
 * @retval SID_ERROR_NO_ROUTE_AVAILABLE no link is up.
 **/
sid_error_t sid_location_send_pseudoranges(struct sid_handle *handle,
                                           const struct sid_location_gnss_pseudorange *svs,
                                           uint8_t num_svs);

/**
 * Send an already-packed compact-pseudorange payload verbatim.
 *
 * Uplinks @p data as a single GNSS notify to the SLS service over the first
 * available link, without inspecting or repacking it. The caller owns the wire
 * format (e.g. the Format 0 header + 21-bit body). Intended for tooling that
 * supplies a pre-built payload (hex from a CLI, a captured frame). Requires an
 * initialized location library and network timesync.
 *
 * @param[in] handle  Handle from sid_init().
 * @param[in] data    Packed payload bytes.
 * @param[in] size    Payload length (1..SID_LOCATION_MAX_PAYLOAD_SIZE).
 *
 * @retval SID_ERROR_NONE               the payload was handed to the link.
 * @retval SID_ERROR_NULL_POINTER       @p handle or @p data is NULL.
 * @retval SID_ERROR_INVALID_ARGS       @p size is 0 or too large.
 * @retval SID_ERROR_UNINITIALIZED      location library not initialized.
 * @retval SID_ERROR_INVALID_STATE      no network timesync.
 * @retval SID_ERROR_NO_ROUTE_AVAILABLE no link is up.
 **/
sid_error_t sid_location_send_pseudoranges_raw(struct sid_handle *handle, const uint8_t *data, size_t size);

#ifdef __cplusplus
}
#endif

#endif   // SID_LOCATION_IFC_H
