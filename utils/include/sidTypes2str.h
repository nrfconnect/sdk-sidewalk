/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#define SID_ERROR_T_STR(err)                                                                       \
	(((-(err)) > -(SID_ERROR_INVALID_STATE)) || ((-(err)) < 0)) ?                              \
		"UNKNOWN" :                                                                        \
		((char *[]){                                                                       \
			"SID_ERROR_NONE",                                                          \
			"SID_ERROR_GENERIC",                                                       \
			"SID_ERROR_TIMEOUT",                                                       \
			"SID_ERROR_OUT_OF_RESOURCES",                                              \
			"SID_ERROR_OOM",                                                           \
			"SID_ERROR_OUT_OF_HANDLES",                                                \
			"SID_ERROR_NOSUPPORT",                                                     \
			"SID_ERROR_NO_PERMISSION",                                                 \
			"SID_ERROR_NOT_FOUND",                                                     \
			"SID_ERROR_NULL_POINTER",                                                  \
			"SID_ERROR_PARAM_OUT_OF_RANGE",                                            \
			"SID_ERROR_INVALID_ARGS",                                                  \
			"SID_ERROR_INCOMPATIBLE_PARAMS",                                           \
			"SID_ERROR_IO_ERROR",                                                      \
			"SID_ERROR_TRY_AGAIN",                                                     \
			"SID_ERROR_BUSY",                                                          \
			"SID_ERROR_DEAD_LOCK",                                                     \
			"SID_ERROR_DATA_TYPE_OVERFLOW",                                            \
			"SID_ERROR_BUFFER_OVERFLOW",                                               \
			"SID_ERROR_IN_PROGRESS",                                                   \
			"SID_ERROR_CANCELED",                                                      \
			"SID_ERROR_OWNER_DEAD",                                                    \
			"SID_ERROR_UNRECOVERABLE",                                                 \
			"SID_ERROR_PORT_INVALID",                                                  \
			"SID_ERROR_PORT_NOT_OPEN",                                                 \
			"SID_ERROR_UNINITIALIZED",                                                 \
			"SID_ERROR_ALREADY_INITIALIZED",                                           \
			"SID_ERROR_ALREADY_EXISTS",                                                \
			"SID_ERROR_BELOW_THRESHOLD",                                               \
			"SID_ERROR_STOPPED",                                                       \
			"SID_ERROR_STORAGE_READ_FAIL",                                             \
			"SID_ERROR_STORAGE_WRITE_FAIL",                                            \
			"SID_ERROR_STORAGE_ERASE_FAIL",                                            \
			"SID_ERROR_STORAGE_FULL",                                                  \
			"SID_ERROR_AUTHENTICATION_FAIL",                                           \
			"SID_ERROR_ENCRYPTION_FAIL",                                               \
			"SID_ERROR_DECRYPTION_FAIL",                                               \
			"SID_ERROR_ID_OBFUSCATION_FAIL",                                           \
			"SID_ERROR_NO_ROUTE_AVAILABLE",                                            \
			"SID_ERROR_INVALID_RESPONSE",                                              \
			"SID_ERROR_INVALID_STATE",                                                 \
		}[(-(err))])

#define SID_MSG_TYPE_STR(val)                                                                      \
	((char *[]){ "SID_MSG_TYPE_GET", "SID_MSG_TYPE_SET", "SID_MSG_TYPE_NOTIFY",                \
		     "SID_MSG_TYPE_RESPONSE" }[(val)])

#define SID_LINK_MODE_STR(val)                                                                     \
	(val) == SID_LINK_MODE_CLOUD   ? "SID_LINK_MODE_CLOUD" :                                   \
	(val) == SID_LINK_MODE_MOBILE  ? "SID_LINK_MODE_MOBILE" :                                  \
	(val) == SID_LINK_MODE_INVALID ? "SID_LINK_MODE_INVALID" :                                 \
					 "UNKNOWN"
