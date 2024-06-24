/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef JSON_PRINTER_H
#define JSON_PRINTER_H

/**
  * @brief This module is a zero cost abstraction to prepare JSON string representation of objects.
  * It works by generating list of arguments for printf function.
  *
  * Benefits and limitations:
  * - Because the macros are expanded by preprocessor, they can not be used by other macros like LOG_*, They should be used with functions like
  *   printk, shell_printk, z_log_minimal_printk.
  * - There is no need to create any JSON object, or prepare memory buffor for storing string representation, it is essentially very sophisticated way generate printf arguments.
  * - New types can be easily created, any structure can be eventually simplified to the basic types that are already defined
  * - Number of elements in JSON objects and arrays have to be known at compile time, so it is impossible to use this module for printing array with size unknown at compile time
  * - The macro must be evaluated by preprocessor to simple list of arguments for printf function, therefore there can not be any conditional code inside macro
  * 
  * Example: 
  * printf(JSON_OBJ(JSON_NAME("my_variable_name", JOSN_INT(123))));
  *
  * printf("{" "\"%s\" : %d" "}", "my_variable_name", 123);
  * > {"my_variable_name": 123}
  *
  * Example 2:
  *
  * printf(JSON_OBJ(JSON_NAME("array", JSON_ARRAY(JSON_LIST_2(JSON_INT(3), JSON_FLOAT(3.14)))));
  * > {"array": [3, 3.14]}
  *
  * Example 3:
  *
  * printf(JSON_OBJ(JSON_LIST_2(
      JSON_NAME("array", JSON_ARRAY(JSON_LIST_2(
        JSON_INT(3), JSON_FLOAT(3.14)
      ))), 
      JSON_NAME("dictionary", JSON_OBJ(JSON_NAME("JSON_is_great", true)))
    )));
  * > {"array": [3, 3.14], "dictionary": {"JSON_is_great": true}}
  * 
  */

/**
 * @brief prepare printf formater and arguments to print variable
 * 
 * @arg val: intiger value to print
 */
#define JSON_INT(val) "%d", val

/**
 * @brief prepare printf formater and arguments to print variable
 * 
 * @arg val: float value to print
 */
#define JSON_FLOAT(val) "%f", val

/**
 * @brief prepare printf formater and arguments to print variable
 * 
 * @arg val: bool value to print
 */
#define JSON_BOOL(val) "%s", val ? "true" : "false"

/**
 * @brief prepare printf formater and arguments to print variable
 * 
 * @arg val: char* value to print
 */
#define JSON_STR(val) "\"%s\"", val

/**
 * @brief Append new line at the end of JSON string passed as argument
 * 
 */
#define JSON_NEW_LINE(JSON) _JSON_FORMAT(JSON) "\n" _JSON_ARGS(JSON)

/**
 * @brief add name to the JSON value, object or array.
 * 
 */
#define JSON_NAME(name, json_object)                                                               \
	"\"%s\": " _JSON_FORMAT(json_object), name _JSON_ARGS(json_object)

#define _JSON_FORMAT(format, ...) format
#define _JSON_ARGS(format, ...) __VA_OPT__(, ) __VA_ARGS__

/**
 * @brief Create JSON array
 * @param obj content of JSON object. Only valid value for this argument is one of JSON_LIST_X macros
 */
#define JSON_ARRAY(obj) "[" _JSON_FORMAT(obj) "]" _JSON_ARGS(obj)

/**
 * @brief Create JSON object
 * @param obj content of JSON object. Only valid value for this argument is one of JSON_LIST_X macros
 * to create a dictionary the JSON_LIST_X have to take JSON_NAME for every argument
 */
#define JSON_OBJ(obj) "{" _JSON_FORMAT(obj) "}" _JSON_ARGS(obj)

// clang-format off
#define JSON_LIST_1(_1) \
_JSON_FORMAT(_1)  _JSON_ARGS(_1)

#define JSON_LIST_2(_1, _2) \
_JSON_FORMAT(_1) ", " _JSON_FORMAT(_2) \
_JSON_ARGS(_1) _JSON_ARGS(_2)

#define JSON_LIST_3(_1, _2, _3) \
_JSON_FORMAT(_1) ", " _JSON_FORMAT(_2) ", " _JSON_FORMAT(_3) \
_JSON_ARGS(_1) _JSON_ARGS(_2) _JSON_ARGS(_3)

#define JSON_LIST_4(_1, _2, _3, _4) \
_JSON_FORMAT(_1) ", " _JSON_FORMAT(_2) ", " _JSON_FORMAT(_3) ", " _JSON_FORMAT(_4) \
_JSON_ARGS(_1) _JSON_ARGS(_2) _JSON_ARGS(_3) _JSON_ARGS(_4) 

#define JSON_LIST_5(_1, _2, _3, _4, _5) \
_JSON_FORMAT(_1) ", " _JSON_FORMAT(_2) ", " _JSON_FORMAT(_3) ", " _JSON_FORMAT(_4) ", " _JSON_FORMAT(_5) \
_JSON_ARGS(_1) _JSON_ARGS(_2) _JSON_ARGS(_3) _JSON_ARGS(_4) _JSON_ARGS(_5)

#define JSON_LIST_6(_1, _2, _3, _4, _5, _6) \
_JSON_FORMAT(_1) ", " _JSON_FORMAT(_2) ", " _JSON_FORMAT(_3) ", " _JSON_FORMAT(_4) ", " _JSON_FORMAT(_5) ", " _JSON_FORMAT(_6) \
_JSON_ARGS(_1) _JSON_ARGS(_2) _JSON_ARGS(_3) _JSON_ARGS(_4) _JSON_ARGS(_5) _JSON_ARGS(_6)

#define JSON_LIST_7(_1, _2, _3, _4, _5, _6, _7) \
_JSON_FORMAT(_1) ", " _JSON_FORMAT(_2) ", " _JSON_FORMAT(_3) ", " _JSON_FORMAT(_4) ", " _JSON_FORMAT(_5) ", " _JSON_FORMAT(_6) ", " _JSON_FORMAT(_7) \
_JSON_ARGS(_1) _JSON_ARGS(_2) _JSON_ARGS(_3) _JSON_ARGS(_4) _JSON_ARGS(_5) _JSON_ARGS(_6) _JSON_ARGS(_7) 

#define JSON_LIST_8(_1, _2, _3, _4, _5, _6, _7, _8) \
_JSON_FORMAT(_1) ", " _JSON_FORMAT(_2) ", " _JSON_FORMAT(_3) ", " _JSON_FORMAT(_4) ", " _JSON_FORMAT(_5) ", " _JSON_FORMAT(_6) ", " _JSON_FORMAT(_7) ", " _JSON_FORMAT(_8)  \
_JSON_ARGS(_1) _JSON_ARGS(_2) _JSON_ARGS(_3) _JSON_ARGS(_4) _JSON_ARGS(_5) _JSON_ARGS(_6) _JSON_ARGS(_7) _JSON_ARGS(_8) 

#define JSON_LIST_9(_1, _2, _3, _4, _5, _6, _7, _8, _9) \
_JSON_FORMAT(_1) ", " _JSON_FORMAT(_2) ", " _JSON_FORMAT(_3) ", " _JSON_FORMAT(_4) ", " _JSON_FORMAT(_5) ", " _JSON_FORMAT(_6) ", " _JSON_FORMAT(_7) ", " _JSON_FORMAT(_8) ", " _JSON_FORMAT(_9) \
_JSON_ARGS(_1) _JSON_ARGS(_2) _JSON_ARGS(_3) _JSON_ARGS(_4) _JSON_ARGS(_5) _JSON_ARGS(_6) _JSON_ARGS(_7) _JSON_ARGS(_8) _JSON_ARGS(_9) 

#define JSON_LIST_10(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10) \
_JSON_FORMAT(_1) ", " _JSON_FORMAT(_2) ", " _JSON_FORMAT(_3) ", " _JSON_FORMAT(_4) ", " _JSON_FORMAT(_5) ", " _JSON_FORMAT(_6) ", " _JSON_FORMAT(_7) ", " _JSON_FORMAT(_8) ", " _JSON_FORMAT(_9) ", " _JSON_FORMAT(_10) \
_JSON_ARGS(_1) _JSON_ARGS(_2) _JSON_ARGS(_3) _JSON_ARGS(_4) _JSON_ARGS(_5) _JSON_ARGS(_6) _JSON_ARGS(_7) _JSON_ARGS(_8) _JSON_ARGS(_9) _JSON_ARGS(_10)

#define JSON_LIST_11(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11) \
_JSON_FORMAT(_1) ", " _JSON_FORMAT(_2) ", " _JSON_FORMAT(_3) ", " _JSON_FORMAT(_4) ", " _JSON_FORMAT(_5) ", " _JSON_FORMAT(_6) ", " _JSON_FORMAT(_7) ", " _JSON_FORMAT(_8) ", " _JSON_FORMAT(_9) ", " _JSON_FORMAT(_10) ", " _JSON_FORMAT(_11) \
_JSON_ARGS(_1) _JSON_ARGS(_2) _JSON_ARGS(_3) _JSON_ARGS(_4) _JSON_ARGS(_5) _JSON_ARGS(_6) _JSON_ARGS(_7) _JSON_ARGS(_8) _JSON_ARGS(_9) _JSON_ARGS(_10) _JSON_ARGS(_11)

#define JSON_LIST_12(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12) \
_JSON_FORMAT(_1) ", " _JSON_FORMAT(_2) ", " _JSON_FORMAT(_3) ", " _JSON_FORMAT(_4) ", " _JSON_FORMAT(_5) ", " _JSON_FORMAT(_6) ", " _JSON_FORMAT(_7) ", " _JSON_FORMAT(_8) ", " _JSON_FORMAT(_9) ", " _JSON_FORMAT(_10) ", " _JSON_FORMAT(_11) ", " _JSON_FORMAT(_12) \
_JSON_ARGS(_1) _JSON_ARGS(_2) _JSON_ARGS(_3) _JSON_ARGS(_4) _JSON_ARGS(_5) _JSON_ARGS(_6) _JSON_ARGS(_7) _JSON_ARGS(_8) _JSON_ARGS(_9) _JSON_ARGS(_10) _JSON_ARGS(_11) _JSON_ARGS(_12)

#define JSON_LIST_13(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13) \
_JSON_FORMAT(_1) ", " _JSON_FORMAT(_2) ", " _JSON_FORMAT(_3) ", " _JSON_FORMAT(_4) ", " _JSON_FORMAT(_5) ", " _JSON_FORMAT(_6) ", " _JSON_FORMAT(_7) ", " _JSON_FORMAT(_8) ", " _JSON_FORMAT(_9) ", " _JSON_FORMAT(_10) ", " _JSON_FORMAT(_11) ", " _JSON_FORMAT(_12) ", " _JSON_FORMAT(_13) \
_JSON_ARGS(_1) _JSON_ARGS(_2) _JSON_ARGS(_3) _JSON_ARGS(_4) _JSON_ARGS(_5) _JSON_ARGS(_6) _JSON_ARGS(_7) _JSON_ARGS(_8) _JSON_ARGS(_9) _JSON_ARGS(_10) _JSON_ARGS(_11) _JSON_ARGS(_12) _JSON_ARGS(_13)

#define JSON_LIST_14(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14) \
_JSON_FORMAT(_1) ", " _JSON_FORMAT(_2) ", " _JSON_FORMAT(_3) ", " _JSON_FORMAT(_4) ", " _JSON_FORMAT(_5) ", " _JSON_FORMAT(_6) ", " _JSON_FORMAT(_7) ", " _JSON_FORMAT(_8) ", " _JSON_FORMAT(_9) ", " _JSON_FORMAT(_10) ", " _JSON_FORMAT(_11) ", " _JSON_FORMAT(_12) ", " _JSON_FORMAT(_13) ", " _JSON_FORMAT(_14) \
_JSON_ARGS(_1) _JSON_ARGS(_2) _JSON_ARGS(_3) _JSON_ARGS(_4) _JSON_ARGS(_5) _JSON_ARGS(_6) _JSON_ARGS(_7) _JSON_ARGS(_8) _JSON_ARGS(_9) _JSON_ARGS(_10) _JSON_ARGS(_11) _JSON_ARGS(_12) _JSON_ARGS(_13) _JSON_ARGS(_14)

#define JSON_LIST_15(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15) \
_JSON_FORMAT(_1) ", " _JSON_FORMAT(_2) ", " _JSON_FORMAT(_3) ", " _JSON_FORMAT(_4) ", " _JSON_FORMAT(_5) ", " _JSON_FORMAT(_6) ", " _JSON_FORMAT(_7) ", " _JSON_FORMAT(_8) ", " _JSON_FORMAT(_9) ", " _JSON_FORMAT(_10) ", " _JSON_FORMAT(_11) ", " _JSON_FORMAT(_12) ", " _JSON_FORMAT(_13) ", " _JSON_FORMAT(_14) ", " _JSON_FORMAT(_15) \
_JSON_ARGS(_1) _JSON_ARGS(_2) _JSON_ARGS(_3) _JSON_ARGS(_4) _JSON_ARGS(_5) _JSON_ARGS(_6) _JSON_ARGS(_7) _JSON_ARGS(_8) _JSON_ARGS(_9) _JSON_ARGS(_10) _JSON_ARGS(_11) _JSON_ARGS(_12) _JSON_ARGS(_13) _JSON_ARGS(_14) _JSON_ARGS(_15)

#define JSON_LIST_16(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16) \
_JSON_FORMAT(_1) ", " _JSON_FORMAT(_2) ", " _JSON_FORMAT(_3) ", " _JSON_FORMAT(_4) ", " _JSON_FORMAT(_5) ", " _JSON_FORMAT(_6) ", " _JSON_FORMAT(_7) ", " _JSON_FORMAT(_8) ", " _JSON_FORMAT(_9) ", " _JSON_FORMAT(_10) ", " _JSON_FORMAT(_11) ", " _JSON_FORMAT(_12) ", " _JSON_FORMAT(_13) ", " _JSON_FORMAT(_14) ", " _JSON_FORMAT(_15) ", " _JSON_FORMAT(_16) \
_JSON_ARGS(_1) _JSON_ARGS(_2) _JSON_ARGS(_3) _JSON_ARGS(_4) _JSON_ARGS(_5) _JSON_ARGS(_6) _JSON_ARGS(_7) _JSON_ARGS(_8) _JSON_ARGS(_9) _JSON_ARGS(_10) _JSON_ARGS(_11) _JSON_ARGS(_12) _JSON_ARGS(_13) _JSON_ARGS(_14) _JSON_ARGS(_15) _JSON_ARGS(_16)

#define JSON_LIST_17(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17) \
_JSON_FORMAT(_1) ", " _JSON_FORMAT(_2) ", " _JSON_FORMAT(_3) ", " _JSON_FORMAT(_4) ", " _JSON_FORMAT(_5) ", " _JSON_FORMAT(_6) ", " _JSON_FORMAT(_7) ", " _JSON_FORMAT(_8) ", " _JSON_FORMAT(_9) ", " _JSON_FORMAT(_10) ", " _JSON_FORMAT(_11) ", " _JSON_FORMAT(_12) ", " _JSON_FORMAT(_13) ", " _JSON_FORMAT(_14) ", " _JSON_FORMAT(_15) ", " _JSON_FORMAT(_16) ", " _JSON_FORMAT(_17) \
_JSON_ARGS(_1) _JSON_ARGS(_2) _JSON_ARGS(_3) _JSON_ARGS(_4) _JSON_ARGS(_5) _JSON_ARGS(_6) _JSON_ARGS(_7) _JSON_ARGS(_8) _JSON_ARGS(_9) _JSON_ARGS(_10) _JSON_ARGS(_11) _JSON_ARGS(_12) _JSON_ARGS(_13) _JSON_ARGS(_14) _JSON_ARGS(_15) _JSON_ARGS(_16) _JSON_ARGS(_17)

#define JSON_LIST_18(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18) \
_JSON_FORMAT(_1) ", " _JSON_FORMAT(_2) ", " _JSON_FORMAT(_3) ", " _JSON_FORMAT(_4) ", " _JSON_FORMAT(_5) ", " _JSON_FORMAT(_6) ", " _JSON_FORMAT(_7) ", " _JSON_FORMAT(_8) ", " _JSON_FORMAT(_9) ", " _JSON_FORMAT(_10) ", " _JSON_FORMAT(_11) ", " _JSON_FORMAT(_12) ", " _JSON_FORMAT(_13) ", " _JSON_FORMAT(_14) ", " _JSON_FORMAT(_15) ", " _JSON_FORMAT(_16) ", " _JSON_FORMAT(_17) ", " _JSON_FORMAT(_18) \
_JSON_ARGS(_1) _JSON_ARGS(_2) _JSON_ARGS(_3) _JSON_ARGS(_4) _JSON_ARGS(_5) _JSON_ARGS(_6) _JSON_ARGS(_7) _JSON_ARGS(_8) _JSON_ARGS(_9) _JSON_ARGS(_10) _JSON_ARGS(_11) _JSON_ARGS(_12) _JSON_ARGS(_13) _JSON_ARGS(_14) _JSON_ARGS(_15) _JSON_ARGS(_16) _JSON_ARGS(_17) _JSON_ARGS(_18)

#define JSON_LIST_19(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19) \
_JSON_FORMAT(_1) ", " _JSON_FORMAT(_2) ", " _JSON_FORMAT(_3) ", " _JSON_FORMAT(_4) ", " _JSON_FORMAT(_5) ", " _JSON_FORMAT(_6) ", " _JSON_FORMAT(_7) ", " _JSON_FORMAT(_8) ", " _JSON_FORMAT(_9) ", " _JSON_FORMAT(_10) ", " _JSON_FORMAT(_11) ", " _JSON_FORMAT(_12) ", " _JSON_FORMAT(_13) ", " _JSON_FORMAT(_14) ", " _JSON_FORMAT(_15) ", " _JSON_FORMAT(_16) ", " _JSON_FORMAT(_17) ", " _JSON_FORMAT(_18) ", " _JSON_FORMAT(_19) \
_JSON_ARGS(_1) _JSON_ARGS(_2) _JSON_ARGS(_3) _JSON_ARGS(_4) _JSON_ARGS(_5) _JSON_ARGS(_6) _JSON_ARGS(_7) _JSON_ARGS(_8) _JSON_ARGS(_9) _JSON_ARGS(_10) _JSON_ARGS(_11) _JSON_ARGS(_12) _JSON_ARGS(_13) _JSON_ARGS(_14) _JSON_ARGS(_15) _JSON_ARGS(_16) _JSON_ARGS(_17) _JSON_ARGS(_18) _JSON_ARGS(_19)

#define JSON_LIST_20(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20) \
_JSON_FORMAT(_1) ", " _JSON_FORMAT(_2) ", " _JSON_FORMAT(_3) ", " _JSON_FORMAT(_4) ", " _JSON_FORMAT(_5) ", " _JSON_FORMAT(_6) ", " _JSON_FORMAT(_7) ", " _JSON_FORMAT(_8) ", " _JSON_FORMAT(_9) ", " _JSON_FORMAT(_10) ", " _JSON_FORMAT(_11) ", " _JSON_FORMAT(_12) ", " _JSON_FORMAT(_13) ", " _JSON_FORMAT(_14) ", " _JSON_FORMAT(_15) ", " _JSON_FORMAT(_16) ", " _JSON_FORMAT(_17) ", " _JSON_FORMAT(_18) ", " _JSON_FORMAT(_19) ", " _JSON_FORMAT(_20) \
_JSON_ARGS(_1) _JSON_ARGS(_2) _JSON_ARGS(_3) _JSON_ARGS(_4) _JSON_ARGS(_5) _JSON_ARGS(_6) _JSON_ARGS(_7) _JSON_ARGS(_8) _JSON_ARGS(_9) _JSON_ARGS(_10) _JSON_ARGS(_11) _JSON_ARGS(_12) _JSON_ARGS(_13) _JSON_ARGS(_14) _JSON_ARGS(_15) _JSON_ARGS(_16) _JSON_ARGS(_17) _JSON_ARGS(_18) _JSON_ARGS(_19) _JSON_ARGS(_20)

#define JSON_LIST_21(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21) \
_JSON_FORMAT(_1) ", " _JSON_FORMAT(_2) ", " _JSON_FORMAT(_3) ", " _JSON_FORMAT(_4) ", " _JSON_FORMAT(_5) ", " _JSON_FORMAT(_6) ", " _JSON_FORMAT(_7) ", " _JSON_FORMAT(_8) ", " _JSON_FORMAT(_9) ", " _JSON_FORMAT(_10) ", " _JSON_FORMAT(_11) ", " _JSON_FORMAT(_12) ", " _JSON_FORMAT(_13) ", " _JSON_FORMAT(_14) ", " _JSON_FORMAT(_15) ", " _JSON_FORMAT(_16) ", " _JSON_FORMAT(_17) ", " _JSON_FORMAT(_18) ", " _JSON_FORMAT(_19) ", " _JSON_FORMAT(_20) ", " _JSON_FORMAT(_21) \
_JSON_ARGS(_1) _JSON_ARGS(_2) _JSON_ARGS(_3) _JSON_ARGS(_4) _JSON_ARGS(_5) _JSON_ARGS(_6) _JSON_ARGS(_7) _JSON_ARGS(_8) _JSON_ARGS(_9) _JSON_ARGS(_10) _JSON_ARGS(_11) _JSON_ARGS(_12) _JSON_ARGS(_13) _JSON_ARGS(_14) _JSON_ARGS(_15) _JSON_ARGS(_16) _JSON_ARGS(_17) _JSON_ARGS(_18) _JSON_ARGS(_19) _JSON_ARGS(_20) _JSON_ARGS(_21)

#define JSON_LIST_22(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22) \
_JSON_FORMAT(_1) ", " _JSON_FORMAT(_2) ", " _JSON_FORMAT(_3) ", " _JSON_FORMAT(_4) ", " _JSON_FORMAT(_5) ", " _JSON_FORMAT(_6) ", " _JSON_FORMAT(_7) ", " _JSON_FORMAT(_8) ", " _JSON_FORMAT(_9) ", " _JSON_FORMAT(_10) ", " _JSON_FORMAT(_11) ", " _JSON_FORMAT(_12) ", " _JSON_FORMAT(_13) ", " _JSON_FORMAT(_14) ", " _JSON_FORMAT(_15) ", " _JSON_FORMAT(_16) ", " _JSON_FORMAT(_17) ", " _JSON_FORMAT(_18) ", " _JSON_FORMAT(_19) ", " _JSON_FORMAT(_20) ", " _JSON_FORMAT(_21) ", " _JSON_FORMAT(_22) \
_JSON_ARGS(_1) _JSON_ARGS(_2) _JSON_ARGS(_3) _JSON_ARGS(_4) _JSON_ARGS(_5) _JSON_ARGS(_6) _JSON_ARGS(_7) _JSON_ARGS(_8) _JSON_ARGS(_9) _JSON_ARGS(_10) _JSON_ARGS(_11) _JSON_ARGS(_12) _JSON_ARGS(_13) _JSON_ARGS(_14) _JSON_ARGS(_15) _JSON_ARGS(_16) _JSON_ARGS(_17) _JSON_ARGS(_18) _JSON_ARGS(_19) _JSON_ARGS(_20) _JSON_ARGS(_21) _JSON_ARGS(_22)

#define JSON_LIST_23(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23) \
_JSON_FORMAT(_1) ", " _JSON_FORMAT(_2) ", " _JSON_FORMAT(_3) ", " _JSON_FORMAT(_4) ", " _JSON_FORMAT(_5) ", " _JSON_FORMAT(_6) ", " _JSON_FORMAT(_7) ", " _JSON_FORMAT(_8) ", " _JSON_FORMAT(_9) ", " _JSON_FORMAT(_10) ", " _JSON_FORMAT(_11) ", " _JSON_FORMAT(_12) ", " _JSON_FORMAT(_13) ", " _JSON_FORMAT(_14) ", " _JSON_FORMAT(_15) ", " _JSON_FORMAT(_16) ", " _JSON_FORMAT(_17) ", " _JSON_FORMAT(_18) ", " _JSON_FORMAT(_19) ", " _JSON_FORMAT(_20) ", " _JSON_FORMAT(_21) ", " _JSON_FORMAT(_22) ", " _JSON_FORMAT(_23)\
_JSON_ARGS(_1) _JSON_ARGS(_2) _JSON_ARGS(_3) _JSON_ARGS(_4) _JSON_ARGS(_5) _JSON_ARGS(_6) _JSON_ARGS(_7) _JSON_ARGS(_8) _JSON_ARGS(_9) _JSON_ARGS(_10) _JSON_ARGS(_11) _JSON_ARGS(_12) _JSON_ARGS(_13) _JSON_ARGS(_14) _JSON_ARGS(_15) _JSON_ARGS(_16) _JSON_ARGS(_17) _JSON_ARGS(_18) _JSON_ARGS(_19) _JSON_ARGS(_20) _JSON_ARGS(_21) _JSON_ARGS(_22) _JSON_ARGS(_23) 

#define JSON_LIST_24(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24) \
_JSON_FORMAT(_1) ", " _JSON_FORMAT(_2) ", " _JSON_FORMAT(_3) ", " _JSON_FORMAT(_4) ", " _JSON_FORMAT(_5) ", " _JSON_FORMAT(_6) ", " _JSON_FORMAT(_7) ", " _JSON_FORMAT(_8) ", " _JSON_FORMAT(_9) ", " _JSON_FORMAT(_10) ", " _JSON_FORMAT(_11) ", " _JSON_FORMAT(_12) ", " _JSON_FORMAT(_13) ", " _JSON_FORMAT(_14) ", " _JSON_FORMAT(_15) ", " _JSON_FORMAT(_16) ", " _JSON_FORMAT(_17) ", " _JSON_FORMAT(_18) ", " _JSON_FORMAT(_19) ", " _JSON_FORMAT(_20) ", " _JSON_FORMAT(_21) ", " _JSON_FORMAT(_22) ", " _JSON_FORMAT(_23) ", " _JSON_FORMAT(_24) \
_JSON_ARGS(_1) _JSON_ARGS(_2) _JSON_ARGS(_3) _JSON_ARGS(_4) _JSON_ARGS(_5) _JSON_ARGS(_6) _JSON_ARGS(_7) _JSON_ARGS(_8) _JSON_ARGS(_9) _JSON_ARGS(_10) _JSON_ARGS(_11) _JSON_ARGS(_12) _JSON_ARGS(_13) _JSON_ARGS(_14) _JSON_ARGS(_15) _JSON_ARGS(_16) _JSON_ARGS(_17) _JSON_ARGS(_18) _JSON_ARGS(_19) _JSON_ARGS(_20) _JSON_ARGS(_21) _JSON_ARGS(_22) _JSON_ARGS(_23) _JSON_ARGS(_24)

#define JSON_LIST_25(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25) \
_JSON_FORMAT(_1) ", " _JSON_FORMAT(_2) ", " _JSON_FORMAT(_3) ", " _JSON_FORMAT(_4) ", " _JSON_FORMAT(_5) ", " _JSON_FORMAT(_6) ", " _JSON_FORMAT(_7) ", " _JSON_FORMAT(_8) ", " _JSON_FORMAT(_9) ", " _JSON_FORMAT(_10) ", " _JSON_FORMAT(_11) ", " _JSON_FORMAT(_12) ", " _JSON_FORMAT(_13) ", " _JSON_FORMAT(_14) ", " _JSON_FORMAT(_15) ", " _JSON_FORMAT(_16) ", " _JSON_FORMAT(_17) ", " _JSON_FORMAT(_18) ", " _JSON_FORMAT(_19) ", " _JSON_FORMAT(_20) ", " _JSON_FORMAT(_21) ", " _JSON_FORMAT(_22) ", " _JSON_FORMAT(_23) ", " _JSON_FORMAT(_24) ", " _JSON_FORMAT(_25) \
_JSON_ARGS(_1) _JSON_ARGS(_2) _JSON_ARGS(_3) _JSON_ARGS(_4) _JSON_ARGS(_5) _JSON_ARGS(_6) _JSON_ARGS(_7) _JSON_ARGS(_8) _JSON_ARGS(_9) _JSON_ARGS(_10) _JSON_ARGS(_11) _JSON_ARGS(_12) _JSON_ARGS(_13) _JSON_ARGS(_14) _JSON_ARGS(_15) _JSON_ARGS(_16) _JSON_ARGS(_17) _JSON_ARGS(_18) _JSON_ARGS(_19) _JSON_ARGS(_20) _JSON_ARGS(_21) _JSON_ARGS(_22) _JSON_ARGS(_23) _JSON_ARGS(_24) _JSON_ARGS(_25)

#define JSON_LIST_26(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26) \
_JSON_FORMAT(_1) ", " _JSON_FORMAT(_2) ", " _JSON_FORMAT(_3) ", " _JSON_FORMAT(_4) ", " _JSON_FORMAT(_5) ", " _JSON_FORMAT(_6) ", " _JSON_FORMAT(_7) ", " _JSON_FORMAT(_8) ", " _JSON_FORMAT(_9) ", " _JSON_FORMAT(_10) ", " _JSON_FORMAT(_11) ", " _JSON_FORMAT(_12) ", " _JSON_FORMAT(_13) ", " _JSON_FORMAT(_14) ", " _JSON_FORMAT(_15) ", " _JSON_FORMAT(_16) ", " _JSON_FORMAT(_17) ", " _JSON_FORMAT(_18) ", " _JSON_FORMAT(_19) ", " _JSON_FORMAT(_20) ", " _JSON_FORMAT(_21) ", " _JSON_FORMAT(_22) ", " _JSON_FORMAT(_23) ", " _JSON_FORMAT(_24) ", " _JSON_FORMAT(_25) ", " _JSON_FORMAT(_26) \
_JSON_ARGS(_1) _JSON_ARGS(_2) _JSON_ARGS(_3) _JSON_ARGS(_4) _JSON_ARGS(_5) _JSON_ARGS(_6) _JSON_ARGS(_7) _JSON_ARGS(_8) _JSON_ARGS(_9) _JSON_ARGS(_10) _JSON_ARGS(_11) _JSON_ARGS(_12) _JSON_ARGS(_13) _JSON_ARGS(_14) _JSON_ARGS(_15) _JSON_ARGS(_16) _JSON_ARGS(_17) _JSON_ARGS(_18) _JSON_ARGS(_19) _JSON_ARGS(_20) _JSON_ARGS(_21) _JSON_ARGS(_22) _JSON_ARGS(_23) _JSON_ARGS(_24) _JSON_ARGS(_25) _JSON_ARGS(_26)

#define JSON_LIST_27(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27) \
_JSON_FORMAT(_1) ", " _JSON_FORMAT(_2) ", " _JSON_FORMAT(_3) ", " _JSON_FORMAT(_4) ", " _JSON_FORMAT(_5) ", " _JSON_FORMAT(_6) ", " _JSON_FORMAT(_7) ", " _JSON_FORMAT(_8) ", " _JSON_FORMAT(_9) ", " _JSON_FORMAT(_10) ", " _JSON_FORMAT(_11) ", " _JSON_FORMAT(_12) ", " _JSON_FORMAT(_13) ", " _JSON_FORMAT(_14) ", " _JSON_FORMAT(_15) ", " _JSON_FORMAT(_16) ", " _JSON_FORMAT(_17) ", " _JSON_FORMAT(_18) ", " _JSON_FORMAT(_19) ", " _JSON_FORMAT(_20) ", " _JSON_FORMAT(_21) ", " _JSON_FORMAT(_22) ", " _JSON_FORMAT(_23) ", " _JSON_FORMAT(_24) ", " _JSON_FORMAT(_25) ", " _JSON_FORMAT(_26) ", " _JSON_FORMAT(_27) \
_JSON_ARGS(_1) _JSON_ARGS(_2) _JSON_ARGS(_3) _JSON_ARGS(_4) _JSON_ARGS(_5) _JSON_ARGS(_6) _JSON_ARGS(_7) _JSON_ARGS(_8) _JSON_ARGS(_9) _JSON_ARGS(_10) _JSON_ARGS(_11) _JSON_ARGS(_12) _JSON_ARGS(_13) _JSON_ARGS(_14) _JSON_ARGS(_15) _JSON_ARGS(_16) _JSON_ARGS(_17) _JSON_ARGS(_18) _JSON_ARGS(_19) _JSON_ARGS(_20) _JSON_ARGS(_21) _JSON_ARGS(_22) _JSON_ARGS(_23) _JSON_ARGS(_24) _JSON_ARGS(_25) _JSON_ARGS(_26) _JSON_ARGS(_27)

#define JSON_LIST_28(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28) \
_JSON_FORMAT(_1) ", " _JSON_FORMAT(_2) ", " _JSON_FORMAT(_3) ", " _JSON_FORMAT(_4) ", " _JSON_FORMAT(_5) ", " _JSON_FORMAT(_6) ", " _JSON_FORMAT(_7) ", " _JSON_FORMAT(_8) ", " _JSON_FORMAT(_9) ", " _JSON_FORMAT(_10) ", " _JSON_FORMAT(_11) ", " _JSON_FORMAT(_12) ", " _JSON_FORMAT(_13) ", " _JSON_FORMAT(_14) ", " _JSON_FORMAT(_15) ", " _JSON_FORMAT(_16) ", " _JSON_FORMAT(_17) ", " _JSON_FORMAT(_18) ", " _JSON_FORMAT(_19) ", " _JSON_FORMAT(_20) ", " _JSON_FORMAT(_21) ", " _JSON_FORMAT(_22) ", " _JSON_FORMAT(_23) ", " _JSON_FORMAT(_24) ", " _JSON_FORMAT(_25) ", " _JSON_FORMAT(_26) ", " _JSON_FORMAT(_27) ", " _JSON_FORMAT(_28) \
_JSON_ARGS(_1) _JSON_ARGS(_2) _JSON_ARGS(_3) _JSON_ARGS(_4) _JSON_ARGS(_5) _JSON_ARGS(_6) _JSON_ARGS(_7) _JSON_ARGS(_8) _JSON_ARGS(_9) _JSON_ARGS(_10) _JSON_ARGS(_11) _JSON_ARGS(_12) _JSON_ARGS(_13) _JSON_ARGS(_14) _JSON_ARGS(_15) _JSON_ARGS(_16) _JSON_ARGS(_17) _JSON_ARGS(_18) _JSON_ARGS(_19) _JSON_ARGS(_20) _JSON_ARGS(_21) _JSON_ARGS(_22) _JSON_ARGS(_23) _JSON_ARGS(_24) _JSON_ARGS(_25) _JSON_ARGS(_26) _JSON_ARGS(_27) _JSON_ARGS(_28)

#define JSON_LIST_29(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29) \
_JSON_FORMAT(_1) ", " _JSON_FORMAT(_2) ", " _JSON_FORMAT(_3) ", " _JSON_FORMAT(_4) ", " _JSON_FORMAT(_5) ", " _JSON_FORMAT(_6) ", " _JSON_FORMAT(_7) ", " _JSON_FORMAT(_8) ", " _JSON_FORMAT(_9) ", " _JSON_FORMAT(_10) ", " _JSON_FORMAT(_11) ", " _JSON_FORMAT(_12) ", " _JSON_FORMAT(_13) ", " _JSON_FORMAT(_14) ", " _JSON_FORMAT(_15) ", " _JSON_FORMAT(_16) ", " _JSON_FORMAT(_17) ", " _JSON_FORMAT(_18) ", " _JSON_FORMAT(_19) ", " _JSON_FORMAT(_20) ", " _JSON_FORMAT(_21) ", " _JSON_FORMAT(_22) ", " _JSON_FORMAT(_23) ", " _JSON_FORMAT(_24) ", " _JSON_FORMAT(_25) ", " _JSON_FORMAT(_26) ", " _JSON_FORMAT(_27) ", " _JSON_FORMAT(_28) ", " _JSON_FORMAT(_29) \
_JSON_ARGS(_1) _JSON_ARGS(_2) _JSON_ARGS(_3) _JSON_ARGS(_4) _JSON_ARGS(_5) _JSON_ARGS(_6) _JSON_ARGS(_7) _JSON_ARGS(_8) _JSON_ARGS(_9) _JSON_ARGS(_10) _JSON_ARGS(_11) _JSON_ARGS(_12) _JSON_ARGS(_13) _JSON_ARGS(_14) _JSON_ARGS(_15) _JSON_ARGS(_16) _JSON_ARGS(_17) _JSON_ARGS(_18) _JSON_ARGS(_19) _JSON_ARGS(_20) _JSON_ARGS(_21) _JSON_ARGS(_22) _JSON_ARGS(_23) _JSON_ARGS(_24) _JSON_ARGS(_25) _JSON_ARGS(_26) _JSON_ARGS(_27) _JSON_ARGS(_28) _JSON_ARGS(_29)

#define JSON_LIST_30(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30) \
_JSON_FORMAT(_1) ", " _JSON_FORMAT(_2) ", " _JSON_FORMAT(_3) ", " _JSON_FORMAT(_4) ", " _JSON_FORMAT(_5) ", " _JSON_FORMAT(_6) ", " _JSON_FORMAT(_7) ", " _JSON_FORMAT(_8) ", " _JSON_FORMAT(_9) ", " _JSON_FORMAT(_10) ", " _JSON_FORMAT(_11) ", " _JSON_FORMAT(_12) ", " _JSON_FORMAT(_13) ", " _JSON_FORMAT(_14) ", " _JSON_FORMAT(_15) ", " _JSON_FORMAT(_16) ", " _JSON_FORMAT(_17) ", " _JSON_FORMAT(_18) ", " _JSON_FORMAT(_19) ", " _JSON_FORMAT(_20) ", " _JSON_FORMAT(_21) ", " _JSON_FORMAT(_22) ", " _JSON_FORMAT(_23) ", " _JSON_FORMAT(_24) ", " _JSON_FORMAT(_25) ", " _JSON_FORMAT(_26) ", " _JSON_FORMAT(_27) ", " _JSON_FORMAT(_28) ", " _JSON_FORMAT(_29) ", " _JSON_FORMAT(_30) \
_JSON_ARGS(_1) _JSON_ARGS(_2) _JSON_ARGS(_3) _JSON_ARGS(_4) _JSON_ARGS(_5) _JSON_ARGS(_6) _JSON_ARGS(_7) _JSON_ARGS(_8) _JSON_ARGS(_9) _JSON_ARGS(_10) _JSON_ARGS(_11) _JSON_ARGS(_12) _JSON_ARGS(_13) _JSON_ARGS(_14) _JSON_ARGS(_15) _JSON_ARGS(_16) _JSON_ARGS(_17) _JSON_ARGS(_18) _JSON_ARGS(_19) _JSON_ARGS(_20) _JSON_ARGS(_21) _JSON_ARGS(_22) _JSON_ARGS(_23) _JSON_ARGS(_24) _JSON_ARGS(_25) _JSON_ARGS(_26) _JSON_ARGS(_27) _JSON_ARGS(_28) _JSON_ARGS(_29) _JSON_ARGS(_30)

#define JSON_LIST_31(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31) \
_JSON_FORMAT(_1) ", " _JSON_FORMAT(_2) ", " _JSON_FORMAT(_3) ", " _JSON_FORMAT(_4) ", " _JSON_FORMAT(_5) ", " _JSON_FORMAT(_6) ", " _JSON_FORMAT(_7) ", " _JSON_FORMAT(_8) ", " _JSON_FORMAT(_9) ", " _JSON_FORMAT(_10) ", " _JSON_FORMAT(_11) ", " _JSON_FORMAT(_12) ", " _JSON_FORMAT(_13) ", " _JSON_FORMAT(_14) ", " _JSON_FORMAT(_15) ", " _JSON_FORMAT(_16) ", " _JSON_FORMAT(_17) ", " _JSON_FORMAT(_18) ", " _JSON_FORMAT(_19) ", " _JSON_FORMAT(_20) ", " _JSON_FORMAT(_21) ", " _JSON_FORMAT(_22) ", " _JSON_FORMAT(_23) ", " _JSON_FORMAT(_24) ", " _JSON_FORMAT(_25) ", " _JSON_FORMAT(_26) ", " _JSON_FORMAT(_27) ", " _JSON_FORMAT(_28) ", " _JSON_FORMAT(_29) ", " _JSON_FORMAT(_30) ", " _JSON_FORMAT(_31) \
_JSON_ARGS(_1) _JSON_ARGS(_2) _JSON_ARGS(_3) _JSON_ARGS(_4) _JSON_ARGS(_5) _JSON_ARGS(_6) _JSON_ARGS(_7) _JSON_ARGS(_8) _JSON_ARGS(_9) _JSON_ARGS(_10) _JSON_ARGS(_11) _JSON_ARGS(_12) _JSON_ARGS(_13) _JSON_ARGS(_14) _JSON_ARGS(_15) _JSON_ARGS(_16) _JSON_ARGS(_17) _JSON_ARGS(_18) _JSON_ARGS(_19) _JSON_ARGS(_20) _JSON_ARGS(_21) _JSON_ARGS(_22) _JSON_ARGS(_23) _JSON_ARGS(_24) _JSON_ARGS(_25) _JSON_ARGS(_26) _JSON_ARGS(_27) _JSON_ARGS(_28) _JSON_ARGS(_29) _JSON_ARGS(_30) _JSON_ARGS(_31)

#define JSON_LIST_32(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32) \
_JSON_FORMAT(_1) ", " _JSON_FORMAT(_2) ", " _JSON_FORMAT(_3) ", " _JSON_FORMAT(_4) ", " _JSON_FORMAT(_5) ", " _JSON_FORMAT(_6) ", " _JSON_FORMAT(_7) ", " _JSON_FORMAT(_8) ", " _JSON_FORMAT(_9) ", " _JSON_FORMAT(_10) ", " _JSON_FORMAT(_11) ", " _JSON_FORMAT(_12) ", " _JSON_FORMAT(_13) ", " _JSON_FORMAT(_14) ", " _JSON_FORMAT(_15) ", " _JSON_FORMAT(_16) ", " _JSON_FORMAT(_17) ", " _JSON_FORMAT(_18) ", " _JSON_FORMAT(_19) ", " _JSON_FORMAT(_20) ", " _JSON_FORMAT(_21) ", " _JSON_FORMAT(_22) ", " _JSON_FORMAT(_23) ", " _JSON_FORMAT(_24) ", " _JSON_FORMAT(_25) ", " _JSON_FORMAT(_26) ", " _JSON_FORMAT(_27) ", " _JSON_FORMAT(_28) ", " _JSON_FORMAT(_29) ", " _JSON_FORMAT(_30) ", " _JSON_FORMAT(_31) ", " _JSON_FORMAT(_32) \
_JSON_ARGS(_1) _JSON_ARGS(_2) _JSON_ARGS(_3) _JSON_ARGS(_4) _JSON_ARGS(_5) _JSON_ARGS(_6) _JSON_ARGS(_7) _JSON_ARGS(_8) _JSON_ARGS(_9) _JSON_ARGS(_10) _JSON_ARGS(_11) _JSON_ARGS(_12) _JSON_ARGS(_13) _JSON_ARGS(_14) _JSON_ARGS(_15) _JSON_ARGS(_16) _JSON_ARGS(_17) _JSON_ARGS(_18) _JSON_ARGS(_19) _JSON_ARGS(_20) _JSON_ARGS(_21) _JSON_ARGS(_22) _JSON_ARGS(_23) _JSON_ARGS(_24) _JSON_ARGS(_25) _JSON_ARGS(_26) _JSON_ARGS(_27) _JSON_ARGS(_28) _JSON_ARGS(_29) _JSON_ARGS(_30) _JSON_ARGS(_31) _JSON_ARGS(_32)

#define JSON_LIST_33(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33) \
_JSON_FORMAT(_1) ", " _JSON_FORMAT(_2) ", " _JSON_FORMAT(_3) ", " _JSON_FORMAT(_4) ", " _JSON_FORMAT(_5) ", " _JSON_FORMAT(_6) ", " _JSON_FORMAT(_7) ", " _JSON_FORMAT(_8) ", " _JSON_FORMAT(_9) ", " _JSON_FORMAT(_10) ", " _JSON_FORMAT(_11) ", " _JSON_FORMAT(_12) ", " _JSON_FORMAT(_13) ", " _JSON_FORMAT(_14) ", " _JSON_FORMAT(_15) ", " _JSON_FORMAT(_16) ", " _JSON_FORMAT(_17) ", " _JSON_FORMAT(_18) ", " _JSON_FORMAT(_19) ", " _JSON_FORMAT(_20) ", " _JSON_FORMAT(_21) ", " _JSON_FORMAT(_22) ", " _JSON_FORMAT(_23) ", " _JSON_FORMAT(_24) ", " _JSON_FORMAT(_25) ", " _JSON_FORMAT(_26) ", " _JSON_FORMAT(_27) ", " _JSON_FORMAT(_28) ", " _JSON_FORMAT(_29) ", " _JSON_FORMAT(_30) ", " _JSON_FORMAT(_31) ", " _JSON_FORMAT(_32)  ", " _JSON_FORMAT(_33) \
_JSON_ARGS(_1) _JSON_ARGS(_2) _JSON_ARGS(_3) _JSON_ARGS(_4) _JSON_ARGS(_5) _JSON_ARGS(_6) _JSON_ARGS(_7) _JSON_ARGS(_8) _JSON_ARGS(_9) _JSON_ARGS(_10) _JSON_ARGS(_11) _JSON_ARGS(_12) _JSON_ARGS(_13) _JSON_ARGS(_14) _JSON_ARGS(_15) _JSON_ARGS(_16) _JSON_ARGS(_17) _JSON_ARGS(_18) _JSON_ARGS(_19) _JSON_ARGS(_20) _JSON_ARGS(_21) _JSON_ARGS(_22) _JSON_ARGS(_23) _JSON_ARGS(_24) _JSON_ARGS(_25) _JSON_ARGS(_26) _JSON_ARGS(_27) _JSON_ARGS(_28) _JSON_ARGS(_29) _JSON_ARGS(_30) _JSON_ARGS(_31) _JSON_ARGS(_32) _JSON_ARGS(_33)

#define JSON_LIST_34(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34) \
_JSON_FORMAT(_1) ", " _JSON_FORMAT(_2) ", " _JSON_FORMAT(_3) ", " _JSON_FORMAT(_4) ", " _JSON_FORMAT(_5) ", " _JSON_FORMAT(_6) ", " _JSON_FORMAT(_7) ", " _JSON_FORMAT(_8) ", " _JSON_FORMAT(_9) ", " _JSON_FORMAT(_10) ", " _JSON_FORMAT(_11) ", " _JSON_FORMAT(_12) ", " _JSON_FORMAT(_13) ", " _JSON_FORMAT(_14) ", " _JSON_FORMAT(_15) ", " _JSON_FORMAT(_16) ", " _JSON_FORMAT(_17) ", " _JSON_FORMAT(_18) ", " _JSON_FORMAT(_19) ", " _JSON_FORMAT(_20) ", " _JSON_FORMAT(_21) ", " _JSON_FORMAT(_22) ", " _JSON_FORMAT(_23) ", " _JSON_FORMAT(_24) ", " _JSON_FORMAT(_25) ", " _JSON_FORMAT(_26) ", " _JSON_FORMAT(_27) ", " _JSON_FORMAT(_28) ", " _JSON_FORMAT(_29) ", " _JSON_FORMAT(_30) ", " _JSON_FORMAT(_31) ", " _JSON_FORMAT(_32) ", " _JSON_FORMAT(_33) ", " _JSON_FORMAT(_34) \
_JSON_ARGS(_1) _JSON_ARGS(_2) _JSON_ARGS(_3) _JSON_ARGS(_4) _JSON_ARGS(_5) _JSON_ARGS(_6) _JSON_ARGS(_7) _JSON_ARGS(_8) _JSON_ARGS(_9) _JSON_ARGS(_10) _JSON_ARGS(_11) _JSON_ARGS(_12) _JSON_ARGS(_13) _JSON_ARGS(_14) _JSON_ARGS(_15) _JSON_ARGS(_16) _JSON_ARGS(_17) _JSON_ARGS(_18) _JSON_ARGS(_19) _JSON_ARGS(_20) _JSON_ARGS(_21) _JSON_ARGS(_22) _JSON_ARGS(_23) _JSON_ARGS(_24) _JSON_ARGS(_25) _JSON_ARGS(_26) _JSON_ARGS(_27) _JSON_ARGS(_28) _JSON_ARGS(_29) _JSON_ARGS(_30) _JSON_ARGS(_31) _JSON_ARGS(_32) _JSON_ARGS(_33) _JSON_ARGS(_34)

#define JSON_LIST_35(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35) \
_JSON_FORMAT(_1) ", " _JSON_FORMAT(_2) ", " _JSON_FORMAT(_3) ", " _JSON_FORMAT(_4) ", " _JSON_FORMAT(_5) ", " _JSON_FORMAT(_6) ", " _JSON_FORMAT(_7) ", " _JSON_FORMAT(_8) ", " _JSON_FORMAT(_9) ", " _JSON_FORMAT(_10) ", " _JSON_FORMAT(_11) ", " _JSON_FORMAT(_12) ", " _JSON_FORMAT(_13) ", " _JSON_FORMAT(_14) ", " _JSON_FORMAT(_15) ", " _JSON_FORMAT(_16) ", " _JSON_FORMAT(_17) ", " _JSON_FORMAT(_18) ", " _JSON_FORMAT(_19) ", " _JSON_FORMAT(_20) ", " _JSON_FORMAT(_21) ", " _JSON_FORMAT(_22) ", " _JSON_FORMAT(_23) ", " _JSON_FORMAT(_24) ", " _JSON_FORMAT(_25) ", " _JSON_FORMAT(_26) ", " _JSON_FORMAT(_27) ", " _JSON_FORMAT(_28) ", " _JSON_FORMAT(_29) ", " _JSON_FORMAT(_30) ", " _JSON_FORMAT(_31) ", " _JSON_FORMAT(_32) ", " _JSON_FORMAT(_33) ", " _JSON_FORMAT(_34) ", " _JSON_FORMAT(_35) \
_JSON_ARGS(_1) _JSON_ARGS(_2) _JSON_ARGS(_3) _JSON_ARGS(_4) _JSON_ARGS(_5) _JSON_ARGS(_6) _JSON_ARGS(_7) _JSON_ARGS(_8) _JSON_ARGS(_9) _JSON_ARGS(_10) _JSON_ARGS(_11) _JSON_ARGS(_12) _JSON_ARGS(_13) _JSON_ARGS(_14) _JSON_ARGS(_15) _JSON_ARGS(_16) _JSON_ARGS(_17) _JSON_ARGS(_18) _JSON_ARGS(_19) _JSON_ARGS(_20) _JSON_ARGS(_21) _JSON_ARGS(_22) _JSON_ARGS(_23) _JSON_ARGS(_24) _JSON_ARGS(_25) _JSON_ARGS(_26) _JSON_ARGS(_27) _JSON_ARGS(_28) _JSON_ARGS(_29) _JSON_ARGS(_30) _JSON_ARGS(_31) _JSON_ARGS(_32) _JSON_ARGS(_33) _JSON_ARGS(_34) _JSON_ARGS(_35)

#define JSON_LIST_36(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36) \
_JSON_FORMAT(_1) ", " _JSON_FORMAT(_2) ", " _JSON_FORMAT(_3) ", " _JSON_FORMAT(_4) ", " _JSON_FORMAT(_5) ", " _JSON_FORMAT(_6) ", " _JSON_FORMAT(_7) ", " _JSON_FORMAT(_8) ", " _JSON_FORMAT(_9) ", " _JSON_FORMAT(_10) ", " _JSON_FORMAT(_11) ", " _JSON_FORMAT(_12) ", " _JSON_FORMAT(_13) ", " _JSON_FORMAT(_14) ", " _JSON_FORMAT(_15) ", " _JSON_FORMAT(_16) ", " _JSON_FORMAT(_17) ", " _JSON_FORMAT(_18) ", " _JSON_FORMAT(_19) ", " _JSON_FORMAT(_20) ", " _JSON_FORMAT(_21) ", " _JSON_FORMAT(_22) ", " _JSON_FORMAT(_23) ", " _JSON_FORMAT(_24) ", " _JSON_FORMAT(_25) ", " _JSON_FORMAT(_26) ", " _JSON_FORMAT(_27) ", " _JSON_FORMAT(_28) ", " _JSON_FORMAT(_29) ", " _JSON_FORMAT(_30) ", " _JSON_FORMAT(_31) ", " _JSON_FORMAT(_32) ", " _JSON_FORMAT(_33) ", " _JSON_FORMAT(_34) ", " _JSON_FORMAT(_35) ", " _JSON_FORMAT(_36) \
_JSON_ARGS(_1) _JSON_ARGS(_2) _JSON_ARGS(_3) _JSON_ARGS(_4) _JSON_ARGS(_5) _JSON_ARGS(_6) _JSON_ARGS(_7) _JSON_ARGS(_8) _JSON_ARGS(_9) _JSON_ARGS(_10) _JSON_ARGS(_11) _JSON_ARGS(_12) _JSON_ARGS(_13) _JSON_ARGS(_14) _JSON_ARGS(_15) _JSON_ARGS(_16) _JSON_ARGS(_17) _JSON_ARGS(_18) _JSON_ARGS(_19) _JSON_ARGS(_20) _JSON_ARGS(_21) _JSON_ARGS(_22) _JSON_ARGS(_23) _JSON_ARGS(_24) _JSON_ARGS(_25) _JSON_ARGS(_26) _JSON_ARGS(_27) _JSON_ARGS(_28) _JSON_ARGS(_29) _JSON_ARGS(_30) _JSON_ARGS(_31) _JSON_ARGS(_32) _JSON_ARGS(_33) _JSON_ARGS(_34) _JSON_ARGS(_35) _JSON_ARGS(_36)

#define JSON_LIST_37(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37) \
_JSON_FORMAT(_1) ", " _JSON_FORMAT(_2) ", " _JSON_FORMAT(_3) ", " _JSON_FORMAT(_4) ", " _JSON_FORMAT(_5) ", " _JSON_FORMAT(_6) ", " _JSON_FORMAT(_7) ", " _JSON_FORMAT(_8) ", " _JSON_FORMAT(_9) ", " _JSON_FORMAT(_10) ", " _JSON_FORMAT(_11) ", " _JSON_FORMAT(_12) ", " _JSON_FORMAT(_13) ", " _JSON_FORMAT(_14) ", " _JSON_FORMAT(_15) ", " _JSON_FORMAT(_16) ", " _JSON_FORMAT(_17) ", " _JSON_FORMAT(_18) ", " _JSON_FORMAT(_19) ", " _JSON_FORMAT(_20) ", " _JSON_FORMAT(_21) ", " _JSON_FORMAT(_22) ", " _JSON_FORMAT(_23) ", " _JSON_FORMAT(_24) ", " _JSON_FORMAT(_25) ", " _JSON_FORMAT(_26) ", " _JSON_FORMAT(_27) ", " _JSON_FORMAT(_28) ", " _JSON_FORMAT(_29) ", " _JSON_FORMAT(_30) ", " _JSON_FORMAT(_31) ", " _JSON_FORMAT(_32) ", " _JSON_FORMAT(_33) ", " _JSON_FORMAT(_34) ", " _JSON_FORMAT(_35) ", " _JSON_FORMAT(_36) ", " _JSON_FORMAT(_37) \
_JSON_ARGS(_1) _JSON_ARGS(_2) _JSON_ARGS(_3) _JSON_ARGS(_4) _JSON_ARGS(_5) _JSON_ARGS(_6) _JSON_ARGS(_7) _JSON_ARGS(_8) _JSON_ARGS(_9) _JSON_ARGS(_10) _JSON_ARGS(_11) _JSON_ARGS(_12) _JSON_ARGS(_13) _JSON_ARGS(_14) _JSON_ARGS(_15) _JSON_ARGS(_16) _JSON_ARGS(_17) _JSON_ARGS(_18) _JSON_ARGS(_19) _JSON_ARGS(_20) _JSON_ARGS(_21) _JSON_ARGS(_22) _JSON_ARGS(_23) _JSON_ARGS(_24) _JSON_ARGS(_25) _JSON_ARGS(_26) _JSON_ARGS(_27) _JSON_ARGS(_28) _JSON_ARGS(_29) _JSON_ARGS(_30) _JSON_ARGS(_31) _JSON_ARGS(_32) _JSON_ARGS(_33) _JSON_ARGS(_34) _JSON_ARGS(_35) _JSON_ARGS(_36) _JSON_ARGS(_37)

#define JSON_LIST_38(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38) \
_JSON_FORMAT(_1) ", " _JSON_FORMAT(_2) ", " _JSON_FORMAT(_3) ", " _JSON_FORMAT(_4) ", " _JSON_FORMAT(_5) ", " _JSON_FORMAT(_6) ", " _JSON_FORMAT(_7) ", " _JSON_FORMAT(_8) ", " _JSON_FORMAT(_9) ", " _JSON_FORMAT(_10) ", " _JSON_FORMAT(_11) ", " _JSON_FORMAT(_12) ", " _JSON_FORMAT(_13) ", " _JSON_FORMAT(_14) ", " _JSON_FORMAT(_15) ", " _JSON_FORMAT(_16) ", " _JSON_FORMAT(_17) ", " _JSON_FORMAT(_18) ", " _JSON_FORMAT(_19) ", " _JSON_FORMAT(_20) ", " _JSON_FORMAT(_21) ", " _JSON_FORMAT(_22) ", " _JSON_FORMAT(_23) ", " _JSON_FORMAT(_24) ", " _JSON_FORMAT(_25) ", " _JSON_FORMAT(_26) ", " _JSON_FORMAT(_27) ", " _JSON_FORMAT(_28) ", " _JSON_FORMAT(_29) ", " _JSON_FORMAT(_30) ", " _JSON_FORMAT(_31) ", " _JSON_FORMAT(_32) ", " _JSON_FORMAT(_33) ", " _JSON_FORMAT(_34) ", " _JSON_FORMAT(_35) ", " _JSON_FORMAT(_36) ", " _JSON_FORMAT(_37) ", " _JSON_FORMAT(_38) \
_JSON_ARGS(_1) _JSON_ARGS(_2) _JSON_ARGS(_3) _JSON_ARGS(_4) _JSON_ARGS(_5) _JSON_ARGS(_6) _JSON_ARGS(_7) _JSON_ARGS(_8) _JSON_ARGS(_9) _JSON_ARGS(_10) _JSON_ARGS(_11) _JSON_ARGS(_12) _JSON_ARGS(_13) _JSON_ARGS(_14) _JSON_ARGS(_15) _JSON_ARGS(_16) _JSON_ARGS(_17) _JSON_ARGS(_18) _JSON_ARGS(_19) _JSON_ARGS(_20) _JSON_ARGS(_21) _JSON_ARGS(_22) _JSON_ARGS(_23) _JSON_ARGS(_24) _JSON_ARGS(_25) _JSON_ARGS(_26) _JSON_ARGS(_27) _JSON_ARGS(_28) _JSON_ARGS(_29) _JSON_ARGS(_30) _JSON_ARGS(_31) _JSON_ARGS(_32) _JSON_ARGS(_33) _JSON_ARGS(_34) _JSON_ARGS(_35) _JSON_ARGS(_36) _JSON_ARGS(_37) _JSON_ARGS(_38)

#define JSON_LIST_39(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39) \
_JSON_FORMAT(_1) ", " _JSON_FORMAT(_2) ", " _JSON_FORMAT(_3) ", " _JSON_FORMAT(_4) ", " _JSON_FORMAT(_5) ", " _JSON_FORMAT(_6) ", " _JSON_FORMAT(_7) ", " _JSON_FORMAT(_8) ", " _JSON_FORMAT(_9) ", " _JSON_FORMAT(_10) ", " _JSON_FORMAT(_11) ", " _JSON_FORMAT(_12) ", " _JSON_FORMAT(_13) ", " _JSON_FORMAT(_14) ", " _JSON_FORMAT(_15) ", " _JSON_FORMAT(_16) ", " _JSON_FORMAT(_17) ", " _JSON_FORMAT(_18) ", " _JSON_FORMAT(_19) ", " _JSON_FORMAT(_20) ", " _JSON_FORMAT(_21) ", " _JSON_FORMAT(_22) ", " _JSON_FORMAT(_23) ", " _JSON_FORMAT(_24) ", " _JSON_FORMAT(_25) ", " _JSON_FORMAT(_26) ", " _JSON_FORMAT(_27) ", " _JSON_FORMAT(_28) ", " _JSON_FORMAT(_29) ", " _JSON_FORMAT(_30) ", " _JSON_FORMAT(_31) ", " _JSON_FORMAT(_32) ", " _JSON_FORMAT(_33) ", " _JSON_FORMAT(_34) ", " _JSON_FORMAT(_35) ", " _JSON_FORMAT(_36) ", " _JSON_FORMAT(_37) ", " _JSON_FORMAT(_38) ", " _JSON_FORMAT(_39) \
_JSON_ARGS(_1) _JSON_ARGS(_2) _JSON_ARGS(_3) _JSON_ARGS(_4) _JSON_ARGS(_5) _JSON_ARGS(_6) _JSON_ARGS(_7) _JSON_ARGS(_8) _JSON_ARGS(_9) _JSON_ARGS(_10) _JSON_ARGS(_11) _JSON_ARGS(_12) _JSON_ARGS(_13) _JSON_ARGS(_14) _JSON_ARGS(_15) _JSON_ARGS(_16) _JSON_ARGS(_17) _JSON_ARGS(_18) _JSON_ARGS(_19) _JSON_ARGS(_20) _JSON_ARGS(_21) _JSON_ARGS(_22) _JSON_ARGS(_23) _JSON_ARGS(_24) _JSON_ARGS(_25) _JSON_ARGS(_26) _JSON_ARGS(_27) _JSON_ARGS(_28) _JSON_ARGS(_29) _JSON_ARGS(_30) _JSON_ARGS(_31) _JSON_ARGS(_32) _JSON_ARGS(_33) _JSON_ARGS(_34) _JSON_ARGS(_35) _JSON_ARGS(_36) _JSON_ARGS(_37) _JSON_ARGS(_38) _JSON_ARGS(_39)

#define JSON_LIST_40(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40) \
_JSON_FORMAT(_1) ", " _JSON_FORMAT(_2) ", " _JSON_FORMAT(_3) ", " _JSON_FORMAT(_4) ", " _JSON_FORMAT(_5) ", " _JSON_FORMAT(_6) ", " _JSON_FORMAT(_7) ", " _JSON_FORMAT(_8) ", " _JSON_FORMAT(_9) ", " _JSON_FORMAT(_10) ", " _JSON_FORMAT(_11) ", " _JSON_FORMAT(_12) ", " _JSON_FORMAT(_13) ", " _JSON_FORMAT(_14) ", " _JSON_FORMAT(_15) ", " _JSON_FORMAT(_16) ", " _JSON_FORMAT(_17) ", " _JSON_FORMAT(_18) ", " _JSON_FORMAT(_19) ", " _JSON_FORMAT(_20) ", " _JSON_FORMAT(_21) ", " _JSON_FORMAT(_22) ", " _JSON_FORMAT(_23) ", " _JSON_FORMAT(_24) ", " _JSON_FORMAT(_25) ", " _JSON_FORMAT(_26) ", " _JSON_FORMAT(_27) ", " _JSON_FORMAT(_28) ", " _JSON_FORMAT(_29) ", " _JSON_FORMAT(_30) ", " _JSON_FORMAT(_31) ", " _JSON_FORMAT(_32) ", " _JSON_FORMAT(_33) ", " _JSON_FORMAT(_34) ", " _JSON_FORMAT(_35) ", " _JSON_FORMAT(_36) ", " _JSON_FORMAT(_37) ", " _JSON_FORMAT(_38) ", " _JSON_FORMAT(_39) ", " _JSON_FORMAT(_40) \
_JSON_ARGS(_1) _JSON_ARGS(_2) _JSON_ARGS(_3) _JSON_ARGS(_4) _JSON_ARGS(_5) _JSON_ARGS(_6) _JSON_ARGS(_7) _JSON_ARGS(_8) _JSON_ARGS(_9) _JSON_ARGS(_10) _JSON_ARGS(_11) _JSON_ARGS(_12) _JSON_ARGS(_13) _JSON_ARGS(_14) _JSON_ARGS(_15) _JSON_ARGS(_16) _JSON_ARGS(_17) _JSON_ARGS(_18) _JSON_ARGS(_19) _JSON_ARGS(_20) _JSON_ARGS(_21) _JSON_ARGS(_22) _JSON_ARGS(_23) _JSON_ARGS(_24) _JSON_ARGS(_25) _JSON_ARGS(_26) _JSON_ARGS(_27) _JSON_ARGS(_28) _JSON_ARGS(_29) _JSON_ARGS(_30) _JSON_ARGS(_31) _JSON_ARGS(_32) _JSON_ARGS(_33) _JSON_ARGS(_34) _JSON_ARGS(_35) _JSON_ARGS(_36) _JSON_ARGS(_37) _JSON_ARGS(_38) _JSON_ARGS(_39) _JSON_ARGS(_40)

#define JSON_LIST_41(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41) \
_JSON_FORMAT(_1) ", " _JSON_FORMAT(_2) ", " _JSON_FORMAT(_3) ", " _JSON_FORMAT(_4) ", " _JSON_FORMAT(_5) ", " _JSON_FORMAT(_6) ", " _JSON_FORMAT(_7) ", " _JSON_FORMAT(_8) ", " _JSON_FORMAT(_9) ", " _JSON_FORMAT(_10) ", " _JSON_FORMAT(_11) ", " _JSON_FORMAT(_12) ", " _JSON_FORMAT(_13) ", " _JSON_FORMAT(_14) ", " _JSON_FORMAT(_15) ", " _JSON_FORMAT(_16) ", " _JSON_FORMAT(_17) ", " _JSON_FORMAT(_18) ", " _JSON_FORMAT(_19) ", " _JSON_FORMAT(_20) ", " _JSON_FORMAT(_21) ", " _JSON_FORMAT(_22) ", " _JSON_FORMAT(_23) ", " _JSON_FORMAT(_24) ", " _JSON_FORMAT(_25) ", " _JSON_FORMAT(_26) ", " _JSON_FORMAT(_27) ", " _JSON_FORMAT(_28) ", " _JSON_FORMAT(_29) ", " _JSON_FORMAT(_30) ", " _JSON_FORMAT(_31) ", " _JSON_FORMAT(_32) ", " _JSON_FORMAT(_33) ", " _JSON_FORMAT(_34) ", " _JSON_FORMAT(_35) ", " _JSON_FORMAT(_36) ", " _JSON_FORMAT(_37) ", " _JSON_FORMAT(_38) ", " _JSON_FORMAT(_39) ", " _JSON_FORMAT(_40) ", " _JSON_FORMAT(_41) \
_JSON_ARGS(_1) _JSON_ARGS(_2) _JSON_ARGS(_3) _JSON_ARGS(_4) _JSON_ARGS(_5) _JSON_ARGS(_6) _JSON_ARGS(_7) _JSON_ARGS(_8) _JSON_ARGS(_9) _JSON_ARGS(_10) _JSON_ARGS(_11) _JSON_ARGS(_12) _JSON_ARGS(_13) _JSON_ARGS(_14) _JSON_ARGS(_15) _JSON_ARGS(_16) _JSON_ARGS(_17) _JSON_ARGS(_18) _JSON_ARGS(_19) _JSON_ARGS(_20) _JSON_ARGS(_21) _JSON_ARGS(_22) _JSON_ARGS(_23) _JSON_ARGS(_24) _JSON_ARGS(_25) _JSON_ARGS(_26) _JSON_ARGS(_27) _JSON_ARGS(_28) _JSON_ARGS(_29) _JSON_ARGS(_30) _JSON_ARGS(_31) _JSON_ARGS(_32) _JSON_ARGS(_33) _JSON_ARGS(_34) _JSON_ARGS(_35) _JSON_ARGS(_36) _JSON_ARGS(_37) _JSON_ARGS(_38) _JSON_ARGS(_39) _JSON_ARGS(_40) _JSON_ARGS(_41)

#define JSON_LIST_42(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42) \
_JSON_FORMAT(_1) ", " _JSON_FORMAT(_2) ", " _JSON_FORMAT(_3) ", " _JSON_FORMAT(_4) ", " _JSON_FORMAT(_5) ", " _JSON_FORMAT(_6) ", " _JSON_FORMAT(_7) ", " _JSON_FORMAT(_8) ", " _JSON_FORMAT(_9) ", " _JSON_FORMAT(_10) ", " _JSON_FORMAT(_11) ", " _JSON_FORMAT(_12) ", " _JSON_FORMAT(_13) ", " _JSON_FORMAT(_14) ", " _JSON_FORMAT(_15) ", " _JSON_FORMAT(_16) ", " _JSON_FORMAT(_17) ", " _JSON_FORMAT(_18) ", " _JSON_FORMAT(_19) ", " _JSON_FORMAT(_20) ", " _JSON_FORMAT(_21) ", " _JSON_FORMAT(_22) ", " _JSON_FORMAT(_23) ", " _JSON_FORMAT(_24) ", " _JSON_FORMAT(_25) ", " _JSON_FORMAT(_26) ", " _JSON_FORMAT(_27) ", " _JSON_FORMAT(_28) ", " _JSON_FORMAT(_29) ", " _JSON_FORMAT(_30) ", " _JSON_FORMAT(_31) ", " _JSON_FORMAT(_32) ", " _JSON_FORMAT(_33) ", " _JSON_FORMAT(_34) ", " _JSON_FORMAT(_35) ", " _JSON_FORMAT(_36) ", " _JSON_FORMAT(_37) ", " _JSON_FORMAT(_38) ", " _JSON_FORMAT(_39) ", " _JSON_FORMAT(_40) ", " _JSON_FORMAT(_41) ", " _JSON_FORMAT(_42) \
_JSON_ARGS(_1) _JSON_ARGS(_2) _JSON_ARGS(_3) _JSON_ARGS(_4) _JSON_ARGS(_5) _JSON_ARGS(_6) _JSON_ARGS(_7) _JSON_ARGS(_8) _JSON_ARGS(_9) _JSON_ARGS(_10) _JSON_ARGS(_11) _JSON_ARGS(_12) _JSON_ARGS(_13) _JSON_ARGS(_14) _JSON_ARGS(_15) _JSON_ARGS(_16) _JSON_ARGS(_17) _JSON_ARGS(_18) _JSON_ARGS(_19) _JSON_ARGS(_20) _JSON_ARGS(_21) _JSON_ARGS(_22) _JSON_ARGS(_23) _JSON_ARGS(_24) _JSON_ARGS(_25) _JSON_ARGS(_26) _JSON_ARGS(_27) _JSON_ARGS(_28) _JSON_ARGS(_29) _JSON_ARGS(_30) _JSON_ARGS(_31) _JSON_ARGS(_32) _JSON_ARGS(_33) _JSON_ARGS(_34) _JSON_ARGS(_35) _JSON_ARGS(_36) _JSON_ARGS(_37) _JSON_ARGS(_38) _JSON_ARGS(_39) _JSON_ARGS(_40) _JSON_ARGS(_41) _JSON_ARGS(_42)

#define JSON_LIST_43(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43) \
_JSON_FORMAT(_1) ", " _JSON_FORMAT(_2) ", " _JSON_FORMAT(_3) ", " _JSON_FORMAT(_4) ", " _JSON_FORMAT(_5) ", " _JSON_FORMAT(_6) ", " _JSON_FORMAT(_7) ", " _JSON_FORMAT(_8) ", " _JSON_FORMAT(_9) ", " _JSON_FORMAT(_10) ", " _JSON_FORMAT(_11) ", " _JSON_FORMAT(_12) ", " _JSON_FORMAT(_13) ", " _JSON_FORMAT(_14) ", " _JSON_FORMAT(_15) ", " _JSON_FORMAT(_16) ", " _JSON_FORMAT(_17) ", " _JSON_FORMAT(_18) ", " _JSON_FORMAT(_19) ", " _JSON_FORMAT(_20) ", " _JSON_FORMAT(_21) ", " _JSON_FORMAT(_22) ", " _JSON_FORMAT(_23) ", " _JSON_FORMAT(_24) ", " _JSON_FORMAT(_25) ", " _JSON_FORMAT(_26) ", " _JSON_FORMAT(_27) ", " _JSON_FORMAT(_28) ", " _JSON_FORMAT(_29) ", " _JSON_FORMAT(_30) ", " _JSON_FORMAT(_31) ", " _JSON_FORMAT(_32) ", " _JSON_FORMAT(_33) ", " _JSON_FORMAT(_34) ", " _JSON_FORMAT(_35) ", " _JSON_FORMAT(_36) ", " _JSON_FORMAT(_37) ", " _JSON_FORMAT(_38) ", " _JSON_FORMAT(_39) ", " _JSON_FORMAT(_40) ", " _JSON_FORMAT(_41) ", " _JSON_FORMAT(_42) ", " _JSON_FORMAT(_43) \
_JSON_ARGS(_1) _JSON_ARGS(_2) _JSON_ARGS(_3) _JSON_ARGS(_4) _JSON_ARGS(_5) _JSON_ARGS(_6) _JSON_ARGS(_7) _JSON_ARGS(_8) _JSON_ARGS(_9) _JSON_ARGS(_10) _JSON_ARGS(_11) _JSON_ARGS(_12) _JSON_ARGS(_13) _JSON_ARGS(_14) _JSON_ARGS(_15) _JSON_ARGS(_16) _JSON_ARGS(_17) _JSON_ARGS(_18) _JSON_ARGS(_19) _JSON_ARGS(_20) _JSON_ARGS(_21) _JSON_ARGS(_22) _JSON_ARGS(_23) _JSON_ARGS(_24) _JSON_ARGS(_25) _JSON_ARGS(_26) _JSON_ARGS(_27) _JSON_ARGS(_28) _JSON_ARGS(_29) _JSON_ARGS(_30) _JSON_ARGS(_31) _JSON_ARGS(_32) _JSON_ARGS(_33) _JSON_ARGS(_34) _JSON_ARGS(_35) _JSON_ARGS(_36) _JSON_ARGS(_37) _JSON_ARGS(_38) _JSON_ARGS(_39) _JSON_ARGS(_40) _JSON_ARGS(_41) _JSON_ARGS(_42) _JSON_ARGS(_43)

#define JSON_LIST_44(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44) \
_JSON_FORMAT(_1) ", " _JSON_FORMAT(_2) ", " _JSON_FORMAT(_3) ", " _JSON_FORMAT(_4) ", " _JSON_FORMAT(_5) ", " _JSON_FORMAT(_6) ", " _JSON_FORMAT(_7) ", " _JSON_FORMAT(_8) ", " _JSON_FORMAT(_9) ", " _JSON_FORMAT(_10) ", " _JSON_FORMAT(_11) ", " _JSON_FORMAT(_12) ", " _JSON_FORMAT(_13) ", " _JSON_FORMAT(_14) ", " _JSON_FORMAT(_15) ", " _JSON_FORMAT(_16) ", " _JSON_FORMAT(_17) ", " _JSON_FORMAT(_18) ", " _JSON_FORMAT(_19) ", " _JSON_FORMAT(_20) ", " _JSON_FORMAT(_21) ", " _JSON_FORMAT(_22) ", " _JSON_FORMAT(_23) ", " _JSON_FORMAT(_24) ", " _JSON_FORMAT(_25) ", " _JSON_FORMAT(_26) ", " _JSON_FORMAT(_27) ", " _JSON_FORMAT(_28) ", " _JSON_FORMAT(_29) ", " _JSON_FORMAT(_30) ", " _JSON_FORMAT(_31) ", " _JSON_FORMAT(_32) ", " _JSON_FORMAT(_33) ", " _JSON_FORMAT(_34) ", " _JSON_FORMAT(_35) ", " _JSON_FORMAT(_36) ", " _JSON_FORMAT(_37) ", " _JSON_FORMAT(_38) ", " _JSON_FORMAT(_39) ", " _JSON_FORMAT(_40) ", " _JSON_FORMAT(_41) ", " _JSON_FORMAT(_42) ", " _JSON_FORMAT(_43) ", " _JSON_FORMAT(_44) \
_JSON_ARGS(_1) _JSON_ARGS(_2) _JSON_ARGS(_3) _JSON_ARGS(_4) _JSON_ARGS(_5) _JSON_ARGS(_6) _JSON_ARGS(_7) _JSON_ARGS(_8) _JSON_ARGS(_9) _JSON_ARGS(_10) _JSON_ARGS(_11) _JSON_ARGS(_12) _JSON_ARGS(_13) _JSON_ARGS(_14) _JSON_ARGS(_15) _JSON_ARGS(_16) _JSON_ARGS(_17) _JSON_ARGS(_18) _JSON_ARGS(_19) _JSON_ARGS(_20) _JSON_ARGS(_21) _JSON_ARGS(_22) _JSON_ARGS(_23) _JSON_ARGS(_24) _JSON_ARGS(_25) _JSON_ARGS(_26) _JSON_ARGS(_27) _JSON_ARGS(_28) _JSON_ARGS(_29) _JSON_ARGS(_30) _JSON_ARGS(_31) _JSON_ARGS(_32) _JSON_ARGS(_33) _JSON_ARGS(_34) _JSON_ARGS(_35) _JSON_ARGS(_36) _JSON_ARGS(_37) _JSON_ARGS(_38) _JSON_ARGS(_39) _JSON_ARGS(_40) _JSON_ARGS(_41) _JSON_ARGS(_42) _JSON_ARGS(_43) _JSON_ARGS(_44)

#define JSON_LIST_45(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45) \
_JSON_FORMAT(_1) ", " _JSON_FORMAT(_2) ", " _JSON_FORMAT(_3) ", " _JSON_FORMAT(_4) ", " _JSON_FORMAT(_5) ", " _JSON_FORMAT(_6) ", " _JSON_FORMAT(_7) ", " _JSON_FORMAT(_8) ", " _JSON_FORMAT(_9) ", " _JSON_FORMAT(_10) ", " _JSON_FORMAT(_11) ", " _JSON_FORMAT(_12) ", " _JSON_FORMAT(_13) ", " _JSON_FORMAT(_14) ", " _JSON_FORMAT(_15) ", " _JSON_FORMAT(_16) ", " _JSON_FORMAT(_17) ", " _JSON_FORMAT(_18) ", " _JSON_FORMAT(_19) ", " _JSON_FORMAT(_20) ", " _JSON_FORMAT(_21) ", " _JSON_FORMAT(_22) ", " _JSON_FORMAT(_23) ", " _JSON_FORMAT(_24) ", " _JSON_FORMAT(_25) ", " _JSON_FORMAT(_26) ", " _JSON_FORMAT(_27) ", " _JSON_FORMAT(_28) ", " _JSON_FORMAT(_29) ", " _JSON_FORMAT(_30) ", " _JSON_FORMAT(_31) ", " _JSON_FORMAT(_32) ", " _JSON_FORMAT(_33) ", " _JSON_FORMAT(_34) ", " _JSON_FORMAT(_35) ", " _JSON_FORMAT(_36) ", " _JSON_FORMAT(_37) ", " _JSON_FORMAT(_38) ", " _JSON_FORMAT(_39) ", " _JSON_FORMAT(_40) ", " _JSON_FORMAT(_41) ", " _JSON_FORMAT(_42) ", " _JSON_FORMAT(_43) ", " _JSON_FORMAT(_44) ", " _JSON_FORMAT(_45) \
_JSON_ARGS(_1) _JSON_ARGS(_2) _JSON_ARGS(_3) _JSON_ARGS(_4) _JSON_ARGS(_5) _JSON_ARGS(_6) _JSON_ARGS(_7) _JSON_ARGS(_8) _JSON_ARGS(_9) _JSON_ARGS(_10) _JSON_ARGS(_11) _JSON_ARGS(_12) _JSON_ARGS(_13) _JSON_ARGS(_14) _JSON_ARGS(_15) _JSON_ARGS(_16) _JSON_ARGS(_17) _JSON_ARGS(_18) _JSON_ARGS(_19) _JSON_ARGS(_20) _JSON_ARGS(_21) _JSON_ARGS(_22) _JSON_ARGS(_23) _JSON_ARGS(_24) _JSON_ARGS(_25) _JSON_ARGS(_26) _JSON_ARGS(_27) _JSON_ARGS(_28) _JSON_ARGS(_29) _JSON_ARGS(_30) _JSON_ARGS(_31) _JSON_ARGS(_32) _JSON_ARGS(_33) _JSON_ARGS(_34) _JSON_ARGS(_35) _JSON_ARGS(_36) _JSON_ARGS(_37) _JSON_ARGS(_38) _JSON_ARGS(_39) _JSON_ARGS(_40) _JSON_ARGS(_41) _JSON_ARGS(_42) _JSON_ARGS(_43) _JSON_ARGS(_44) _JSON_ARGS(_45)

#define JSON_LIST_46(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46) \
_JSON_FORMAT(_1) ", " _JSON_FORMAT(_2) ", " _JSON_FORMAT(_3) ", " _JSON_FORMAT(_4) ", " _JSON_FORMAT(_5) ", " _JSON_FORMAT(_6) ", " _JSON_FORMAT(_7) ", " _JSON_FORMAT(_8) ", " _JSON_FORMAT(_9) ", " _JSON_FORMAT(_10) ", " _JSON_FORMAT(_11) ", " _JSON_FORMAT(_12) ", " _JSON_FORMAT(_13) ", " _JSON_FORMAT(_14) ", " _JSON_FORMAT(_15) ", " _JSON_FORMAT(_16) ", " _JSON_FORMAT(_17) ", " _JSON_FORMAT(_18) ", " _JSON_FORMAT(_19) ", " _JSON_FORMAT(_20) ", " _JSON_FORMAT(_21) ", " _JSON_FORMAT(_22) ", " _JSON_FORMAT(_23) ", " _JSON_FORMAT(_24) ", " _JSON_FORMAT(_25) ", " _JSON_FORMAT(_26) ", " _JSON_FORMAT(_27) ", " _JSON_FORMAT(_28) ", " _JSON_FORMAT(_29) ", " _JSON_FORMAT(_30) ", " _JSON_FORMAT(_31) ", " _JSON_FORMAT(_32) ", " _JSON_FORMAT(_33) ", " _JSON_FORMAT(_34) ", " _JSON_FORMAT(_35) ", " _JSON_FORMAT(_36) ", " _JSON_FORMAT(_37) ", " _JSON_FORMAT(_38) ", " _JSON_FORMAT(_39) ", " _JSON_FORMAT(_40) ", " _JSON_FORMAT(_41) ", " _JSON_FORMAT(_42) ", " _JSON_FORMAT(_43) ", " _JSON_FORMAT(_44) ", " _JSON_FORMAT(_45) ", " _JSON_FORMAT(_46) \
_JSON_ARGS(_1) _JSON_ARGS(_2) _JSON_ARGS(_3) _JSON_ARGS(_4) _JSON_ARGS(_5) _JSON_ARGS(_6) _JSON_ARGS(_7) _JSON_ARGS(_8) _JSON_ARGS(_9) _JSON_ARGS(_10) _JSON_ARGS(_11) _JSON_ARGS(_12) _JSON_ARGS(_13) _JSON_ARGS(_14) _JSON_ARGS(_15) _JSON_ARGS(_16) _JSON_ARGS(_17) _JSON_ARGS(_18) _JSON_ARGS(_19) _JSON_ARGS(_20) _JSON_ARGS(_21) _JSON_ARGS(_22) _JSON_ARGS(_23) _JSON_ARGS(_24) _JSON_ARGS(_25) _JSON_ARGS(_26) _JSON_ARGS(_27) _JSON_ARGS(_28) _JSON_ARGS(_29) _JSON_ARGS(_30) _JSON_ARGS(_31) _JSON_ARGS(_32) _JSON_ARGS(_33) _JSON_ARGS(_34) _JSON_ARGS(_35) _JSON_ARGS(_36) _JSON_ARGS(_37) _JSON_ARGS(_38) _JSON_ARGS(_39) _JSON_ARGS(_40) _JSON_ARGS(_41) _JSON_ARGS(_42) _JSON_ARGS(_43) _JSON_ARGS(_44) _JSON_ARGS(_45) _JSON_ARGS(_46)

#define JSON_LIST_47(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47) \
_JSON_FORMAT(_1) ", " _JSON_FORMAT(_2) ", " _JSON_FORMAT(_3) ", " _JSON_FORMAT(_4) ", " _JSON_FORMAT(_5) ", " _JSON_FORMAT(_6) ", " _JSON_FORMAT(_7) ", " _JSON_FORMAT(_8) ", " _JSON_FORMAT(_9) ", " _JSON_FORMAT(_10) ", " _JSON_FORMAT(_11) ", " _JSON_FORMAT(_12) ", " _JSON_FORMAT(_13) ", " _JSON_FORMAT(_14) ", " _JSON_FORMAT(_15) ", " _JSON_FORMAT(_16) ", " _JSON_FORMAT(_17) ", " _JSON_FORMAT(_18) ", " _JSON_FORMAT(_19) ", " _JSON_FORMAT(_20) ", " _JSON_FORMAT(_21) ", " _JSON_FORMAT(_22) ", " _JSON_FORMAT(_23) ", " _JSON_FORMAT(_24) ", " _JSON_FORMAT(_25) ", " _JSON_FORMAT(_26) ", " _JSON_FORMAT(_27) ", " _JSON_FORMAT(_28) ", " _JSON_FORMAT(_29) ", " _JSON_FORMAT(_30) ", " _JSON_FORMAT(_31) ", " _JSON_FORMAT(_32) ", " _JSON_FORMAT(_33) ", " _JSON_FORMAT(_34) ", " _JSON_FORMAT(_35) ", " _JSON_FORMAT(_36) ", " _JSON_FORMAT(_37) ", " _JSON_FORMAT(_38) ", " _JSON_FORMAT(_39) ", " _JSON_FORMAT(_40) ", " _JSON_FORMAT(_41) ", " _JSON_FORMAT(_42) ", " _JSON_FORMAT(_43) ", " _JSON_FORMAT(_44) ", " _JSON_FORMAT(_45) ", " _JSON_FORMAT(_46) ", " _JSON_FORMAT(_47) \
_JSON_ARGS(_1) _JSON_ARGS(_2) _JSON_ARGS(_3) _JSON_ARGS(_4) _JSON_ARGS(_5) _JSON_ARGS(_6) _JSON_ARGS(_7) _JSON_ARGS(_8) _JSON_ARGS(_9) _JSON_ARGS(_10) _JSON_ARGS(_11) _JSON_ARGS(_12) _JSON_ARGS(_13) _JSON_ARGS(_14) _JSON_ARGS(_15) _JSON_ARGS(_16) _JSON_ARGS(_17) _JSON_ARGS(_18) _JSON_ARGS(_19) _JSON_ARGS(_20) _JSON_ARGS(_21) _JSON_ARGS(_22) _JSON_ARGS(_23) _JSON_ARGS(_24) _JSON_ARGS(_25) _JSON_ARGS(_26) _JSON_ARGS(_27) _JSON_ARGS(_28) _JSON_ARGS(_29) _JSON_ARGS(_30) _JSON_ARGS(_31) _JSON_ARGS(_32) _JSON_ARGS(_33) _JSON_ARGS(_34) _JSON_ARGS(_35) _JSON_ARGS(_36) _JSON_ARGS(_37) _JSON_ARGS(_38) _JSON_ARGS(_39) _JSON_ARGS(_40) _JSON_ARGS(_41) _JSON_ARGS(_42) _JSON_ARGS(_43) _JSON_ARGS(_44) _JSON_ARGS(_45) _JSON_ARGS(_46) _JSON_ARGS(_47)

#define JSON_LIST_48(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48) \
_JSON_FORMAT(_1) ", " _JSON_FORMAT(_2) ", " _JSON_FORMAT(_3) ", " _JSON_FORMAT(_4) ", " _JSON_FORMAT(_5) ", " _JSON_FORMAT(_6) ", " _JSON_FORMAT(_7) ", " _JSON_FORMAT(_8) ", " _JSON_FORMAT(_9) ", " _JSON_FORMAT(_10) ", " _JSON_FORMAT(_11) ", " _JSON_FORMAT(_12) ", " _JSON_FORMAT(_13) ", " _JSON_FORMAT(_14) ", " _JSON_FORMAT(_15) ", " _JSON_FORMAT(_16) ", " _JSON_FORMAT(_17) ", " _JSON_FORMAT(_18) ", " _JSON_FORMAT(_19) ", " _JSON_FORMAT(_20) ", " _JSON_FORMAT(_21) ", " _JSON_FORMAT(_22) ", " _JSON_FORMAT(_23) ", " _JSON_FORMAT(_24) ", " _JSON_FORMAT(_25) ", " _JSON_FORMAT(_26) ", " _JSON_FORMAT(_27) ", " _JSON_FORMAT(_28) ", " _JSON_FORMAT(_29) ", " _JSON_FORMAT(_30) ", " _JSON_FORMAT(_31) ", " _JSON_FORMAT(_32) ", " _JSON_FORMAT(_33) ", " _JSON_FORMAT(_34) ", " _JSON_FORMAT(_35) ", " _JSON_FORMAT(_36) ", " _JSON_FORMAT(_37) ", " _JSON_FORMAT(_38) ", " _JSON_FORMAT(_39) ", " _JSON_FORMAT(_40) ", " _JSON_FORMAT(_41) ", " _JSON_FORMAT(_42) ", " _JSON_FORMAT(_43) ", " _JSON_FORMAT(_44) ", " _JSON_FORMAT(_45) ", " _JSON_FORMAT(_46) ", " _JSON_FORMAT(_47) ", " _JSON_FORMAT(_48) \
_JSON_ARGS(_1) _JSON_ARGS(_2) _JSON_ARGS(_3) _JSON_ARGS(_4) _JSON_ARGS(_5) _JSON_ARGS(_6) _JSON_ARGS(_7) _JSON_ARGS(_8) _JSON_ARGS(_9) _JSON_ARGS(_10) _JSON_ARGS(_11) _JSON_ARGS(_12) _JSON_ARGS(_13) _JSON_ARGS(_14) _JSON_ARGS(_15) _JSON_ARGS(_16) _JSON_ARGS(_17) _JSON_ARGS(_18) _JSON_ARGS(_19) _JSON_ARGS(_20) _JSON_ARGS(_21) _JSON_ARGS(_22) _JSON_ARGS(_23) _JSON_ARGS(_24) _JSON_ARGS(_25) _JSON_ARGS(_26) _JSON_ARGS(_27) _JSON_ARGS(_28) _JSON_ARGS(_29) _JSON_ARGS(_30) _JSON_ARGS(_31) _JSON_ARGS(_32) _JSON_ARGS(_33) _JSON_ARGS(_34) _JSON_ARGS(_35) _JSON_ARGS(_36) _JSON_ARGS(_37) _JSON_ARGS(_38) _JSON_ARGS(_39) _JSON_ARGS(_40) _JSON_ARGS(_41) _JSON_ARGS(_42) _JSON_ARGS(_43) _JSON_ARGS(_44) _JSON_ARGS(_45) _JSON_ARGS(_46) _JSON_ARGS(_47) _JSON_ARGS(_48)

#define JSON_LIST_49(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49) \
_JSON_FORMAT(_1) ", " _JSON_FORMAT(_2) ", " _JSON_FORMAT(_3) ", " _JSON_FORMAT(_4) ", " _JSON_FORMAT(_5) ", " _JSON_FORMAT(_6) ", " _JSON_FORMAT(_7) ", " _JSON_FORMAT(_8) ", " _JSON_FORMAT(_9) ", " _JSON_FORMAT(_10) ", " _JSON_FORMAT(_11) ", " _JSON_FORMAT(_12) ", " _JSON_FORMAT(_13) ", " _JSON_FORMAT(_14) ", " _JSON_FORMAT(_15) ", " _JSON_FORMAT(_16) ", " _JSON_FORMAT(_17) ", " _JSON_FORMAT(_18) ", " _JSON_FORMAT(_19) ", " _JSON_FORMAT(_20) ", " _JSON_FORMAT(_21) ", " _JSON_FORMAT(_22) ", " _JSON_FORMAT(_23) ", " _JSON_FORMAT(_24) ", " _JSON_FORMAT(_25) ", " _JSON_FORMAT(_26) ", " _JSON_FORMAT(_27) ", " _JSON_FORMAT(_28) ", " _JSON_FORMAT(_29) ", " _JSON_FORMAT(_30) ", " _JSON_FORMAT(_31) ", " _JSON_FORMAT(_32) ", " _JSON_FORMAT(_33) ", " _JSON_FORMAT(_34) ", " _JSON_FORMAT(_35) ", " _JSON_FORMAT(_36) ", " _JSON_FORMAT(_37) ", " _JSON_FORMAT(_38) ", " _JSON_FORMAT(_39) ", " _JSON_FORMAT(_40) ", " _JSON_FORMAT(_41) ", " _JSON_FORMAT(_42) ", " _JSON_FORMAT(_43) ", " _JSON_FORMAT(_44) ", " _JSON_FORMAT(_45) ", " _JSON_FORMAT(_46) ", " _JSON_FORMAT(_47) ", " _JSON_FORMAT(_48) ", " _JSON_FORMAT(_49) \
_JSON_ARGS(_1) _JSON_ARGS(_2) _JSON_ARGS(_3) _JSON_ARGS(_4) _JSON_ARGS(_5) _JSON_ARGS(_6) _JSON_ARGS(_7) _JSON_ARGS(_8) _JSON_ARGS(_9) _JSON_ARGS(_10) _JSON_ARGS(_11) _JSON_ARGS(_12) _JSON_ARGS(_13) _JSON_ARGS(_14) _JSON_ARGS(_15) _JSON_ARGS(_16) _JSON_ARGS(_17) _JSON_ARGS(_18) _JSON_ARGS(_19) _JSON_ARGS(_20) _JSON_ARGS(_21) _JSON_ARGS(_22) _JSON_ARGS(_23) _JSON_ARGS(_24) _JSON_ARGS(_25) _JSON_ARGS(_26) _JSON_ARGS(_27) _JSON_ARGS(_28) _JSON_ARGS(_29) _JSON_ARGS(_30) _JSON_ARGS(_31) _JSON_ARGS(_32) _JSON_ARGS(_33) _JSON_ARGS(_34) _JSON_ARGS(_35) _JSON_ARGS(_36) _JSON_ARGS(_37) _JSON_ARGS(_38) _JSON_ARGS(_39) _JSON_ARGS(_40) _JSON_ARGS(_41) _JSON_ARGS(_42) _JSON_ARGS(_43) _JSON_ARGS(_44) _JSON_ARGS(_45) _JSON_ARGS(_46) _JSON_ARGS(_47) _JSON_ARGS(_48) _JSON_ARGS(_49)

#define JSON_LIST_50(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50) \
_JSON_FORMAT(_1) ", " _JSON_FORMAT(_2) ", " _JSON_FORMAT(_3) ", " _JSON_FORMAT(_4) ", " _JSON_FORMAT(_5) ", " _JSON_FORMAT(_6) ", " _JSON_FORMAT(_7) ", " _JSON_FORMAT(_8) ", " _JSON_FORMAT(_9) ", " _JSON_FORMAT(_10) ", " _JSON_FORMAT(_11) ", " _JSON_FORMAT(_12) ", " _JSON_FORMAT(_13) ", " _JSON_FORMAT(_14) ", " _JSON_FORMAT(_15) ", " _JSON_FORMAT(_16) ", " _JSON_FORMAT(_17) ", " _JSON_FORMAT(_18) ", " _JSON_FORMAT(_19) ", " _JSON_FORMAT(_20) ", " _JSON_FORMAT(_21) ", " _JSON_FORMAT(_22) ", " _JSON_FORMAT(_23) ", " _JSON_FORMAT(_24) ", " _JSON_FORMAT(_25) ", " _JSON_FORMAT(_26) ", " _JSON_FORMAT(_27) ", " _JSON_FORMAT(_28) ", " _JSON_FORMAT(_29) ", " _JSON_FORMAT(_30) ", " _JSON_FORMAT(_31) ", " _JSON_FORMAT(_32) ", " _JSON_FORMAT(_33) ", " _JSON_FORMAT(_34) ", " _JSON_FORMAT(_35) ", " _JSON_FORMAT(_36) ", " _JSON_FORMAT(_37) ", " _JSON_FORMAT(_38) ", " _JSON_FORMAT(_39) ", " _JSON_FORMAT(_40) ", " _JSON_FORMAT(_41) ", " _JSON_FORMAT(_42) ", " _JSON_FORMAT(_43) ", " _JSON_FORMAT(_44) ", " _JSON_FORMAT(_45) ", " _JSON_FORMAT(_46) ", " _JSON_FORMAT(_47) ", " _JSON_FORMAT(_48) ", " _JSON_FORMAT(_49) ", " _JSON_FORMAT(_50) \
_JSON_ARGS(_1) _JSON_ARGS(_2) _JSON_ARGS(_3) _JSON_ARGS(_4) _JSON_ARGS(_5) _JSON_ARGS(_6) _JSON_ARGS(_7) _JSON_ARGS(_8) _JSON_ARGS(_9) _JSON_ARGS(_10) _JSON_ARGS(_11) _JSON_ARGS(_12) _JSON_ARGS(_13) _JSON_ARGS(_14) _JSON_ARGS(_15) _JSON_ARGS(_16) _JSON_ARGS(_17) _JSON_ARGS(_18) _JSON_ARGS(_19) _JSON_ARGS(_20) _JSON_ARGS(_21) _JSON_ARGS(_22) _JSON_ARGS(_23) _JSON_ARGS(_24) _JSON_ARGS(_25) _JSON_ARGS(_26) _JSON_ARGS(_27) _JSON_ARGS(_28) _JSON_ARGS(_29) _JSON_ARGS(_30) _JSON_ARGS(_31) _JSON_ARGS(_32) _JSON_ARGS(_33) _JSON_ARGS(_34) _JSON_ARGS(_35) _JSON_ARGS(_36) _JSON_ARGS(_37) _JSON_ARGS(_38) _JSON_ARGS(_39) _JSON_ARGS(_40) _JSON_ARGS(_41) _JSON_ARGS(_42) _JSON_ARGS(_43) _JSON_ARGS(_44) _JSON_ARGS(_45) _JSON_ARGS(_46) _JSON_ARGS(_47) _JSON_ARGS(_48) _JSON_ARGS(_49) _JSON_ARGS(_50)

// clang-format on

#endif /* JSON_PRINTER_H */
