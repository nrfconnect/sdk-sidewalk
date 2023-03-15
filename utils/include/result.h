/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

typedef enum { Val, Err } result_t;

#define Result(value_t, error_e) struct {			\
		result_t result;				\
		union { value_t val; error_e err; } val_or_err;	\
}

#define Result_Val(value) { .result = Val, .val_or_err.val = value }
#define Result_Err(error_code) { .result = Err, .val_or_err.err = error_code }

#define DECLARE_RESULT(value_t, name, error_cases ...) \
	enum name ## _error_cases{ error_cases };      \
	typedef Result(value_t, enum name ## _error_cases) name;
