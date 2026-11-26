/*!
 * @file      lr20xx_pram_load.h
 *
 * @brief     Patch RAM loading commands for Lr20xx
 *
 * The Clear BSD License
 * Copyright Semtech Corporation 2026. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted (subject to the limitations in the disclaimer
 * below) provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Semtech corporation nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY
 * THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT
 * NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SEMTECH CORPORATION BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef LR20XX_PRAM_LOAD_H
#define LR20XX_PRAM_LOAD_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * -----------------------------------------------------------------------------
 * --- DEPENDENCIES ------------------------------------------------------------
 */

#include "lr20xx_status.h"

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC MACROS -----------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC CONSTANTS --------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC TYPES ------------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC FUNCTIONS PROTOTYPES ---------------------------------------------
 */

/**
 * @brief Load and enable the LR2021-dedicated Patch RAM (PRAM) into the chip
 *
 * This function is a helper function that calls @ref lr20xx_patch_load_pram and @ref lr20xx_patch_enable_pram.
 *
 * @param context Chip implementation context
 *
 * @return lr20xx_status_t Operation status
 *
 * @see lr20xx_patch_load_pram, lr20xx_patch_enable_pram, lr20xx_pram_load_pram_lr20x2
 */
lr20xx_status_t lr20xx_pram_load_pram_lr2021( const void* context );

/**
 * @brief Load and enable the LR20X2-dedicated Patch RAM (PRAM) into the chip
 *
 * This function is a helper function that calls @ref lr20xx_patch_load_pram and @ref lr20xx_patch_enable_pram.
 *
 * @param context Chip implementation context
 *
 * @return lr20xx_status_t Operation status
 *
 * @see lr20xx_patch_load_pram, lr20xx_patch_enable_pram, lr20xx_pram_load_pram_lr2021
 */
lr20xx_status_t lr20xx_pram_load_pram_lr20x2( const void* context );

#ifdef __cplusplus
}
#endif

#endif  // LR20XX_PRAM_LOAD_H

/* --- EOF ------------------------------------------------------------------ */
