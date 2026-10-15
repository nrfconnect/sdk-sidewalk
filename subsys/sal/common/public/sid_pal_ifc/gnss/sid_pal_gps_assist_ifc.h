/*
 * Copyright 2026 Amazon.com, Inc. or its affiliates. All rights reserved.
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

/** @file
 *
 * @defgroup sid_pal_gps_assist_ifc SID GPS-assist (A-GNSS) PAL interface
 * @{
 * @ingroup sid_pal_ifc
 *
 * @details Platform interface for cloud-initiated GPS-assisted location (L6).
 * The flow is:
 *
 *   1. init()               -- bring up the backend + GNSS IC transport.
 *   2. inject_ephemeris()   -- push the compact aiding payload from the cloud
 *                              into the IC (satellite list + coarse code phase
 *                              and Doppler); leaves the IC ready to acquire.
 *   3. start_acquisition()  -- kick off a fix; an acquisition-timeout is armed.
 *   4. on_event()           -- fires FIX_ACQUIRED (measurements ready) or
 *                              FIX_TIMEOUT / ERROR asynchronously.
 *   5. get_pseudoranges()   -- read back the raw per-SV measurements, which are
 *                              uplinked for cloud-side (LSKF) position solve.
 *
 * A concrete backend (e.g. the Airoha AG3352 driver in
 * platform/sid_mcu/airoha/hal) implements these symbols; the transport it uses
 * is supplied opaquely via #sid_pal_gps_assist_config::pal_config.
 */

#ifndef SID_PAL_GPS_ASSIST_IFC_H
#define SID_PAL_GPS_ASSIST_IFC_H

#include <stdint.h>
#include <stddef.h>
#include <sid_error.h>
#include <stdbool.h>

#include <sid_event_queue_ifc.h>
#include <sid_memory_pool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Maximum number of satellites reported in one pseudorange result. */
#define SID_PAL_GPS_ASSIST_MAX_SVS 12

/**
 * Maximum size, in bytes, of the compact aiding payload accepted by
 * sid_pal_gps_assist_inject_ephemeris().
 *
 * Compact aiding wire format (produced by the cloud, consumed by the backend):
 * a packed, MSB-first bitstream of fixed-width per-SV records, no padding
 * between records. Each record is 27 bits:
 *
 *   | bits (MSB-first) | field           | width | units / range            |
 *   |------------------|-----------------|-------|--------------------------|
 *   | [26:22]          | sv_id           |  5    | GPS PRN, 1..32            |
 *   | [21:12]          | int_code_phase  | 10    | integer-ms code phase, 0..1023 ms |
 *   | [11:0]           | doppler         | 12    | signed range rate, m/s, clamped +/-2048 |
 *
 * So N satellites occupy 27*N bits = ceil(27*N/8) bytes. The 106-byte cap
 * comfortably holds the 12-SV maximum (27*12 = 324 bits = 41 bytes) plus any
 * backend-specific header/aiding fields.
 */
#define SID_PAL_GPS_ASSIST_MAX_PAYLOAD_SIZE 106

/** Asynchronous events delivered to #sid_pal_gps_assist_config::on_event. */
enum sid_pal_gps_assist_event {
    /** A fix was acquired; call get_pseudoranges() to read the measurements. */
    SID_PAL_GPS_ASSIST_EVENT_FIX_ACQUIRED,
    /** The acquisition-timeout elapsed before a usable fix was obtained. */
    SID_PAL_GPS_ASSIST_EVENT_FIX_TIMEOUT,
    /** An unrecoverable backend/transport error occurred during acquisition. */
    SID_PAL_GPS_ASSIST_EVENT_ERROR,
};

/** Power/activity state of the GNSS IC as tracked by the backend. */
enum sid_pal_gps_assist_power_state {
    SID_PAL_GPS_ASSIST_POWER_OFF,       /**< IC powered down / not initialized. */
    SID_PAL_GPS_ASSIST_POWER_SLEEP,     /**< Initialized and aided, idle (ready to acquire). */
    SID_PAL_GPS_ASSIST_POWER_ACQUIRING, /**< Actively searching for a fix. */
    SID_PAL_GPS_ASSIST_POWER_TRACKING,  /**< Fix acquired; measurements available. */
};

/**
 * One satellite's raw measurement, as read back after a fix.
 *
 * The pseudorange is carried in the compact21 code-phase representation the
 * uplink uses directly, so no lossy metres round-trip is needed:
 *
 *   pseudorange_m = (int_ms + code_phase_21b / 2^21) * 0.001 * c
 *
 * where c is the speed of light. @ref code_phase_21b is the fractional-ms part
 * (the chip's native 21-bit CO field, LSB = 1/2^21 ms ~= 0.143 m) and is what
 * the compact uplink carries; @ref int_ms is the integer-ms count the cloud
 * also holds from the aiding it sent down.
 */
struct sid_pal_gps_assist_sv_measurement {
    uint8_t sv_id;           /**< GPS PRN (1..32). */
    uint32_t code_phase_21b; /**< Fractional-ms code phase, ms * 2^21 (0..2^21-1). */
    uint16_t int_ms;         /**< Integer-ms code phase (0..1023 ms). */
    uint8_t cn0_dbhz;        /**< Carrier-to-noise density, dB-Hz (0 if not reported). */
    int16_t doppler_hz;      /**< Measured Doppler in Hz (signed). */
};

/** Set of per-SV measurements from a single fix. */
struct sid_pal_gps_assist_pseudorange_result {
    uint8_t num_svs; /**< Number of valid entries in @ref svs (<= SID_PAL_GPS_ASSIST_MAX_SVS). */
    struct sid_pal_gps_assist_sv_measurement svs[SID_PAL_GPS_ASSIST_MAX_SVS];
};

/** Configuration passed to sid_pal_gps_assist_init(). */
struct sid_pal_gps_assist_config {
    /** Async event callback (FIX_ACQUIRED / FIX_TIMEOUT / ERROR). Required. */
    void (*on_event)(enum sid_pal_gps_assist_event event, void *ctx);
    /** Opaque context passed back to @ref on_event. */
    void *ctx;
    /** Acquisition timeout in seconds; 0 lets the backend pick a default. */
    uint32_t acquisition_timeout_s;
    /** SDK event queue the backend schedules the acquisition-timeout on (as a
     *  deferred event rather than a sid_pal_timer). Must be serviced by the
     *  owner's run loop. Required. */
    const struct sid_event_queue *event_queue;
    /** Memory pool the backend may allocate from; NULL if unused. */
    struct sid_memory_pool *memory_pool;
    /** Opaque, PAL-implementation-specific configuration (e.g. the transport
     *  the backend uses to reach the GNSS IC). The concrete type is defined by
     *  the linked backend; see its header (e.g. sid_pal_gps_assist_airoha.h). */
    void *pal_config;
};

/**
 * @brief Initialize the GPS-assist PAL and bring up the GNSS IC transport.
 *
 * @param[in] config  Configuration (see #sid_pal_gps_assist_config). @c on_event
 *                     and @c event_queue are required; @c pal_config carries the
 *                     backend transport.
 * @retval SID_ERROR_NONE           on success.
 * @retval SID_ERROR_INVALID_ARGS   on a missing/invalid required field.
 * @return Backend-specific error otherwise.
 */
sid_error_t sid_pal_gps_assist_init(const struct sid_pal_gps_assist_config *config);

/**
 * @brief Deinitialize the PAL, cancel any pending acquisition, and release the IC.
 * @retval SID_ERROR_NONE on success (also if already deinitialized).
 */
sid_error_t sid_pal_gps_assist_deinit(void);

/**
 * @brief Inject the compact A-GNSS aiding payload into the GNSS IC.
 *
 * @param[in] data  Compact aiding bitstream (see @ref SID_PAL_GPS_ASSIST_MAX_PAYLOAD_SIZE
 *                  for the 27-bit/SV record layout).
 * @param[in] size  Payload length in bytes (1..SID_PAL_GPS_ASSIST_MAX_PAYLOAD_SIZE).
 * @retval SID_ERROR_NONE           the aiding was accepted; IC is ready to acquire.
 * @retval SID_ERROR_UNINITIALIZED  init() has not been called.
 * @retval SID_ERROR_INVALID_ARGS   @p data is NULL or @p size is out of range.
 */
sid_error_t sid_pal_gps_assist_inject_ephemeris(const uint8_t *data, size_t size);

/**
 * @brief Start a fix attempt and arm the acquisition-timeout.
 *
 * Completion is signalled asynchronously via #sid_pal_gps_assist_config::on_event.
 * @retval SID_ERROR_NONE           acquisition started.
 * @retval SID_ERROR_UNINITIALIZED  init() has not been called.
 */
sid_error_t sid_pal_gps_assist_start_acquisition(void);

/**
 * @brief Abort an in-progress acquisition and return the IC to sleep.
 * @retval SID_ERROR_NONE           on success.
 * @retval SID_ERROR_UNINITIALIZED  init() has not been called.
 */
sid_error_t sid_pal_gps_assist_abort_acquisition(void);

/**
 * @brief Read back the raw per-SV measurements from the most recent fix.
 *
 * Valid after a FIX_ACQUIRED event.
 *
 * @param[out] result  Filled with up to SID_PAL_GPS_ASSIST_MAX_SVS measurements.
 * @retval SID_ERROR_NONE           measurements returned in @p result.
 * @retval SID_ERROR_UNINITIALIZED  init() has not been called.
 * @retval SID_ERROR_INVALID_ARGS   @p result is NULL.
 * @retval SID_ERROR_INVALID_STATE  no valid fix is available yet.
 */
sid_error_t sid_pal_gps_assist_get_pseudoranges(struct sid_pal_gps_assist_pseudorange_result *result);

/**
 * @brief Get the current power/activity state of the GNSS IC.
 * @return the current #sid_pal_gps_assist_power_state.
 */
enum sid_pal_gps_assist_power_state sid_pal_gps_assist_get_power_state(void);

#ifdef __cplusplus
}
#endif

/** @} */

#endif
