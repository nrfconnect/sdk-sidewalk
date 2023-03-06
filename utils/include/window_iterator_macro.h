/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#include <zephyr/sys/util.h>

/**
 * @brief split set of arguments into windows of size 2, and call `macro` on every set
 * If the argument element count is not divisible by 2, it will not compile
 *
 * example:
 * value1, value2, value3, value4, value5, value 6
 * result:
 * macro(value1, value2) sep macro(value3, value4) sep macro(value5, value6)
 */
#define WINDOW_ITERATOR_SIZE_2(macro, sep, ...) UTIL_CAT(WINDOW_ITERATOR_2_EL_,				     \
							 NUM_VA_ARGS_LESS_1(dummy, __VA_ARGS__))(macro, sep, \
												 __VA_ARGS__)

// INTERNAL macros
#define WINDOW_ITERATOR_2_EL_0(...)
#define WINDOW_ITERATOR_2_EL_2(action, sep, a, b) action(a, b)
#define WINDOW_ITERATOR_2_EL_4(action, sep, a, b, ...) action(a, b) __DEBRACKET sep WINDOW_ITERATOR_2_EL_2(action, sep,	\
													   __VA_ARGS__)
#define WINDOW_ITERATOR_2_EL_6(action, sep, a, b, ...) action(a, b) __DEBRACKET sep WINDOW_ITERATOR_2_EL_4(action, sep,	\
													   __VA_ARGS__)
#define WINDOW_ITERATOR_2_EL_8(action, sep, a, b, ...) action(a, b) __DEBRACKET sep WINDOW_ITERATOR_2_EL_6(action, sep,	\
													   __VA_ARGS__)
#define WINDOW_ITERATOR_2_EL_10(action, sep, a, b, ...) action(a, b) __DEBRACKET sep WINDOW_ITERATOR_2_EL_8(action, sep, \
													    __VA_ARGS__)
#define WINDOW_ITERATOR_2_EL_12(action, sep, a, b, ...) action(a, b) __DEBRACKET sep WINDOW_ITERATOR_2_EL_10(action, \
													     sep,    \
													     __VA_ARGS__)
#define WINDOW_ITERATOR_2_EL_14(action, sep, a, b, ...) action(a, b) __DEBRACKET sep WINDOW_ITERATOR_2_EL_12(action, \
													     sep,    \
													     __VA_ARGS__)
#define WINDOW_ITERATOR_2_EL_16(action, sep, a, b, ...) action(a, b) __DEBRACKET sep WINDOW_ITERATOR_2_EL_14(action, \
													     sep,    \
													     __VA_ARGS__)
#define WINDOW_ITERATOR_2_EL_18(action, sep, a, b, ...) action(a, b) __DEBRACKET sep WINDOW_ITERATOR_2_EL_16(action, \
													     sep,    \
													     __VA_ARGS__)
#define WINDOW_ITERATOR_2_EL_20(action, sep, a, b, ...) action(a, b) __DEBRACKET sep WINDOW_ITERATOR_2_EL_18(action, \
													     sep,    \
													     __VA_ARGS__)
