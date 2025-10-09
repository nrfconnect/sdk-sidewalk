/***************************************************************************//**
 * @file
 * @brief gpio.h
 * overrides gpio.h from silabs amazon sdk via include path precidence
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 * Your use of this software is governed by the terms of
 * Silicon Labs Master Software License Agreement (MSLA)available at
 * www.silabs.com/about-us/legal/master-software-license-agreement.
 * This software contains Third Party Software licensed by Silicon Labs from
 * Amazon.com Services LLC and its affiliates and is governed by the sections
 * of the MSLA applicable to Third Party Software and the additional terms set
 * forth in amazon_sidewalk_license.txt.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *  claim that you wrote the original software. If you use this software
 *  in a product, an acknowledgment in the product documentation would be
 *  appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *  misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/

#ifndef GPIO_H
#define GPIO_H

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <sid_pal_gpio_ifc.h>
#include "em_gpio.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
enum SL_PINout {
  SL_PIN_BUSY = 0,
  SL_PIN_ANTSW,
  SL_PIN_DIO,
  SL_PIN_NRESET,
  SL_PIN_NSS,
#ifdef LR11XX_E707
  SL_PIN_GNSS_LNA,
  SL_PIN_LED_RX,
  SL_PIN_LED_TX,
  SL_PIN_LED_SNIFFING,
#endif  //LR11XX_E707
                          
#ifdef EFR32XG21
  SL_PIN_KG100S_BAND_SEL,
#endif
  SL_PIN_MAX
};

struct GPIO_PinConfig{
  sid_pal_gpio_direction_t dir;
  sid_pal_gpio_input_t input_mode;
  sid_pal_gpio_output_t output_mode;
  sid_pal_gpio_pull_t pull_mode;
};

struct GPIO_LookupItem{
  uint32_t GPIO_Port;
  uint8_t Pin;
  struct GPIO_PinConfig PinConfig;
  GPIO_Mode_TypeDef mode;
  sid_pal_gpio_irq_handler_t callback;
  struct {
    bool falling;
    bool rising;
  }
  irq;
  void * callbackarg;
};

#ifdef __cplusplus
}
#endif

#endif /* GPIO_H */
