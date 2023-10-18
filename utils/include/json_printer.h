/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <stdbool.h>

/* Redefine this macro before use to redirect json output */
#define JSON_RAW_PRINT(format, ...) JSON_RAW_PRINTER_SHELL(shell, format __VA_OPT__(, ) __VA_ARGS__)

/**
 * @brief Implementation of JSON_RAW_PRINT that sends the messages to shell
 * @arg shell pointer to shell object
 */
#define JSON_RAW_PRINTER_SHELL(shell, format, ...)                                                 \
	shell_fprintf(shell, SHELL_NORMAL, format __VA_OPT__(, ) __VA_ARGS__)

/**
 * @brief Implementation of JSON_RAW_PRINT that sends the messages to log
 * 
 */
#define JSON_RAW_PRINTER_LOG(format, ...) LOG_RAW(format __VA_OPT__(, ) __VA_ARGS__)

/**
 * @brief Implementation of JSON_RAW_PRINT that sends the messages to memory buffer
 * @arg out memory buffer
 * @arg chars_written variable that stores the count of written characters
 * @arg capacity size of the out buffer
 * 
 */
#define JSON_RAW_PRINTER_STRING(out, chars_written, capacity, format, ...)                         \
	{                                                                                          \
		do {                                                                               \
			int printed_chars =                                                        \
				snprintf(out + chars_written, capacity - chars_written,            \
					 format __VA_OPT__(, ) __VA_ARGS__);                       \
			if (printed_chars < 0) {                                                   \
				LOG_ERR("Failed to add message to JSON string");                   \
			} else {                                                                   \
				chars_written += printed_chars;                                    \
			}                                                                          \
		} while (0);                                                                       \
	}

/**
 * @brief handler for printing JSON values, uses JSON_RAW_PRINT to redirect message
 * @arg indent if set to true, the JSON will be printed in multiline mode with indentation of 1 tab 
 *
 */
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

/**
 * @brief value of comma_or_empty argument in JSON_VAL_* macros that signals that this is the last value in array or dict
 * 
 */
#define JSON_LAST ""
/**
 * @brief value of comma_or_empty argument in JSON_VAL_* macros that signals that this is not the last value in array or dict
 * 
 */
#define JSON_NEXT ", "

/**
 * @brief Create JSON starting from root dictionary element
 * @arg name the key to the dictionary
 * @arg in_line print JSON in single line or multiline (true = single line /false = multi line)
 * 
 * example output:
 ```
	JSON_DICT("test", true, JSON_VAL_INT("v1", 3, JSON_LAST))
 ```
 * 	{"test": {"v1": 3}}
 */
#define JSON_DICT(name, in_line, ...)                                                              \
	{                                                                                          \
		int indent = 0;                                                                    \
		bool JSON_INDENT = (in_line == false);                                             \
		JSON_PRINT(indent, "{");                                                           \
		JSON_VAL_DICT(name, JSON_LAST, { __VA_ARGS__ });                                   \
		JSON_PRINT(indent, "}\n");                                                         \
	}

/**
 * @brief Add N string elements to dictionary from arrays with key and values
 * @arg key_array array with names of the string values
 * @arg val_array array with values, must have the same size as key_array
 * @arg array_size number of elements in key_array/val_array
 *
 * example output:
 ```
	char* key_arr[2] = {"var1", "var2"};
	char* val_arr[2] = {"value1", "value2"};
	JSON_DICT("example", false, JSON_VAL_STR_ENUMERATE(key_arr, val_arr, 2, JSON_LAST))
 ```
 * {"example": {"var1": "value1", "var2": "value2"}}
 */
#define JSON_VAL_STR_ENUMERATE(key_array, val_array, array_size, comma_or_empty)                   \
	{                                                                                          \
		for (int i = 0; i < array_size - 1; i++) {                                         \
			JSON_VAL_STR(key_array[i], val_array[i], JSON_NEXT);                       \
		}                                                                                  \
		JSON_VAL_STR(key_array[array_size - 1], val_array[array_size - 1],                 \
			     comma_or_empty);                                                      \
	}

/**
 * @brief Add INT value to the dict or array
 * 
 * example output:
 ```
	JSON_DICT("test", true, JSON_VAL_INT("v1", 3, JSON_LAST))
 ```
 * 	{"test": {"v1": 3}}
 */
#define JSON_VAL_INT(key, val, comma_or_empty)                                                     \
	{                                                                                          \
		JSON_PRINT(indent, "\"%s\": %ld" comma_or_empty, (char *)key, (long int)val);      \
	}

/**
 * @brief Add FLOAT value to the dict or array
 * 
 * example output:
 ```
	JSON_DICT("test", true, JSON_VAL_FLOAT("v1", 3.14, JSON_LAST))
 ```
 * 	{"test": {"v1": 3.14}}
 */
#define JSON_VAL_FLOAT(key, val, comma_or_empty)                                                   \
	{                                                                                          \
		JSON_PRINT(indent, "\"%s\": %f" comma_or_empty, (char *)key, val);                 \
	}

/**
 * @brief Add DOUBLE value to the dict or array
 * 
 * example output:
 ```
	JSON_DICT("test", true, JSON_VAL_DOUBLE("v1", 3.14, JSON_LAST))
 ```
 * 	{"test": {"v1": 3.14}}
 */
#define JSON_VAL_DOUBLE(key, val, comma_or_empty)                                                  \
	{                                                                                          \
		JSON_PRINT(indent, "\"%s\": %lf" comma_or_empty, (char *)key, val);                \
	}

/**
 * @brief Add BOOL value to the dict or array
 * 
 * example output:
 ```
	JSON_DICT("test", true, JSON_VAL_BOOL("v1", 23, JSON_LAST))
 ```
 * 	{"test": {"v1": true}}
 */
#define JSON_VAL_BOOL(key, val, comma_or_empty)                                                    \
	{                                                                                          \
		JSON_PRINT(indent, "\"%s\": %s" comma_or_empty, (char *)key,                       \
			   val ? "true" : "false");                                                \
	}

/**
 * @brief Add STR value to the dict or array
 * 
 * example output:
 ```
	JSON_DICT("test", true, JSON_VAL_STR("v1", "message", JSON_LAST))
 ```
 * 	{"test": {"v1": "message"}}
 */
#define JSON_VAL_STR(key, str, comma_or_empty)                                                     \
	{                                                                                          \
		JSON_PRINT(indent, "\"%s\": \"%s\"" comma_or_empty, (char *)key, (char *)str);     \
	}

/**
 * @brief Add DICT object to the dict or array
 * 
 * example output:
 ```
	JSON_DICT("test", true, JSON_VAL_DICT("obj", JSON_LAST, JSON_VAL_STR("v1", "message", JSON_LAST)))
 ```
 * 	{"test": {"obj":{"v1": "message"}}}
 */
#define JSON_VAL_DICT(key, comma_or_empty, ...)                                                    \
	{                                                                                          \
		JSON_PRINT(indent, "\"%s\": {", (char *)key);                                      \
		indent++;                                                                          \
		{ __VA_ARGS__ };                                                                   \
		indent--;                                                                          \
		JSON_PRINT(indent, "}" comma_or_empty);                                            \
	}
