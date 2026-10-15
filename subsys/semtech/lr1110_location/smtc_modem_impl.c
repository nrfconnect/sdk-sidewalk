/*
 * Copyright 2025 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and conditions
 * set forth in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
// #include <sys/param.h>
#include <sid_pal_timer_ifc.h>
#include <sid_pal_delay_ifc.h>
#include <sid_clock_ifc.h>
#include <sid_time_ops.h>
#include <sid_utils.h>
#include "smtc_modem_hal.h"

#ifdef SMTC_MODEM_HAL_IRQ_FROM_SID_PAL
#include <halo_lr11xx_radio.h>
#else
#include <sid_pal_gpio_ifc.h>
#endif /* SMTC_MODEM_HAL_IRQ_FROM_SID_PAL */

#include "sl_sidewalk_log_app.h"

#if SID_PLATFORM_NORDIC
#ifdef LR11XX
#include "app_900_lr11xx_config.h"
#endif
#else
#include "app_subghz_config.h"
#endif
#include <smtc_modem_utilities.h>

#include <sid_pal_gnss_wifi.h>
#include <smtc_modem.h>

__attribute__( ( section( ".noinit" ) ) ) static uint8_t      crashlog_buff_noinit[CRASH_LOG_SIZE];
__attribute__( ( section( ".noinit" ) ) ) static volatile uint8_t crashlog_length_noinit;
__attribute__( ( section( ".noinit" ) ) ) static volatile bool  crashlog_available_noinit;

static void           (*HalDio1Callback)(void *context);
static void           *halDio1Context;

static volatile bool      halTimerIrqEnabled = true;
static volatile bool      halTimerIrqPending = false;

static void           *halTimerContext;
static sid_pal_timer_t        hal_timer;
volatile bool hal_timer_running;
static void           (*HalTimerCallback)(void *context);

uint8_t smtc_modem_hal_get_battery_level(void)
{
   uint8_t bat = 254;
   SL_SID_LOG_APP_DEBUG("Battery: %u", bat);
   return bat;
}


void smtc_modem_hal_radio_irq_clear_pending(void)
{
  /* ? TODO does this need to do anything ? */
}

void HalDio1WorkHandler()
{
  if (HalDio1Callback != NULL)
  {
    SL_SID_LOG_APP_DEBUG("DIO1 interrupt. Call handler.");
    HalDio1Callback(halDio1Context);
  } else {
    SL_SID_LOG_APP_WARNING("DIO1 interrupt. no handler.");
  }

}

#ifdef SMTC_MODEM_HAL_IRQ_FROM_SID_PAL
void smtc_modem_hal_radio_irq(void)
{
  if (HalDio1Callback != NULL) {
    HalDio1Callback(halDio1Context);
  }
  app_event_run_engine(); // trigger a subsequent call to smtc_modem_run_engine()
}
#else
static void radio_irq(uint32_t pin, void * callback_arg)
{
  if (HalDio1Callback != NULL) {
    HalDio1Callback(halDio1Context);
  }

  app_event_run_engine(); // trigger a subsequent call to smtc_modem_run_engine()
}
#endif /* !SMTC_MODEM_HAL_IRQ_FROM_SID_PAL */

void smtc_modem_hal_irq_config_radio_irq(void(*callback)(void *context), void *context)
{
#ifdef SMTC_MODEM_HAL_IRQ_FROM_SID_PAL
  HalDio1Callback = callback;
  halDio1Context = context;
#else
#ifdef LR11XX
  const radio_lr11xx_device_config_t *cfg = get_radio_cfg();
  if (sid_pal_gpio_set_irq(cfg->gpios.int1,
#else
  const radio_sx126x_device_config_t *cfg = get_radio_cfg();
  if (sid_pal_gpio_set_irq(cfg->gpio_int1,
#endif
    SID_PAL_GPIO_IRQ_TRIGGER_EDGE, radio_irq, NULL) != SID_ERROR_NONE) {
      SL_SID_LOG_APP_ERROR("Set DIO1 interrupt callback error");
  } else {
    HalDio1Callback = callback;
    halDio1Context = context;
  }
#endif /* !SMTC_MODEM_HAL_IRQ_FROM_SID_PAL */
}

uint32_t smtc_modem_hal_get_radio_tcxo_startup_delay_ms(void)
{
#ifdef LR11XX
  const radio_lr11xx_device_config_t *cfg = get_radio_cfg();
  if (cfg->tcxo_config.ctrl == LR11XX_TCXO_CTRL_NONE)
    return 0;
  else
    return 2;
#else
xxx;
   return 0;
#endif
}

void smtc_modem_hal_start_radio_tcxo(void)
{
   // put here the code that will start the tcxo if needed
}

void smtc_modem_hal_stop_radio_tcxo(void)
{
   // put here the code that will stop the tcxo if needed
}

void smtc_modem_hal_context_restore(const modem_context_type_t ctx_type, uint32_t offset, uint8_t *buffer, const uint32_t size)
{
}

void smtc_modem_hal_context_store(const modem_context_type_t ctx_type, uint32_t offset, const uint8_t *buffer, const uint32_t size)
{
}

void smtc_modem_hal_reset_mcu(void)
{
#ifdef LBM_REBOOT_ON_PANIC
    sid_pal_delay_us(2000);
    NVIC_SystemReset();   // release build
#else
    /* TODO Flush the logs before locking the CPU */

    while (true) {};
#endif
}

void smtc_modem_hal_set_ant_switch( bool is_tx_on )
{
  //TODO ? does radio control antenna switch or host mcu ?
}

void smtc_modem_hal_user_lbm_irq( void )
{
  // TODO -- Do nothing in case implementation is bare metal
}

uint32_t smtc_modem_hal_get_random_nb(void)
{
   static bool srandInit = false;
   if (!srandInit)
   {
      srand(time(NULL));   // init seed for random
      srandInit = true;
   }
   return (uint32_t) rand();
}

uint32_t smtc_modem_hal_get_random_nb_in_range(const uint32_t val_1, const uint32_t val_2)
{
  uint32_t rnd = smtc_modem_hal_get_random_nb();
  if (val_1 <= val_2)
  {
    uint32_t limit = val_2 - val_1 + 1;
    if (limit != 0)
      rnd = (rnd % limit) + val_1;
  }
  else
  {
    uint32_t limit = val_1 - val_2 + 1;
    if (limit != 0)
      rnd = (rnd % limit) + val_2;
  }
  return rnd;
}

bool smtc_modem_hal_crashlog_get_status()
{
  return crashlog_available_noinit;
}

void smtc_modem_hal_crashlog_store( const uint8_t* crash_string, uint8_t crash_string_length )
{
  crashlog_length_noinit = SID_MIN( crash_string_length, CRASH_LOG_SIZE );
  memcpy( crashlog_buff_noinit, crash_string, crashlog_length_noinit );
  crashlog_available_noinit = true;
}

void smtc_modem_hal_on_panic( uint8_t* func, uint32_t line, const char* fmt, ... )
{
  uint8_t out_buff[255] = { 0 };
  uint8_t out_len     = snprintf( ( char* ) out_buff, sizeof( out_buff ), "%s:%u ", func, line );

  va_list args;
  va_start( args, fmt );
  out_len += vsnprintf( ( char* ) &out_buff[out_len], sizeof( out_buff ) - out_len, fmt, args );
  va_end( args );

  smtc_modem_hal_crashlog_store( out_buff, out_len );

  SL_SID_LOG_APP_ERROR( "Modem panic: %s\n", out_buff );
  SID_PAL_LOG_FLUSH();
  for (;;) __asm("nop"); //smtc_modem_hal_reset_mcu( );
}


uint32_t smtc_modem_hal_get_time_in_ms(void)
{
  struct sid_timespec now = SID_TIME_ZERO;
  sid_clock_now(SID_CLOCK_SOURCE_UPTIME, &now, NULL);

  return sid_timespec_to_ms(&now);
}

uint32_t smtc_modem_hal_get_time_in_s(void)
{
  struct sid_timespec now = SID_TIME_ZERO;
  sid_clock_now(SID_CLOCK_SOURCE_UPTIME, &now, NULL);

  return sid_timespec_to_ms(&now) / SID_TIME_MSEC_PER_SEC;
}

/**
 * @brief Prints debug trace.
 *
 * @param variadics arguments.
 */
void smtc_modem_hal_print_trace(const char *fmt, ...)
{
  static uint8_t log__str_joined_len;
  static char log__str_joined[192];
  static bool subsequent = false;
  char str[128];
  uint8_t this_len;
  va_list args;

  va_start(args, fmt);
  vsnprintf(str, sizeof(str), fmt, args);
  va_end(args);

  this_len = strlen(str);

  if (!subsequent) {
      if (str[this_len-1] == '\n' || str[0] == 0x1b) {
          if (str[this_len-1] == '\n') {
              str[this_len-1] = '\0';
          }
          SID_PAL_LOG_INFO("%s", str);
      } else {
          /* first part of concatenated string */
          strncpy(log__str_joined, str, sizeof(log__str_joined));
          log__str_joined_len = this_len;
          subsequent = true;
      }
  } else {
      uint8_t remain = sizeof(log__str_joined) - log__str_joined_len;
      strncat(log__str_joined, str, remain);
      log__str_joined_len += this_len;
      if (str[this_len-1] == '\n') {
          /* termination of concatenated string */
          log__str_joined[log__str_joined_len-1] = '\0';
          SID_PAL_LOG_INFO("%s", log__str_joined);
          subsequent = false;
      }
  }
#if SID_PLATFORM_NORDIC
  SID_PAL_LOG_FLUSH();
#endif
}

void smtc_modem_hal_disable_modem_irq(void)
{
#ifndef SMTC_MODEM_HAL_IRQ_FROM_SID_PAL
  sid_error_t err;
  SL_SID_LOG_APP_INFO("Disable DIO1 IRQ.");

#ifdef LR11XX
  const radio_lr11xx_device_config_t *cfg = get_radio_cfg();
  if ((err = sid_pal_gpio_irq_disable(cfg->gpios.int1)) != SID_ERROR_NONE) {
    SL_SID_LOG_APP_ERROR("%d = sid_pal_gpio_irq_disable()", err);
  }
#else
  const radio_sx126x_device_config_t *cfg = get_radio_cfg();
  sid_pal_gpio_irq_disable(cfg->gpio_int1);
#endif

#endif /* !SMTC_MODEM_HAL_IRQ_FROM_SID_PAL */

  halTimerIrqEnabled = false;
}

void HalTimerWorkHandler()
{
   if (HalTimerCallback != NULL)
   {
    HalTimerCallback(halTimerContext);
   }
}

/**
 * @brief Enables interruptions used in Modem (radio_dio and timer).
 */
void smtc_modem_hal_enable_modem_irq(void)
{
#ifndef SMTC_MODEM_HAL_IRQ_FROM_SID_PAL
  SL_SID_LOG_APP_INFO("Enable DIO1 IRQ.");

#ifdef LR11XX
  const radio_lr11xx_device_config_t *cfg = get_radio_cfg();
  sid_pal_gpio_irq_enable(cfg->gpios.int1);
#else
  const radio_sx126x_device_config_t *cfg = get_radio_cfg();
  sid_pal_gpio_irq_enable(cfg->gpio_int1);
#endif

#endif /* !SMTC_MODEM_HAL_IRQ_FROM_SID_PAL */

  halTimerIrqEnabled = true;
  if (halTimerIrqPending) {
    HalTimerCallback(halTimerContext);
    app_event_run_engine(); // trigger a subsequent call to smtc_modem_run_engine()
    halTimerIrqPending = false;
  }
}

static void HalTimerHandler(void* arg, sid_pal_timer_t* owner)
{
  hal_timer_running = false;
  if (halTimerIrqEnabled)
  {
    if (HalTimerCallback != NULL)
    {
      // Offload the timer handling to a work queue thread.
      HalTimerCallback(halTimerContext);
      app_event_run_engine(); // trigger a subsequent call to smtc_modem_run_engine()
    }
  } else
    halTimerIrqPending = true;
}

void smtc_modem_hal_start_timer(const uint32_t milliseconds, void(*callback)(void *context), void *context)
{
  struct sid_timespec first_shot, tm2;
  unsigned ms = milliseconds;
  static bool initialized = false;
  //SL_SID_LOG_APP_DEBUG("Start timer for %lu msec.", milliseconds);

  if (milliseconds < 5) {
    ms = 5; // reasonable minimal value
  }
  if (!initialized) {
    if( sid_pal_timer_init( &hal_timer, HalTimerHandler, NULL ) != SID_ERROR_NONE ) {
      SL_SID_LOG_APP_ERROR(BRED"hal_timer init fail"COLOR_RESET);
      return;
    } else
      initialized = true;
  }
  if (hal_timer_running) {
    SL_SID_LOG_APP_ERROR(BRED"smtc_modem_hal_start_timer hal_timer_running"COLOR_RESET);
  }
  HalTimerCallback = callback;
  halTimerContext = context;

  sid_error_t err;
  tm2.tv_sec = ms / 1000;
  ms -= tm2.tv_sec * 1000;
  tm2.tv_nsec = ms * 1000000;
  sid_clock_now(SID_CLOCK_SOURCE_UPTIME, &first_shot, NULL);
  sid_time_add(&first_shot, &tm2); /* Add tm1 and tm2, set result in tm1 */
  if ((err = sid_pal_timer_arm(&hal_timer, SID_PAL_TIMER_PRIO_CLASS_PRECISE, &first_shot, NULL)) == SID_ERROR_NONE) {
    hal_timer_running = true;
  } else {
    SL_SID_LOG_APP_ERROR(BRED"sid_pal_timer_arm fail %d, ms=%u"COLOR_RESET, err, milliseconds);
  }
}

void smtc_modem_hal_stop_timer(void)
{
  if (sid_pal_timer_cancel( &hal_timer ) != SID_ERROR_NONE) {
    SL_SID_LOG_APP_ERROR(BRED"smtc_modem_hal_stop_timer fail sid_pal_timer_cancel"COLOR_RESET);
  } else
    hal_timer_running = false;

  HalTimerCallback = NULL;
  halTimerContext = NULL;
}

/**
 * @brief Return board wake up delay in ms.
 *
 * @return uint8_t Board wake up delay in ms.
 */
int8_t smtc_modem_hal_get_board_delay_ms(void)
{
   return 1;
}

bool smtc_modem_external_stack_currently_use_radio()
{
  return sid_pal_radio_hold_scan() != 0;
}

bool smtc_modem_hal_is_radio_is_free()
{
   return sid_pal_radio_hold_scan() == 0;
}

void smtc_modem_hal_radio_release()
{
    sid_pal_radio_release_scan();
}
