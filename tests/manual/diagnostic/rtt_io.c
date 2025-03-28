/*
 * Copyright 2024 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#include <SEGGER_RTT.h>
#include <stdio.h>

int puts(const char *s)
{
    return SEGGER_RTT_WriteString(0, s);
}

int putchar(int x)
{
    SEGGER_RTT_Write(0, (char *)&x, 1);
    return x;
}

int getchar(void)
{
    return SEGGER_RTT_GetKey();
}

int _write_r(void *reent, int fd, char *s, size_t n)
{
    SEGGER_RTT_Write(0, s, n);
    return n;
}
