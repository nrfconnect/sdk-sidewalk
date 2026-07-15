/*
 * Copyright 2020-2025 Amazon.com, Inc. or its affiliates. All rights reserved.
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

/* SDK users may define these macros themselves to override the values used here,
 * in order to control which group IDs the Sidewalk code uses. This allows users
 * to deconflict any group ID collisions between application code and Sidewalk
 * code that also uses this PAL.
 */

/* Note: Do not use group Ids greater than 0x6FFE*/

#if !defined(SID_PAL_STORAGE_KV_INTERNAL_PROTOCOL_GROUP_ID)
#define SID_PAL_STORAGE_KV_INTERNAL_PROTOCOL_GROUP_ID 0x2000
#endif


#if !defined(SID_PAL_STORAGE_KV_INTERNAL_CONFIG_GROUP_ID)
#define SID_PAL_STORAGE_KV_INTERNAL_CONFIG_GROUP_ID 0x3456
#endif

#if !defined(SID_PAL_STORAGE_KV_INTERNAL_BULK_DATA_TRANSFER_GROUP_ID)
#define SID_PAL_STORAGE_KV_INTERNAL_BULK_DATA_TRANSFER_GROUP_ID 0x4567
#endif

#if !defined(SID_PAL_STORAGE_KV_INTERNAL_METRICS_GROUP_ID)
#define SID_PAL_STORAGE_KV_INTERNAL_METRICS_GROUP_ID 0x5678
#endif

#if !defined(SID_PAL_STORAGE_KV_INTERNAL_SETUP_DIAG_GROUP_ID)
#define SID_PAL_STORAGE_KV_INTERNAL_SETUP_DIAG_GROUP_ID 0x6789
#endif
