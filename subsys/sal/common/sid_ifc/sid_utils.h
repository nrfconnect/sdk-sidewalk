/*
 * Copyright 2020-2025 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef SID_UTILS_H
#define SID_UTILS_H

#include <stdbool.h>
#include <stdint.h>

#ifdef DEBUG
#ifndef SID_BUILD_DEBUG
#define SID_BUILD_DEBUG 1
#endif
#endif

#ifdef NDEBUG
#ifndef SID_BUILD_RELEASE
#define SID_BUILD_RELEASE 1
#endif
#endif

#if SID_BUILD_DEBUG && SID_BUILD_RELEASE
#error Both SID_BUILD_DEBUG and SID_BUILD_RELEASE cannot be set at the same time
#endif

#if !SID_BUILD_DEBUG && !SID_BUILD_RELEASE
#define SID_BUILD_DEBUG 1
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef countof
#undef countof
#endif

#ifdef __clang__
#define countof(array_) (sizeof(array_) / sizeof(array_[0]))
#else
#define countof(array_)                       \
    (1 ? sizeof(array_) / sizeof((array_)[0]) \
       : sizeof(struct { int do_not_use_countof_for_pointers : ((void *)(array_) == (void *)&array_); }))
#endif

#ifdef containerof
#undef containerof
#endif

#define containerof(ptr, type, member)                                 \
    ({                                                                 \
        const __typeof__(((type *)0)->member) *tmp_member_ = (ptr);    \
        ((type *)((uintptr_t)(tmp_member_) - offsetof(type, member))); \
    })

#ifndef STR
#define STR(a_) STR_(a_)
#define STR_(a_) #a_
#endif

#ifndef CONCAT
#define CONCAT_(a_, b_) a_##b_
#define CONCAT(a_, b_) CONCAT_(a_, b_)
#endif

#ifndef CONCAT3
#define CONCAT3(a_, b_, c_) CONCAT(a_, CONCAT(b_, c_))
#endif

#ifndef CONCAT4
#define CONCAT4(a_, b_, c_, d_) CONCAT(a_, CONCAT3(b_, c_, d_))
#endif

#ifndef CONCAT5
#define CONCAT5(a_, b_, c_, d_, e_) CONCAT(a_, CONCAT4(b_, c_, d_, e_))
#endif

#ifndef CONCAT6
#define CONCAT6(a_, b_, c_, d_, e_, f_) CONCAT(a_, CONCAT5(b_, c_, d_, e_, f_))
#endif

#ifndef CONCAT7
#define CONCAT7(a_, b_, c_, d_, e_, f_, h_) CONCAT(a_, CONCAT6(b_, c_, d_, e_, f_, h_))
#endif

#ifndef SID_CEIL_DIV
#define SID_CEIL_DIV(a_, b_)          \
    ({                                \
        __typeof__(b_) b_tmp_ = (b_); \
        (a_ + b_tmp_ - 1) / b_tmp_;   \
    })
#endif

#ifndef ROUND_DIV
#define ROUND_DIV(a_, b_)             \
    ({                                \
        __typeof__(b_) b_tmp_ = (b_); \
        (a_ + b_tmp_ / 2) / b_tmp_;   \
    })
#endif

#ifndef ABS
#define ABS(a_)                          \
    ({                                   \
        __typeof__(a_) a_tmp_ = a_;      \
        (a_tmp_ > 0 ? a_tmp_ : -a_tmp_); \
    })
#endif

#ifndef SWAP
#define SWAP(a_, b_)                  \
    ({                                \
        __typeof__(a_) a_tmp_ = (a_); \
        (a_) = (b_);                  \
        (b_) = a_tmp_;                \
    })
#endif

#ifndef SID_MIN
#define SID_MIN(a_, b_) ((a_) < (b_) ? (a_) : (b_))
#endif

#ifndef SID_MAX
#define SID_MAX(a_, b_) ((a_) > (b_) ? (a_) : (b_))
#endif

#ifndef SID_IS_BIT_SET
#define SID_IS_BIT_SET(value, idx) ((value) & (1 << (idx)))
#endif

#ifndef SID_INC32_ROLLOVER_PROTECT
#define SID_INC32_ROLLOVER_PROTECT(value, inc) ((value) <= (UINT32_MAX - inc) ? (value + inc) : (value))
#endif

#ifndef SID_INC16_ROLLOVER_PROTECT
#define SID_INC16_ROLLOVER_PROTECT(value, inc) ((value) <= ((65535U) - (inc)) ? ((value) + (inc)) : (value))
#endif

#ifndef SID_INC8_ROLLOVER_PROTECT
#define SID_INC8_ROLLOVER_PROTECT(value, inc) ((value) <= ((255U) - (inc)) ? ((value) + (inc)) : (value))
#endif

#ifndef SID_SET8_ROLLOVER_PROTECT
#define SID_SET8_ROLLOVER_PROTECT(old_val, new_val) ((new_val) <= (255U) ? (new_val) : (old_val))
#endif

#ifndef SID_SET16_ROLLOVER_PROTECT
#define SID_SET16_ROLLOVER_PROTECT(old_val, new_val) ((new_val) <= (65535U) ? (new_val) : (old_val))
#endif

#ifndef SID_SET_INT8_ROLLOVER_PROTECT
#define SID_SET_INT8_ROLLOVER_PROTECT(old_val, new_val) \
    (((new_val >= (INT8_MIN)) && (new_val <= (INT8_MAX))) ? (new_val) : (old_val))
#endif

#ifndef SID_SET_INT16_ROLLOVER_PROTECT
#define SID_SET_INT16_ROLLOVER_PROTECT(old_val, new_val) \
    (((new_val >= (INT16_MIN)) && (new_val <= (INT16_MAX))) ? (new_val) : (old_val))
#endif

#define array_size(array_) (sizeof(array_) / sizeof(array_[0]))

#define FOR_EACH_ARRAY_PTR(array, ptr) \
    for (__typeof__(*(array)) *ptr = (array); (size_t)((ptr) - (array)) < array_size(array); ++(ptr))

#define FOR_EACH_ARRAY_IDX(array_, idx_) for (size_t idx_ = 0; (idx_) < array_size(array_); ++(idx_))

/*
 * See
 * https://elixir.bootlin.com/zephyr/v2.1.0/source/include/sys/util.h#L232
 */
#ifndef IS_ENABLED
#define REPLACE_TRUE_WITH_COMMA1 COMMA_,
#define IS_ENABLED(option_) IS_ENABLED_(option_)
#define IS_ENABLED_(option_) IS_ENABLED__(REPLACE_TRUE_WITH_COMMA##option_)
#define IS_ENABLED__(true_or_false_option_) IS_ENABLED___(true_or_false_option_ true, false)
#define IS_ENABLED___(ignored, option_value, ...) option_value
#endif

#ifndef UNUSED
#define UNUSED(x_) ((void)(x_))
#endif

#ifndef alignof
#define alignof(x_) __alignof(x_)
#endif

/**@brief Function for calculating CRC-16 in blocks.
 *
 * Feed each consecutive data block into this function, along with the current value of p_crc as
 * returned by the previous call of this function. The first call of this function should pass NULL
 * as the initial value of the crc in p_crc.
 *
 * @param[in] p_data The input data block for computation.
 * @param[in] size   The size of the input data block in bytes.
 * @param[in] p_crc  The previous calculated CRC-16 value or NULL if first call.
 *
 * @return The updated CRC-16 value, based on the input supplied.
 */
uint16_t crc16_calculate(uint8_t const *p_data, uint32_t size, uint16_t const *p_crc);

/**@brief Function for calculating CRC-32 in blocks.
 *
 * Feed each consecutive data block into this function, along with the current value of p_crc as
 * returned by the previous call of this function. The first call of this function should pass NULL
 * as the initial value of the crc in p_crc.
 *
 * @param[in] p_data The input data block for computation.
 * @param[in] size   The size of the input data block in bytes.
 * @param[in] p_crc  The previous calculated CRC-32 value or NULL if first call.
 *
 * @return The updated CRC-32 value, based on the input supplied.
 */
uint32_t crc32_calculate(uint8_t const *p_data, uint32_t size, uint32_t const *p_crc);

#ifdef __cplusplus
}
#endif

#endif
