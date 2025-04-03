/*
 * Copyright 2021-2024 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.TXT file.  This file is a
 * Modifiable File, as defined in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef SID_ASD_CLI_H
#define SID_ASD_CLI_H

#include <sid_error.h>
#include <sid_pal_log_ifc.h>

#include <ace/cli.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * CLI flog functions based on ace CLI lib.
 */
#define CLI_LOG_INFO SID_PAL_LOG_INFO
#define CLI_LOG_WARNING SID_PAL_LOG_WARNING
#define CLI_LOG_ERROR SID_PAL_LOG_ERROR
#define CLI_LOG_HEXDUMP SID_PAL_HEXDUMP

#define CLI_LOG_FLUSH() SID_PAL_LOG_FLUSH()

/**
 * Initializes and opens the CLI.
 */
sid_error_t sid_cli_init(void);

/**
 * Closes the CLI.
 */
void sid_cli_deinit(void);

/**
 * Add additional CLI commands.
 */
sid_error_t sid_cli_add_cmd_list(const aceCli_moduleCmd_t *cmd, int32_t items);

/**
 * CLI proess API is for BLOCKING_IN_CLI_INPUT not defined.
 */
void sid_cli_process(void);

/**
 * Free cli cmd list.
 */
void sid_cli_free_cmd_list(void);

/**
 * Macro to simplify CLI structure definition based on ACE CLI lib
 */
#define SID_CLI_REGISTER_COMMAND(function_handler_) const aceCli_moduleCmd_t function_handler_[] =

#ifdef __cplusplus

#define SID_CLI_DEFINE_SUB_COMMAND_SET(name_, sub_command_set_, help_, sub_command_handler_) \
    {#name_, help_, ACE_CLI_SET_FUNC, 0, {.subCommands = sub_command_handler_}}

#define SID_CLI_DEFINE_COMMAND(name_, sub_command_set_, help_, function_handler_) \
    {#name_, help_, ACE_CLI_SET_LEAF, 0, {.func = &function_handler_}}

#define SID_CLI_REGISTER_SUB_COMMAND_SET(handler_) \
    sid_cli_add_cmd_list(handler_, (int32_t)sizeof(handler_) / sizeof(aceCli_moduleCmd_t))

#define SID_CLI_SUBCMD_SET_END {NULL, NULL, 0, 0, {NULL}}

#else

#define SID_CLI_DEFINE_SUB_COMMAND_SET(name_, sub_command_set_, help_, sub_command_handler_) \
    {#name_, help_, ACE_CLI_SET_FUNC, .command.subCommands = sub_command_handler_}

#define SID_CLI_DEFINE_COMMAND(name_, sub_command_set_, help_, function_handler_) \
    {#name_, help_, ACE_CLI_SET_LEAF, .command.func = &function_handler_}

#define SID_CLI_REGISTER_SUB_COMMAND_SET(handler_) \
    sid_cli_add_cmd_list(handler_, (int32_t)sizeof(handler_) / sizeof(aceCli_moduleCmd_t))

#define SID_CLI_SUBCMD_SET_END ACE_CLI_NULL_MODULE

#endif

#ifdef __cplusplus
}
#endif

#endif
