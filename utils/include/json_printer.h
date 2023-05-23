/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <stdbool.h>

// Redefine this macro before use to redirect json output
#define JSON_RAW_PRINT(format, ...)                                                                \
	shell_fprintf(shell, SHELL_NORMAL, format __VA_OPT__(, ) __VA_ARGS__)

#define JSON_PRINT(indent, format, ...)                                                            \
	{                                                                                          \
		if (JSON_INDENT) {                                                                 \
			for (int i = 0; i < indent; i++) {                                         \
				JSON_RAW_PRINT("\t");                                              \
			};                                                                         \
			JSON_RAW_PRINT(format "\n" __VA_OPT__(, ) __VA_ARGS__);                    \
		} else {                                                                           \
			JSON_RAW_PRINT(format __VA_OPT__(, ) __VA_ARGS__);                         \
		}                                                                                  \
	}

#define JSON_LAST ""
#define JSON_NEXT ", "
#define JSON_END "\n"

#define JSON_DICT(name, in_line, content)                                                          \
	{                                                                                          \
		int indent = 0;                                                                    \
		bool JSON_INDENT = (in_line == false);                                             \
		JSON_VAL_DICT(name, content, JSON_END);                                            \
	}

#define JSON_VAL_STR_ENUMERATE(key_array, val_array, array_size, comma_or_empty)                   \
	{                                                                                          \
		for (int i = 0; i < array_size - 1; i++) {                                         \
			JSON_VAL_STR(key_array[i], val_array[i], JSON_NEXT);                       \
		}                                                                                  \
		JSON_VAL_STR(key_array[array_size - 1], val_array[array_size - 1],                 \
			     comma_or_empty);                                                      \
	}

#define JSON_VAL(key, val, comma_or_empty)                                                         \
	{                                                                                          \
		JSON_PRINT(indent, "\"%s\": %d" comma_or_empty, (char *)key, val);                 \
	}
#define JSON_VAL_STR(key, str, comma_or_empty)                                                     \
	{                                                                                          \
		JSON_PRINT(indent, "\"%s\": \"%s\"" comma_or_empty, (char *)key, (char *)str);     \
	}

#define JSON_VAL_DICT(key, content, comma_or_empty)                                                \
	{                                                                                          \
		JSON_PRINT(indent, "\"%s\": {", (char *)key);                                      \
		indent++;                                                                          \
		{ content };                                                                       \
		indent--;                                                                          \
		JSON_PRINT(indent, "}" comma_or_empty);                                            \
	}
