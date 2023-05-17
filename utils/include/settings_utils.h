/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <stdlib.h>

typedef enum { DFU_APPLICATION, SIDEWALK_APPLICATION } app_start_t;

int load_immediate_value(const char *name, void *dest, size_t len);

app_start_t application_to_start();
