/*
 * Copyright 2025 Amazon.com, Inc. or its affiliates.  All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and conditions
 * set forth in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef SID_DIAG_LOG_CFG_H
#define SID_DIAG_LOG_CFG_H

/// @cond sid_ifc_ep_en

/** @file
 *
 * @defgroup SIDEWALK_API Sidewalk API
 * @brief API for communicating with the Sidewalk network
 * @{
 * @ingroup  SIDEWALK_API
 */

#include <stdint.h>
#include <stddef.h>
#include <sid_error.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @struct sid_diag_log_buffer
 * @brief Structure to hold information about the diagnostic log buffer.
 *
 * This structure contains a pointer to the buffer and its size.
 */
struct sid_diag_log_buffer {
    /** Pointer to the buffer where diagnostic logs are stored. */
    uint8_t *buffer;
    /** Size of the buffer in bytes. */
    size_t size;
};

/**
 * @struct sid_diag_log_callbacks
 * @brief Callbacks for the diagnostic logging system.
 *
 * This structure holds the callback functions used by the diagnostic
 * logging system to retrieve application log messages and handle log
 * message sending completion.
 */
struct sid_diag_log_callbacks {
    /** User context data */
    void *context;

    /**
     * @brief Callback to get the application's log buffer.
     *
     * This callback is used by the diagnostic logging system to request a buffer from
     * the application where application logs are stored. The application should return a
     * sid_diag_app_log_buffer structure containing the buffer pointer and its size.
     *
     * @param[in] context The context pointer given in sid_diag_log_callbacks.context
     * @param[out] buffer Pointer to sid_diag_log_buffer.
     */
    void (*get_app_log)(struct sid_diag_log_buffer *buffer, void *context);

    /**
     * @brief Callback to notify when log sending is complete.
     *
     * This callback is invoked by the diagnostic logging system when it has finished
     * sending log data. It provides information about the number of bytes written and
     * any error that may have occurred during the send operation.
     *
     * @param[in] bytes_written The number of bytes successfully written/sent.
     * @param[in] error_code The error code indicating the result of the send operation.
     *                       SID_ERROR_NONE indicates success, other values indicate specific errors.
     * @param[in] context The context pointer given in sid_diag_log_callbacks.context
     */
    void (*on_send_complete)(size_t bytes_written, sid_error_t error_code, void *context);
};

/**
 * @enum sid_diag_log_level
 * @brief Enumeration of diagnostic log levels.
 *
 * This enum defines the various log levels for the diagnostic logging system.
 */
enum sid_diag_log_level {
    /** This level indicates that the logging evel has not been
     * explicitly set. SID_DIAG_LOG_LEVEL_INFO will be default
     * in that case */
    SID_DIAG_LOG_LEVEL_NOT_SET = 0,
    /** Error level logging */
    SID_DIAG_LOG_LEVEL_ERROR = 1,
    /** Warning level logging */
    SID_DIAG_LOG_LEVEL_WARNING = 2,
    /** Information level logging. This is the default logging level */
    SID_DIAG_LOG_LEVEL_INFO = 3,
    /** This is the most verbose logging level. Use it for detailed information
     * useful during development and debugging */
    SID_DIAG_LOG_LEVEL_DEBUG = 4,
};

/**
 * @struct sid_diag_log_config
 * @brief Configuration structure for the diagnostic logging system.
 *
 * This structure holds the configuration parameters needed to initialize
 * the diagnostic logging system. It includes settings for the log buffer
 * logging level and the callbacks for retrieving application log messages
 * and handling log message sending completion.
 */
struct sid_diag_log_config {
    /** Pointer to the buffer used for storing log messages.
     * This buffer should be allocated by the caller */
    uint8_t *buffer;
    /** Size of the buffer in bytes pointed to by `buffer` Should
     * be at least SID_DIAG_LOG_BUFFER_MIN_SIZE */
    size_t buffer_size;
    /** This field determines the verbosity of the logging system */
    enum sid_diag_log_level level;
    /** This field holds the callbacks for the diagnostic logging system */
    struct sid_diag_log_callbacks *callbacks;
};

struct sid_handle;

/**
 * @brief Retrieves the diagnostic log buffer.
 *
 * This function populates the provided buffer_info structure with information
 * about the current diagnostic log buffer. The caller can then use this
 * information to access the log data.
 *
 * @param[in] handle Pointer to the SID handle structure.
 * @param[out] buffer_info Pointer to a structure that will be filled with
 *                         log buffer information.
 *
 * @note The caller is responsible for calling sid_diag_release_log_buffer()
 *       after using the buffer, to make it accessible for subsequent operations.
 *       However, if the returned buffer is NULL or empty, there's no need to
 *       call sid_diag_release_log_buffer().
 *
 * @see sid_diag_release_log_buffer()
 */
sid_error_t sid_diag_get_log_buffer(struct sid_handle *handle, struct sid_diag_log_buffer *buffer_info);

/**
 * @brief Releases the diagnostic log buffer.
 *
 * This function should be called after the caller has finished using the
 * log buffer obtained via sid_diag_get_log_buffer(). It releases the buffer,
 * making it available for subsequent logging operations or for other parts
 * of the system to access.
 *
 * @param[in] handle Pointer to the SID handle structure.
 *
 * @note Failing to call this function after sid_diag_get_log_buffer() may
 *       prevent new log entries from being recorded.
 *
 * @see sid_diag_get_log_buffer()
 */
sid_error_t sid_diag_release_log_buffer(struct sid_handle *handle);

#ifdef __cplusplus
} /* extern "C" */
#endif

/** @} */

/// @endcond

#endif /* SID_DIAG_LOG_CFG_H */