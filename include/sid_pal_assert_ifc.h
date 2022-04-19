/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.txt file. This file is a
 * Modifiable File, as defined in the accompanying LICENSE.txt file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef SID_PAL_ASSERT_IFC_H
#define SID_PAL_ASSERT_IFC_H

/** @file
 *
 * @defgroup sid_pal_assert_ifc sid assert interface
 * @{
 * @ingroup sid_pal_ifc
 *
 * @details     Provides assert interface to be implemented by the platform.
 */

#ifdef __cplusplus
extern "C" {
#endif

#if defined(SID_PAL_ASSERT_DISABLED)
#   define SID_PAL_ASSERT(expression) (void)(expression)
#elif defined(SID_PAL_ASSERT_USE_C_ASSERT)
#   include <assert.h>
#   define SID_PAL_ASSERT(expression) assert(expression)
#else // !defined(SID_PAL_ASSERT_DISABLED) && !defined(SID_PAL_ASSERT_USE_C_ASSERT)
#   define SID_PAL_ASSERT(expression) \
    ((expression) ? \
        ((void)0) : \
        sid_pal_assert(__LINE__, __FILE__))

void sid_pal_assert(int line, const char * file) __attribute__((__noreturn__));

#endif

#ifdef __cplusplus
}
#endif

/** @} */

#endif // SID_PAL_ASSERT_IFC_H
