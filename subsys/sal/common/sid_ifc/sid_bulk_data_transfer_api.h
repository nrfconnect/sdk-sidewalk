/*
 * Copyright 2023-2024 Amazon.com, Inc. or its affiliates.  All rights reserved.
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

#ifndef SID_BULK_DATA_TRANSFER_API_H
#define SID_BULK_DATA_TRANSFER_API_H

/// @cond sid_ifc_ep_en

/** @file
 *
 * @defgroup SIDEWALK_BULK_DATA_TRANSFER_API Sidewalk Bulk Data Transfer API
 * @brief API for communicating with the Sidewalk file transfer
 * @{
 * @ingroup  SIDEWALK_BULK_DATA_TRANSFER_API
 */

#include <sid_api.h>

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    /** Max size of file descriptor field */
    SID_BULK_DATA_TRANSFER_FILE_DESCRIPTOR_MAX_SIZE = 128,
};

/**
 * Describes the action to be taken for the file transfer request
 */
enum sid_bulk_data_transfer_action {
    /** Accept the file transfer */
    SID_BULK_DATA_TRANSFER_ACTION_ACCEPT = 0,
    /** Reject the file transfer */
    SID_BULK_DATA_TRANSFER_ACTION_REJECT = 1,
};

/**
 * Describes the reject or cancel reason
 */
enum sid_bulk_data_transfer_reject_reason {
    /** No reject reason */
    SID_BULK_DATA_TRANSFER_REJECT_REASON_NONE = 0x0,
    /** Rejected for generic reasons */
    SID_BULK_DATA_TRANSFER_REJECT_REASON_GENERIC = 0x1,
    /** Rejected because file is too big */
    SID_BULK_DATA_TRANSFER_REJECT_REASON_FILE_TOO_BIG = 0x3,
    /** Rejected because device has not space */
    SID_BULK_DATA_TRANSFER_REJECT_REASON_NO_SPACE = 0x4,
    /** Rejected bacause low battery to complete transfer */
    SID_BULK_DATA_TRANSFER_REJECT_REASON_LOW_BATTERY = 0x5,
    /** Rejected because file verification failed */
    SID_BULK_DATA_TRANSFER_REJECT_REASON_FILE_VERIFICATION_FAILED = 0x9,
    /** Rejected because file already exist */
    SID_BULK_DATA_TRANSFER_REJECT_REASON_FILE_ALREADY_EXISTS = 0xB,
    /** Rejected because fragment size is not supported */
    SID_BULK_DATA_TRANSFER_REJECT_REASON_INVALID_FRAGMENT_SIZE = 0xE,
};

/**
 * Describes the final status of the file transfer
 *
 */
enum sid_bulk_data_transfer_final_status {
    /** Indicates if the file transfer was successful */
    SID_BULK_DATA_TRANSFER_FINAL_STATUS_SUCCESS = 0,
    /** Indicates if the file transfer ultimately failed */
    SID_BULK_DATA_TRANSFER_FINAL_STATUS_FAILURE = 1,
};

/**
 * Describes the meta information for a file transfer request
 * @see #sid_bulk_data_transfer_response
 */
struct sid_bulk_data_transfer_request {
    /** Size of the fragment, that is given to the user */
    uint32_t fragment_size;
    /** Identifier of incoming file transfer */
    uint32_t file_id;
    /** Offset of the file for the incoming file transfer */
    uint32_t file_offset;
    /** Size of the incoming file transfer */
    uint32_t file_size;
    /** Size of the scratch buffer needed by the Sidewalk stack to stage
     * intermediate file transfer data
     */
    size_t minimum_scratch_buffer_size;
    /* Information to describe file
     */
    const uint8_t *file_descriptor;
    /* Size of file_descriptor
     */
    uint32_t file_descriptor_size;
};

/**
 * Describes the response for a file transfer request
 * @see #sid_bulk_data_transfer_request
 */
struct sid_bulk_data_transfer_response {
    /** Used to indicate to the Sidewalk stack to either reject or accept the
     * file transfer */
    enum sid_bulk_data_transfer_action status;
    /** Used to indicate additional information on why transfer was rejected
     */
    enum sid_bulk_data_transfer_reject_reason reject_reason;
    /** Pointer to the scratch buffer memory requested in
     * \ref sid_bulk_data_transfer_request.scratch_buffer_size */
    uint8_t *scratch_buffer;
    /** Size of the scratch buffer */
    size_t scratch_buffer_size;
};

/**
 * Describes the metadata associated with the received transfer data
 */
struct sid_bulk_data_transfer_desc {
    /** The link type this message was recieved from */
    enum sid_link_type link_type;
    /** Identifier of incoming file transfer */
    uint32_t file_id;
    /** Expected offset of the file that this data is from */
    size_t file_offset;
};

/**
 * Describes bulk transfer data
 */
struct sid_bulk_data_transfer_buffer {
    void *data;
    size_t size;
};

/**
 * Describes bulk data transfer stats
 */
struct sid_bulk_data_transfer_stats {
    /** Percent of the file already transferred */
    uint8_t file_progress_percent;
    /** Offset of the completed portion of the file*/
    uint32_t file_offset;
};

/*
 * Describes the parameters for a given transfer
 */
struct sid_bulk_data_transfer_params {
    /** Size of the fragment being used in the transfer */
    uint32_t fragment_size;
    /** Size of the file currently being transferred */
    uint32_t file_size;
    /** Size of the file descriptor, is 0 if file_descriptor is not present */
    uint32_t file_descriptor_size;
    /** Buffer holding the file descriptor value */
    uint8_t file_descriptor[SID_BULK_DATA_TRANSFER_FILE_DESCRIPTOR_MAX_SIZE];
    /** The size of the scrach buffer requested for this transfer */
    size_t minimum_scratch_buffer_size;
    /** The size of the scrach buffer provided by the user */
    size_t scratch_buffer_size;
    /* Pointer to the scratch buffer provided by the user */
    uint8_t *scratch_buffer;
};

/**
 * The set of callbacks user has to register through sid_bulk_data_transfer_init()
 */
struct sid_bulk_data_transfer_event_callbacks {
    /** User context data */
    void *context;

    /**
     * Callback that is invoked when a transfer request is recieved
     *
     * The Sidewalk stack invokes this callback to let the user know about an
     * Incoming transfer request. The user can then choose to accept or reject
     * said transfer request
     *
     * @param[in] transfer_request A pointer to the data transfer request, which is never NULL
     * @param[out] transfer_response A pointer to the data transfer response, which is never NULL
     * @param[in] context The context pointer given in sid_bulk_data_transfer_event_callbacks.context
     *
     */
    void (*on_transfer_request)(const struct sid_bulk_data_transfer_request *const transfer_request,
                                struct sid_bulk_data_transfer_response *const transfer_response,
                                void *context);

    /**
     * Callback that is invoked when data has been received
     *
     * @param[in] desc A pointer to the bulk transfer data, which is never NULL
     * @param[in] buffer A pointer to the buffer that contains the received data, which is never NULL
     * @param[in] context The context pointer given in sid_bulk_data_transfer_event_callbacks.context
     */
    void (*on_data_received)(const struct sid_bulk_data_transfer_desc *const desc,
                             const struct sid_bulk_data_transfer_buffer *const buffer,
                             void *context);

    /**
     * Callback that is invoked when data transfer is done
     *
     * @param[in] file_id Identifier of the file being transferred
     * @param[in] context The context pointer given in sid_bulk_data_transfer_event_callbacks.context
     */
    void (*on_finalize_request)(uint32_t file_id, void *context);

    /**
     * Callback that is invoked when data transfer is cancelled from cloud
     *
     * @param[in] file_id Identifier of the file being transferred
     * @param[in] context The context pointer given in sid_bulk_data_transfer_event_callbacks.context
     */
    void (*on_cancel_request)(uint32_t file_id, void *context);

    /**
     * Callback that is invoked when an error is encountered during data transfer
     *
     * The Sidewalk stack will invoke this api to let the user know an error
     * was encountered during file transfer. Internally Sidewalk stack cleans
     * up it's state and goes to the same state as when sid_bulk_data_transfer_start()
     * is called.
     *
     * @param[in] file_id Identifier of the file being transferred
     * @param[in] context The context pointer given in sid_bulk_data_transfer_event_callbacks.context
     */
    void (*on_error)(uint32_t file_id, void *context);

    /**
     * Callback that is invoked when Sidewalk stack wants to release scratch buffer
     *
     * The Sidewalk stack will invoke this api to let the user cleanup the scratch buffer
     * that was allocated to during on_transfer_request call
     *
     * @param[in] file_id Identifier of the file being released
     * @param[in] context The context pointer given in sid_bulk_data_transfer_event_callbacks.context
     */
    void (*on_release_scratch_buffer)(uint32_t file_id, void *context);
};

/**
 * Describes the config needed to sidewalk bulk transfer
 */
struct sid_bulk_data_transfer_config {
    /** The callbacks associated with the bulk transfer
     *  Must use static const storage as members are accessed without being
     *  copied.
     */
    struct sid_bulk_data_transfer_event_callbacks *callbacks;
};

/**
 * Initializes sidewalk bulk transfer
 *
 * @see sid_bulk_data_transfer_init
 * @see sid_init
 *
 * @param[in] config Required configuration needed to initialize sidewalk bulk transfer
 * @param[in] handle A pointer to the handle returned by sid_init()
 *
 * @returns #SID_ERROR_NONE in case of success
 */
sid_error_t sid_bulk_data_transfer_init(const struct sid_bulk_data_transfer_config *config, struct sid_handle *handle);

/**
 * De-initialize Sidewalk bulk transfer
 *
 * @see sid_bulk_data_transfer_init
 *
 * @param[in] handle A pointer to the handle returned by sid_init()
 *
 * @returns #SID_ERROR_NONE in case of success
 */
sid_error_t sid_bulk_data_transfer_deinit(struct sid_handle *handle);

/**
 * Cancels any ongoing sidewalk bulk transfer
 *
 * @param[in] handle A pointer to the handle returned by sid_init()
 * @param[in] file_id Identifier of the file transfer that needs to be cancelled
 * @param[in] reason Reason for cancelling the transfer
 *
 * @return #SID_ERROR_NONE in case of success
 */
sid_error_t sid_bulk_data_transfer_cancel(struct sid_handle *handle,
                                          uint32_t file_id,
                                          enum sid_bulk_data_transfer_reject_reason reason);

/**
 * Release data buffer
 *
 * This api is called to release ownership of the data buffer back to sidewalk bulk transfer
 *
 * @param[in] handle A pointer to the handle returned by sid_init()
 * @param[in] file_id Identifier of the file transfer whose buffer is being released
 * @param[in] buffer A pointer to the buffer that needs to be released, which is never NULL
 *
 * @returns #SID_ERROR_NONE in case of success
 **/
sid_error_t sid_bulk_data_transfer_release_buffer(struct sid_handle *handle,
                                                  uint32_t file_id,
                                                  const struct sid_bulk_data_transfer_buffer *const buffer);

/**
 * Complete an ongoing Sidewalk bulk transfer
 *
 * This function is called when on_finalize_request callback is invoked. The purpose
 * of the api is to inform sidewalk bulk transfer if the transfer was successful (CRC checks/signature is verified),
 * or it was a failure.
 *
 * @see sid_bulk_data_transfer_event_callbacks.on_finalize_request
 *
 * @param[in] handle A pointer to the handle returned by sid_init()
 * @param[in] file_id Identifier of the file being transferred
 * @param[in] status Indicates if the transfer was completed successfully from the user end
 *
 * @returns #SID_ERROR_NONE in case of success
 */
sid_error_t sid_bulk_data_transfer_finalize(struct sid_handle *handle,
                                            uint32_t file_id,
                                            enum sid_bulk_data_transfer_final_status status);

/**
 * Get the stats of  an ongoing Sidewalk bulk transfer
 *
 * This function is called to get the current stats/progress of an ongoing file transfer
 *
 * @see #sid_bulk_data_transfer_stats
 *
 * @param[in] handle A pointer to the handle returned by sid_init()
 * @param[in] file_id Identifier of the file being transferred
 * @param[in] stats A pointer to the structure that holds the stats information
 *
 * @returns #SID_ERROR_NONE in case of success
 */
sid_error_t sid_bulk_data_transfer_get_transfer_stats(struct sid_handle *handle,
                                                      uint32_t file_id,
                                                      struct sid_bulk_data_transfer_stats *const stats);

/**
 * Get the params of an ongoing Sidewalk bulk transfer
 *
 * This function retrieves the invariant properties of an ongoing file transfer. This api can be called once a file
 * transfer is accepted.
 *
 * @see #sid_bulk_data_transfer_params
 *
 * @param[in] handle A pointer to the handle returned by sid_init()
 * @param[in] file_id Identifier of the file being transferred
 * @param[in] params A pointer to the structure that holds the params information
 *
 * @returns #SID_ERROR_NONE in case of success
 */
sid_error_t sid_bulk_data_transfer_get_transfer_params(struct sid_handle *handle,
                                                       uint32_t file_id,
                                                       struct sid_bulk_data_transfer_params *const params);

/**
 * Compute the minimum scratch_buffer_size given a fragment_size
 *
 * Based on a fragement size selected for the transfer, sidewalk bulk transfer has
 * minimum scratch buffer size requirements.
 *
 * @param[in] fragment_size Size of the fragment for calculating the scratch buffer size
 *
 * @returns minimum scratch buffer size
 */
size_t sid_bulk_data_transfer_compute_min_scratch_buffer_size(uint32_t fragment_size);

#ifdef __cplusplus
} /* extern "C" */
#endif

/** @} */

/// @endcond

#endif /* SID_BULK_DATA_TRANSFER_API_H */
