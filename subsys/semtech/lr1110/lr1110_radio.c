/*
 * Copyright (c) 2019-2025 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * This file supports radio HAL interface
 */

#define HALO_NEEDS_SAFE_ASSERT 1

#include <assert.h>

#include <sid_pal_log_ifc.h>

#include "halo_lr1110_radio.h"

#include "lr1110_radio.h"
#include "lr1110_system.h"
#include "lr1110_regmem.h"

#include <sid_time_ops.h>
#include <sid_clock_ifc.h>
#include <sid_pal_delay_ifc.h>

#define LR1110_DEFAULT_LORA_IRQ_MASK       (LR1110_SYSTEM_IRQ_ALL_MASK & ~(LR1110_SYSTEM_IRQ_PREAMBLE_DETECTED | \
                                            LR1110_SYSTEM_IRQ_SYNC_WORD_HEADER_VALID))

#define LR1110_DEFAULT_FSK_IRQ_MASK        (LR1110_SYSTEM_IRQ_ALL_MASK & ~LR1110_SYSTEM_IRQ_PREAMBLE_DETECTED)

#define LR1110_RX_CONTINUOUS_VAL           0xFFFFFF
#define INFINITE_TIME                      0xFFFFFFFF
#define LR1110_MIN_CHANNEL_FREE_DELAY_US   1
#define LR1110_NOISE_SAMPLE_SIZE           32
#define LR1110_RTC_FREQ_IN_HZ              32768UL
#define COMMON_DIVIDER                     64UL

/* Delay time to allow for any external PA/FEM turn ON/OFF */
#define SEMTECH_STDBY_STATE_DELAY_US       10

#ifndef RADIO_LR1110_TXPWR_WORKAROUND
#define RADIO_LR1110_TXPWR_WORKAROUND 0
#endif

#if RADIO_LR1110_TXPWR_WORKAROUND

#define LR1110_BAND_EDGE_BAND_START_FREQ   902000000
#define LR1110_BAND_EDGE_LIMIT_FREQ        903000000
#define LR1110_REG_ADDR_BAND_EDGE_FIX      0x00F20420
#define LR1110_REG_MASK_BAND_EDGE_FIX      0x00070000 // bits 16-18
#ifndef LR1110_REG_VALUE_BAND_EDGE_FIX_LO  // flag to change other register value.
#define LR1110_REG_VALUE_BAND_EDGE_FIX_LO  0x00050000 // 0b101 at mask
#endif
#define LR1110_REG_VALUE_BAND_EDGE_FIX_HI  0x00040000 // 0b100 at mask

#endif // RADIO_LR1110_TXPWR_WORKAROUND

#define LR1110_CAD_DEFAULT_TX_TIMEOUT       0 // disable Tx timeout for CAD

static halo_drv_semtech_ctx_t              drv_ctx = {0};

static inline uint32_t us_to_rtc_ticks(uint32_t us)
{
    /* Zero and  0xFFFFFF are two magic values in Semtech FW */
    switch(us) {
    case LR1110_RX_CONTINUOUS_VAL:
        return LR1110_RX_CONTINUOUS_VAL;
    case 0:
        return 0;
    default:
        return (((us)*(LR1110_RTC_FREQ_IN_HZ/COMMON_DIVIDER))/(US_IN_SEC/COMMON_DIVIDER));
    }
}

static int32_t radio_lr1110_platform_init(void)
{
    int32_t err = RADIO_ERROR_INVALID_PARAMS;

    if (drv_ctx.config->gpios.radio_busy != HALO_GPIO_NOT_CONNECTED) {
        if (sid_pal_gpio_set_direction(drv_ctx.config->gpios.radio_busy,
            SID_PAL_GPIO_DIRECTION_INPUT) != SID_ERROR_NONE) {
            goto ret;
        }
    }

    if (drv_ctx.config->gpios.tx_bypass != HALO_GPIO_NOT_CONNECTED) {
        if (sid_pal_gpio_set_direction(drv_ctx.config->gpios.tx_bypass,
            SID_PAL_GPIO_DIRECTION_OUTPUT) != SID_ERROR_NONE) {
            goto ret;
        }
    }

    if (drv_ctx.config->gpios.rf_sw_ena != HALO_GPIO_NOT_CONNECTED) {
        if (sid_pal_gpio_set_direction(drv_ctx.config->gpios.rf_sw_ena,
            SID_PAL_GPIO_DIRECTION_OUTPUT) != SID_ERROR_NONE) {
            goto ret;
        }
    }

    if (drv_ctx.config->gpios.txrx != HALO_GPIO_NOT_CONNECTED) {
        if (sid_pal_gpio_set_direction(drv_ctx.config->gpios.txrx,
            SID_PAL_GPIO_DIRECTION_OUTPUT) != SID_ERROR_NONE) {
            goto ret;
        }
    }

    if (drv_ctx.config->gpios.power != HALO_GPIO_NOT_CONNECTED) {
        if (sid_pal_gpio_set_direction(drv_ctx.config->gpios.power,
            SID_PAL_GPIO_DIRECTION_OUTPUT) != SID_ERROR_NONE) {
            goto ret;
        }
    }

    if (drv_ctx.config->pa_cfg_callback == NULL) {
        goto ret;
    }

    if (drv_ctx.config->bus_factory->create(&drv_ctx.bus_iface, drv_ctx.config->bus_factory->config) != SID_ERROR_NONE) {
        err = RADIO_ERROR_IO_ERROR;
        goto ret;
    }

    err = RADIO_ERROR_NONE;
ret:
    return err;
}

static void radio_irq(uint32_t pin, void * callback_arg)
{
    (void)callback_arg;

    uint8_t pinState;
    if (sid_pal_gpio_read(pin, &pinState) == SID_ERROR_NONE) {
        if (pinState) {
            sid_clock_now(SID_CLOCK_SOURCE_UPTIME, &drv_ctx.radio_rx_packet->rcv_tm, NULL);
            drv_ctx.irq_handler();
        }
    }
}

int32_t set_radio_int(const halo_drv_semtech_ctx_t *drv_ctx, bool int_enable)
{
    if (int_enable == true) {
        sid_pal_gpio_irq_enable(drv_ctx->config->gpios.int1);
    } else {
        sid_pal_gpio_irq_disable(drv_ctx->config->gpios.int1);
    }
    return RADIO_ERROR_NONE;
}

static int32_t radio_set_irq_mask(lr1110_system_irq_mask_t irq_mask)
{
    if (lr1110_system_set_dio_irq_params(&drv_ctx, irq_mask, LR1110_SYSTEM_IRQ_NONE)
            != LR1110_STATUS_OK) {
        return RADIO_ERROR_IO_ERROR;
    }
    return RADIO_ERROR_NONE;
}

static int32_t radio_clear_irq_status_all(void)
{
    if (lr1110_system_clear_irq_status(&drv_ctx, LR1110_SYSTEM_IRQ_ALL_MASK)
            != LR1110_STATUS_OK) {
        return RADIO_ERROR_HARDWARE_ERROR;
    }
    return RADIO_ERROR_NONE;
}

int32_t radio_enable_irq(const halo_drv_semtech_ctx_t* drv)
{
    const lr1110_status_t status =  lr1110_system_set_dio_irq_params(drv,
                                                                    drv_ctx.irq_mask,
                                                                    LR1110_SYSTEM_IRQ_NONE);

    return (status == LR1110_STATUS_OK)? RADIO_ERROR_NONE : RADIO_ERROR_IO_ERROR;
}

int32_t radio_disable_irq(const halo_drv_semtech_ctx_t* drv)
{
    const lr1110_status_t status =  lr1110_system_set_dio_irq_params(drv,
                                                                    LR1110_SYSTEM_IRQ_NONE,
                                                                    LR1110_SYSTEM_IRQ_NONE);

    return (status == LR1110_STATUS_OK)? RADIO_ERROR_NONE : RADIO_ERROR_IO_ERROR;
}

static int32_t sid_pal_radio_set_modem_to_lora_mode(void)
{
    if (lr1110_radio_set_pkt_type(&drv_ctx, LR1110_RADIO_PKT_TYPE_LORA) != LR1110_STATUS_OK) {
        return RADIO_ERROR_HARDWARE_ERROR;
    }
    drv_ctx.modem = SID_PAL_RADIO_MODEM_MODE_LORA;
    drv_ctx.irq_mask = LR1110_DEFAULT_LORA_IRQ_MASK;

    return radio_enable_irq(&drv_ctx);
}

static int32_t sid_pal_radio_set_modem_to_fsk_mode(void)
{
    if (lr1110_radio_set_pkt_type(&drv_ctx, LR1110_RADIO_PKT_TYPE_GFSK) != LR1110_STATUS_OK) {
        return RADIO_ERROR_HARDWARE_ERROR;
    }
    drv_ctx.modem = SID_PAL_RADIO_MODEM_MODE_FSK;
    drv_ctx.irq_mask = LR1110_DEFAULT_FSK_IRQ_MASK;

    return radio_enable_irq(&drv_ctx);
}

halo_drv_semtech_ctx_t* lr1110_get_drv_ctx(void)
{
    return &drv_ctx;
}

void set_lora_exit_mode(sid_pal_radio_cad_param_exit_mode_t cad_exit_mode)
{
    drv_ctx.cad_exit_mode = cad_exit_mode;
}

int32_t radio_lr1110_set_radio_mode(bool rf_en, bool tx_en, bool ext_pa)
{
    int32_t err = RADIO_ERROR_NONE;

    do {
        if (rf_en == false && (tx_en == true || ext_pa == true)) {
            /* Invalid pin config */
            err = RADIO_ERROR_INVALID_PARAMS;
            break;
        }

        if (drv_ctx.config->gpios.rf_sw_ena != HALO_GPIO_NOT_CONNECTED) {
            if (sid_pal_gpio_write(drv_ctx.config->gpios.rf_sw_ena, rf_en) != SID_ERROR_NONE) {
                err = RADIO_ERROR_IO_ERROR;
                break;
            }
        }

        if (drv_ctx.config->gpios.tx_bypass != HALO_GPIO_NOT_CONNECTED) {
            if (sid_pal_gpio_write(drv_ctx.config->gpios.tx_bypass, ext_pa) != SID_ERROR_NONE) {
                err = RADIO_ERROR_IO_ERROR;
                break;
            }
        }

        if (drv_ctx.config->gpios.txrx != HALO_GPIO_NOT_CONNECTED) {
            if (sid_pal_gpio_write(drv_ctx.config->gpios.txrx, tx_en) != SID_ERROR_NONE) {
                err = RADIO_ERROR_IO_ERROR;
                break;
            }
        }
    } while (0);

    return err;
}

void set_radio_lr1110_device_config(const radio_lr1110_device_config_t *cfg)
{
    drv_ctx.config = cfg;
}

uint8_t sid_pal_radio_get_status(void)
{
    return drv_ctx.radio_state;
}

sid_pal_radio_modem_mode_t sid_pal_radio_get_modem_mode(void)
{
    return drv_ctx.modem;
}

int32_t sid_pal_radio_set_modem_mode(sid_pal_radio_modem_mode_t mode)
{
    switch(mode) {
        case SID_PAL_RADIO_MODEM_MODE_LORA:
            return sid_pal_radio_set_modem_to_lora_mode();
        case SID_PAL_RADIO_MODEM_MODE_FSK:
            return sid_pal_radio_set_modem_to_fsk_mode();
        default:
            return RADIO_ERROR_NOT_SUPPORTED;
    }
}

int32_t sid_pal_radio_irq_process(void)
{
    sid_pal_radio_events_t radio_event = SID_PAL_RADIO_EVENT_UNKNOWN;
    lr1110_system_irq_mask_t irq_status;
    int32_t err = RADIO_ERROR_NONE;

    do {
        if ((err = radio_disable_irq(&drv_ctx)) != RADIO_ERROR_NONE) {
            break;
        }

        if (lr1110_system_get_and_clear_irq_status(&drv_ctx, &irq_status) != LR1110_STATUS_OK) {
            err =  RADIO_ERROR_HARDWARE_ERROR;
            break;
        }

        if (irq_status & LR1110_SYSTEM_IRQ_ERROR) {
            lr1110_system_errors_t errors = 0;
            lr1110_system_get_errors(&drv_ctx, &errors);
            lr1110_system_clear_errors(&drv_ctx);
            SID_HAL_LOG_WARNING("LR1110: Errors 0x%.4X", errors);
        }

        if (irq_status & LR1110_SYSTEM_IRQ_TX_DONE) {
            radio_event = SID_PAL_RADIO_EVENT_TX_DONE;
            break;
        }

        if (irq_status & LR1110_SYSTEM_IRQ_CRC_ERROR) {
            radio_event = SID_PAL_RADIO_EVENT_RX_ERROR;
            break;
        }

        if (irq_status & LR1110_SYSTEM_IRQ_TIMEOUT) {
            radio_event = (drv_ctx.radio_state == SID_PAL_RADIO_TX) ?
                SID_PAL_RADIO_EVENT_TX_TIMEOUT :
                SID_PAL_RADIO_EVENT_RX_TIMEOUT;
            if (drv_ctx.cad_exit_mode == SID_PAL_RADIO_CAD_EXIT_MODE_CS_LBT
                    && radio_event == SID_PAL_RADIO_EVENT_RX_TIMEOUT) {
                drv_ctx.cad_exit_mode = SID_PAL_RADIO_CAD_EXIT_MODE_NONE;
#if HALO_ENABLE_DIAGNOSTICS
                radio_event = SID_PAL_RADIO_EVENT_CS_TIMEOUT;
#else
                radio_event = SID_PAL_RADIO_EVENT_UNKNOWN;
                sid_pal_radio_start_tx(LR1110_CAD_DEFAULT_TX_TIMEOUT);
#endif
            }
            break;
        }

        if (irq_status & LR1110_SYSTEM_IRQ_HEADER_ERROR) {
            radio_event = SID_PAL_RADIO_EVENT_RX_TIMEOUT;
            break;
        }

        if (drv_ctx.modem == SID_PAL_RADIO_MODEM_MODE_LORA) {
            if (irq_status & LR1110_SYSTEM_IRQ_CAD_DONE) {
                radio_event = (irq_status & LR1110_SYSTEM_IRQ_CAD_DETECTED) ?
                               SID_PAL_RADIO_EVENT_CAD_DONE :
                               SID_PAL_RADIO_EVENT_CAD_TIMEOUT;
                if (drv_ctx.cad_exit_mode == SID_PAL_RADIO_CAD_EXIT_MODE_CS_LBT &&
                    radio_event == SID_PAL_RADIO_EVENT_CAD_TIMEOUT) {
                    radio_event = SID_PAL_RADIO_EVENT_UNKNOWN;
                    sid_pal_radio_start_tx(SID_PAL_RADIO_LORA_CAD_DEFAULT_TX_TIMEOUT);
                }
                drv_ctx.cad_exit_mode = SID_PAL_RADIO_CAD_EXIT_MODE_NONE;
                break;
            }

            if ((irq_status & LR1110_SYSTEM_IRQ_RX_DONE) && !(irq_status & LR1110_SYSTEM_IRQ_CRC_ERROR) && DUAL_LINK_SUPPORT) {
                if ((err = radio_lora_process_rx_done(&drv_ctx)) == RADIO_ERROR_NONE) {
                    memset(&drv_ctx.radio_rx_packet->fsk_rx_packet_status, 0, sizeof(sid_pal_radio_fsk_rx_packet_status_t));
                    radio_event = SID_PAL_RADIO_EVENT_RX_DONE;
                }
                break;
            }
        } else if (drv_ctx.modem == SID_PAL_RADIO_MODEM_MODE_FSK) {
            if (irq_status & LR1110_SYSTEM_IRQ_SYNC_WORD_HEADER_VALID) {
                radio_fsk_process_sync_word_detected(&drv_ctx);
                break;
            }

            if (irq_status & LR1110_SYSTEM_IRQ_PREAMBLE_DETECTED) {
                if (drv_ctx.cad_exit_mode == SID_PAL_RADIO_CAD_EXIT_MODE_CS_LBT) {
                    radio_event = SID_PAL_RADIO_EVENT_CS_DONE;
                    drv_ctx.cad_exit_mode = SID_PAL_RADIO_CAD_EXIT_MODE_NONE;
                    sid_pal_radio_standby();
                }
                break;
            }

            // CRC check not necessary for fsk
            if (irq_status & LR1110_SYSTEM_IRQ_RX_DONE) {
                radio_fsk_rx_done_status_t fsk_rx_done_status = RADIO_FSK_RX_DONE_STATUS_OK;
                sid_clock_now(SID_CLOCK_SOURCE_UPTIME, &drv_ctx.radio_rx_packet->rcv_tm, NULL);
                if ((err = radio_fsk_process_rx_done(&drv_ctx, &fsk_rx_done_status)) == RADIO_ERROR_NONE) {
                    memset(&drv_ctx.radio_rx_packet->lora_rx_packet_status, 0, sizeof(sid_pal_radio_lora_rx_packet_status_t));
                    radio_event = SID_PAL_RADIO_EVENT_RX_DONE;
                } else
                if (err == RADIO_ERROR_GENERIC) {
                    // IF the same has to be done for all error rx done status.
                    // check only err == RADIO_ERROR_GENERIC
                    switch (fsk_rx_done_status) {
                        case RADIO_FSK_RX_DONE_STATUS_SW_MARK_NOT_PRESENT:
                            // TODO WHAT here?
                            radio_event = SID_PAL_RADIO_EVENT_RX_ERROR;
                            break;
                        case RADIO_FSK_RX_DONE_STATUS_UNKNOWN_ERROR:
                            // TODO WHAT here?
                            radio_event = SID_PAL_RADIO_EVENT_RX_ERROR;
                            break;
                        case RADIO_FSK_RX_DONE_STATUS_BAD_CRC:
                            radio_event = SID_PAL_RADIO_EVENT_RX_ERROR;
                            break;
                        default:
                            assert(0);
                    }
                }
            }
        }
    } while(0);

    if (SID_PAL_RADIO_EVENT_UNKNOWN != radio_event) {
        drv_ctx.report_radio_event(radio_event);
    }

    if (drv_ctx.radio_state != SID_PAL_RADIO_SLEEP) {
        err = radio_enable_irq(&drv_ctx);
    }

    return err;
}

#define FREQ_STEP 4000000

typedef struct {
    uint32_t start;
    uint32_t stop;
} freq_band_t;

static const freq_band_t bands[] = {
    {430000000, 440000000},
    {470000000, 510000000},
    {779000000, 787000000},
    {863000000, 879000000},
    {902000000, 928000000},
    {2400000000, 2480000000},
};

static const freq_band_t* lr1110_get_freq_band(uint32_t freq)
{
    for (size_t i = 0; i < sizeof(bands)/sizeof(bands[0]); i++) {
        if (freq >= bands[i].start && freq <= bands[i].stop) {
            return &bands[i];
        }
    }
    return NULL;
}

int32_t sid_pal_radio_set_frequency(uint32_t freq)
{
    int32_t err = RADIO_ERROR_NONE;

    do {
        if (drv_ctx.radio_state != SID_PAL_RADIO_STANDBY) {
            err =  RADIO_ERROR_INVALID_STATE;
            break;
        }

        const freq_band_t* const cur_freq_band = lr1110_get_freq_band(drv_ctx.radio_freq_hz);
        const freq_band_t* const freq_band     = lr1110_get_freq_band(freq);

        if (freq_band == NULL) {
            err = RADIO_ERROR_INVALID_PARAMS;
            break;
        }

        if (cur_freq_band != freq_band) {
            if(lr1110_system_calibrate_image(&drv_ctx, freq_band->start / FREQ_STEP, freq_band->stop / FREQ_STEP) != LR1110_STATUS_OK) {
                err = RADIO_ERROR_HARDWARE_ERROR;
                break;
            }
        }

        #if RADIO_LR1110_TXPWR_WORKAROUND
        if (freq_band->start == LR1110_BAND_EDGE_BAND_START_FREQ &&
            drv_ctx.modem == SID_PAL_RADIO_MODEM_MODE_LORA) {
            uint32_t reg_val;
            if (freq <= LR1110_BAND_EDGE_LIMIT_FREQ) {
                reg_val = LR1110_REG_VALUE_BAND_EDGE_FIX_LO;
            } else {
                reg_val = LR1110_REG_VALUE_BAND_EDGE_FIX_HI;
            }

            if (lr1110_regmem_write_regmem32_mask(&drv_ctx, LR1110_REG_ADDR_BAND_EDGE_FIX,
                                     LR1110_REG_MASK_BAND_EDGE_FIX, reg_val) != LR1110_STATUS_OK) {
                SID_HAL_LOG_WARNING("LR1110: Band edge fix was not applied.");
                err = RADIO_ERROR_HARDWARE_ERROR;
                break;
            }
        }
        #endif // RADIO_LR1110_TXPWR_WORKAROUND

        if (lr1110_radio_set_rf_freq(&drv_ctx, freq) != LR1110_STATUS_OK) {
            err = RADIO_ERROR_HARDWARE_ERROR;
            break;
        }
        drv_ctx.radio_freq_hz = freq;

    } while(0);

    return err;
}

int32_t sid_pal_radio_get_max_tx_power(sid_pal_radio_data_rate_t data_rate, int8_t *tx_power)
{
    if (data_rate <= SID_PAL_RADIO_DATA_RATE_INVALID || data_rate > SID_PAL_RADIO_DATA_RATE_MAX_NUM) {
        return RADIO_ERROR_INVALID_PARAMS;
    }

    *tx_power = drv_ctx.regional_radio_param.max_tx_power[data_rate - 1];
    return RADIO_ERROR_NONE;
}

int32_t sid_pal_radio_set_region(sid_pal_radio_region_code_t region)
{
    int32_t err = RADIO_ERROR_NOT_SUPPORTED;

    if (region <= SID_PAL_RADIO_RC_NONE || region >= SID_PAL_RADIO_RC_MAX) {
        return RADIO_ERROR_INVALID_PARAMS;
    }

    for (uint8_t i = 0; i < drv_ctx.config->regional_config.reg_param_table_size; i++) {
        if (region == drv_ctx.config->regional_config.reg_param_table[i].param_region) {
            drv_ctx.regional_radio_param = drv_ctx.config->regional_config.reg_param_table[i];
            err = RADIO_ERROR_NONE;
            break;
        }
    }

    return err;
}

#if HALO_ENABLE_DIAGNOSTICS
int32_t semtech_radio_set_sx126x_pa_config(semtech_radio_pa_cfg_t *cfg)
{
    if (!cfg) {
        return RADIO_ERROR_INVALID_PARAMS;
    }

    radio_lr1110_pa_cfg_t cur_cfg;

    cur_cfg.pa_cfg.pa_duty_cycle = cfg->pa_duty_cycle;
    cur_cfg.pa_cfg.pa_hp_sel = cfg->hp_max;
    cur_cfg.pa_cfg.pa_sel = (lr1110_radio_pa_selection_t)cfg->device_sel;
    cur_cfg.pa_cfg.pa_reg_supply = (lr1110_radio_pa_reg_supply_t)cfg->pa_lut;
    cur_cfg.tx_power_in_dbm = cfg->tx_power;
    cur_cfg.ramp_time = (lr1110_radio_ramp_time_t)cfg->ramp_time;
    cur_cfg.enable_ext_pa = cfg->enable_ext_pa;

    drv_ctx.pa_cfg = cur_cfg;
    drv_ctx.pa_cfg_configured = true;
    return RADIO_ERROR_NONE;
}
#endif

int32_t get_radio_lr1110_pa_config(radio_lr1110_pa_cfg_t *cfg)
{
    if (!cfg) {
        return RADIO_ERROR_INVALID_PARAMS;
    }

    *cfg = drv_ctx.pa_cfg;
    return RADIO_ERROR_NONE;
}

int32_t sid_pal_radio_set_tx_power(int8_t power)
{
    int32_t err = RADIO_ERROR_NONE;


    if (drv_ctx.radio_state != SID_PAL_RADIO_STANDBY) {
        return RADIO_ERROR_INVALID_STATE;
    }

#if HALO_ENABLE_DIAGNOSTICS
    if (drv_ctx.pa_cfg_configured == false)
#endif
    {
        if (drv_ctx.config->pa_cfg_callback(power, &drv_ctx.pa_cfg) < 0) {
            return RADIO_ERROR_INVALID_PARAMS;
        }
    }

    // TODO: Add over current config when the API is added to the Driver

    if (err == RADIO_ERROR_NONE) {
        err = RADIO_ERROR_IO_ERROR;
        do {
            if (lr1110_radio_set_pa_cfg(&drv_ctx, &drv_ctx.pa_cfg.pa_cfg) != LR1110_STATUS_OK) {
                break;
            }

            if (lr1110_radio_set_tx_params(&drv_ctx, drv_ctx.pa_cfg.tx_power_in_dbm,
                  drv_ctx.pa_cfg.ramp_time) != LR1110_STATUS_OK) {
                break;
            }
            err = RADIO_ERROR_NONE;
        } while (0);
    }
    return err;
}

static sid_pal_radio_sleep_start_notify_handler_t sleep_start_notify_cb;
int32_t sid_hal_set_sleep_start_notify_cb(sid_pal_radio_sleep_start_notify_handler_t callback)
{
    sleep_start_notify_cb = callback;
    return RADIO_ERROR_NONE;
}

int32_t sid_pal_radio_sleep(uint32_t sleep_us)
{
    int32_t err;

    if (sleep_start_notify_cb && sleep_us) {
        struct sid_timespec delta_time, wakeup_time;
        sid_clock_now(SID_CLOCK_SOURCE_UPTIME, &wakeup_time, NULL);
        sid_us_to_timespec(sleep_us, &delta_time);
        sid_time_add(&wakeup_time, &delta_time);
        sleep_start_notify_cb(&wakeup_time);
    }

    do {
        if (drv_ctx.radio_state == SID_PAL_RADIO_SLEEP) {
            err = RADIO_ERROR_NONE;
            break;
        }

        if ((err = radio_clear_irq_status_all()) != RADIO_ERROR_NONE) {
            break;
        }

        if ((err = radio_lr1110_set_radio_mode(false, false, false)) != RADIO_ERROR_NONE) {
            break;
        }

        lr1110_system_sleep_cfg_t cfg = {
            .is_warm_start   = 1,
            .is_rtc_timeout  = 1,
        };

        if (drv_ctx.config->mitigations.irq_noise_during_sleep) {
            set_radio_int(&drv_ctx, false);
        }

        if (lr1110_system_set_sleep(&drv_ctx, cfg, INFINITE_TIME) != LR1110_STATUS_OK) {
            err = RADIO_ERROR_HARDWARE_ERROR;
            if (drv_ctx.config->mitigations.irq_noise_during_sleep) {
                set_radio_int(&drv_ctx, true);
            }
            break;
        }

        drv_ctx.radio_state = SID_PAL_RADIO_SLEEP;
    } while(0);

    return err;
}

int32_t sid_pal_radio_standby()
{
    int32_t err = RADIO_ERROR_NONE;

    do {
        if (drv_ctx.radio_state == SID_PAL_RADIO_STANDBY) {
            break;
        }

        if (drv_ctx.radio_state == SID_PAL_RADIO_SLEEP) {
            if (lr1110_system_wakeup(&drv_ctx) != LR1110_STATUS_OK) {
                err = RADIO_ERROR_HARDWARE_ERROR;
                break;
            }
            if (drv_ctx.config->mitigations.lbd_clear_on_wakeup) {
                lr1110_system_irq_mask_t irq_status = 0;
                lr1110_system_get_and_clear_irq_status(&drv_ctx, &irq_status);
            }
            if (drv_ctx.config->mitigations.irq_noise_during_sleep) {
                set_radio_int(&drv_ctx, true);
            }
            drv_ctx.radio_state = SID_PAL_RADIO_UNKNOWN;
        }

        if ((err = radio_clear_irq_status_all()) != RADIO_ERROR_NONE) {
            break;
        }

        if ((err = radio_lr1110_set_radio_mode(false, false, false)) != RADIO_ERROR_NONE) {
            break;
        }

        sid_pal_delay_us(SEMTECH_STDBY_STATE_DELAY_US);
        lr1110_system_standby_cfg_t standby_cfg = LR1110_SYSTEM_STANDBY_CFG_RC;

        if (drv_ctx.config->tcxo_config.ctrl != LR1110_TCXO_CTRL_NONE) {
            standby_cfg = LR1110_SYSTEM_STANDBY_CFG_XOSC;
        }

        if (lr1110_system_set_standby(&drv_ctx, standby_cfg) != LR1110_STATUS_OK) {
            err = RADIO_ERROR_HARDWARE_ERROR;
            break;
        }

        drv_ctx.radio_state = SID_PAL_RADIO_STANDBY;
    } while(0);

    return err;
}

int32_t sid_pal_set_radio_busy(void)
{
    if (drv_ctx.radio_state == SID_PAL_RADIO_STANDBY) {
        drv_ctx.radio_state = SID_PAL_RADIO_BUSY;
    } else {
        return RADIO_ERROR_INVALID_STATE;
    }
    return RADIO_ERROR_NONE;
}

int32_t sid_pal_radio_set_tx_payload(const uint8_t *buffer, uint8_t size)
{
    if (buffer == NULL || size == 0) {
        return RADIO_ERROR_INVALID_PARAMS;
    }

    if (lr1110_regmem_write_buffer8(&drv_ctx, buffer, size) != LR1110_STATUS_OK) {
        return RADIO_ERROR_IO_ERROR;
    }
    return RADIO_ERROR_NONE;
}

int32_t sid_pal_radio_start_tx(uint32_t timeout)
{
    int32_t err;

    do {
        if ((err = radio_lr1110_set_radio_mode(true, true, drv_ctx.pa_cfg.enable_ext_pa)) != RADIO_ERROR_NONE) {
            break;
        }

        if ((err = radio_clear_irq_status_all()) != RADIO_ERROR_NONE) {
            break;
        }

        if (lr1110_radio_set_tx_with_timeout_in_rtc_step(&drv_ctx, us_to_rtc_ticks(timeout)) != LR1110_STATUS_OK) {
            err = RADIO_ERROR_HARDWARE_ERROR;
            break;
        }

        drv_ctx.radio_state = SID_PAL_RADIO_TX;
    } while(0);

    return err;
}

int32_t sid_pal_radio_set_tx_continuous_wave(uint32_t freq, int8_t power)
{
    int32_t err;

    do {
        if ((err = sid_pal_radio_set_frequency(freq) != RADIO_ERROR_NONE)) {
            break;
        }

        if ((err = sid_pal_radio_set_tx_power(power) != RADIO_ERROR_NONE)) {
            break;
        }

        if (lr1110_system_set_standby(&drv_ctx, LR1110_SYSTEM_STANDBY_CFG_XOSC) != LR1110_STATUS_OK) {
            err = RADIO_ERROR_HARDWARE_ERROR;
            break;
        }

        if ((err = radio_lr1110_set_radio_mode(true, true, drv_ctx.pa_cfg.enable_ext_pa)) != RADIO_ERROR_NONE) {
            break;
        }

        if ((err = radio_clear_irq_status_all()) != RADIO_ERROR_NONE) {
            break;
        }

        if (lr1110_radio_set_tx_cw(&drv_ctx) != LR1110_STATUS_OK) {
            err = RADIO_ERROR_HARDWARE_ERROR;
            break;
        }
        drv_ctx.radio_state = SID_PAL_RADIO_TX;
    } while(0);

    return err;
}

int32_t sid_pal_radio_set_tx_continuous_preamble(uint32_t freq, int8_t power)
{
    int32_t err;

    do {
        if ((err = sid_pal_radio_set_frequency(freq) != RADIO_ERROR_NONE)) {
            break;
        }

        if ((err = sid_pal_radio_set_tx_power(power) != RADIO_ERROR_NONE)) {
            break;
        }

        if (lr1110_system_set_standby(&drv_ctx, LR1110_SYSTEM_STANDBY_CFG_XOSC) != LR1110_STATUS_OK) {
            err = RADIO_ERROR_HARDWARE_ERROR;
            break;
        }

        if ((err = radio_lr1110_set_radio_mode(true, true, drv_ctx.pa_cfg.enable_ext_pa)) != RADIO_ERROR_NONE) {
            break;
        }

        if ((err = radio_clear_irq_status_all()) != RADIO_ERROR_NONE) {
            break;
        }

        if (lr1110_radio_set_tx_infinite_preamble(&drv_ctx) != LR1110_STATUS_OK) {
            err = RADIO_ERROR_HARDWARE_ERROR;
            break;
        }
        drv_ctx.radio_state = SID_PAL_RADIO_TX;
    } while(0);

    return err;
}

int32_t sid_pal_radio_start_rx(uint32_t timeout)
{
    int32_t err;

    do {

        bool pbl_det_timer = false;
        if (drv_ctx.modem == SID_PAL_RADIO_MODEM_MODE_FSK) {
            pbl_det_timer = true;
        }

        if (lr1110_radio_stop_timeout_on_preamble(&drv_ctx, pbl_det_timer) != LR1110_STATUS_OK) {
            err = RADIO_ERROR_HARDWARE_ERROR;
            break;
        }

        if ((err = radio_lr1110_set_radio_mode(true, false, false)) != RADIO_ERROR_NONE) {
            break;
        }

        if ((err = radio_clear_irq_status_all()) != RADIO_ERROR_NONE) {
            break;
        }

        if (lr1110_radio_set_rx_with_timeout_in_rtc_step(&drv_ctx, us_to_rtc_ticks(timeout)) != LR1110_STATUS_OK) {
            err = RADIO_ERROR_HARDWARE_ERROR;
            break;
        }

        drv_ctx.radio_state = SID_PAL_RADIO_RX;
    } while(0);

    return err;
}

int32_t sid_pal_radio_is_cad_exit_mode(sid_pal_radio_cad_param_exit_mode_t exit_mode)
{
    int32_t err = RADIO_ERROR_NONE;

    if (!((exit_mode == SID_PAL_RADIO_CAD_EXIT_MODE_CS_ONLY) ||
        (exit_mode == SID_PAL_RADIO_CAD_EXIT_MODE_CS_RX) ||
        (exit_mode == SID_PAL_RADIO_CAD_EXIT_MODE_CS_LBT) ||
        (exit_mode == SID_PAL_RADIO_CAD_EXIT_MODE_ED_ONLY) ||
        (exit_mode == SID_PAL_RADIO_CAD_EXIT_MODE_ED_RX) ||
        (exit_mode == SID_PAL_RADIO_CAD_EXIT_MODE_ED_LBT))) {
        err = RADIO_ERROR_INVALID_PARAMS;
    }

    return err;
}

int32_t sid_pal_radio_start_carrier_sense(const sid_pal_radio_fsk_cad_params_t *cad_params,
                                          sid_pal_radio_cad_param_exit_mode_t exit_mode)
{
    int32_t err;

    do {

        if (drv_ctx.modem != SID_PAL_RADIO_MODEM_MODE_FSK) {
            err = RADIO_ERROR_INVALID_PARAMS;
            break;
        }

        if ((err = sid_pal_radio_is_cad_exit_mode(exit_mode)) != RADIO_ERROR_NONE) {
            break;
        }

        if (lr1110_radio_stop_timeout_on_preamble(&drv_ctx, true) != LR1110_STATUS_OK) {
            err = RADIO_ERROR_HARDWARE_ERROR;
            break;
        }

        if ((err = radio_lr1110_set_radio_mode(true, false, false)) != RADIO_ERROR_NONE) {
            break;
        }

        if ((err = radio_clear_irq_status_all()) != RADIO_ERROR_NONE) {
            break;
        }

        if ((err = radio_set_irq_mask(LR1110_SYSTEM_IRQ_ALL_MASK))
                != RADIO_ERROR_NONE) {
            break;
        }

        if (lr1110_radio_set_rx_with_timeout_in_rtc_step(&drv_ctx,
            us_to_rtc_ticks(cad_params->fsk_cs_duration_us)) != LR1110_STATUS_OK) {
            err = RADIO_ERROR_HARDWARE_ERROR;
            break;
        }
        drv_ctx.settings_cache.fsk_cad_params = *cad_params;
        drv_ctx.radio_state = SID_PAL_RADIO_RX;
        drv_ctx.cad_exit_mode = exit_mode;
     } while(0);

    return err;
}

int32_t sid_pal_radio_start_continuous_rx(void)
{
    int32_t err;

    do {

        bool pbl_det_timer = false;
        if (drv_ctx.modem == SID_PAL_RADIO_MODEM_MODE_FSK) {
            pbl_det_timer = true;
        }

        if (lr1110_radio_stop_timeout_on_preamble(&drv_ctx, pbl_det_timer) != LR1110_STATUS_OK) {
            err = RADIO_ERROR_HARDWARE_ERROR;
            break;
        }

        if ((err = radio_lr1110_set_radio_mode(true, false, false)) != RADIO_ERROR_NONE) {
            break;
        }

        if ((err = radio_clear_irq_status_all()) != RADIO_ERROR_NONE) {
            break;
        }

        if (lr1110_radio_set_rx_with_timeout_in_rtc_step(&drv_ctx, LR1110_RX_CONTINUOUS_VAL)
                != LR1110_STATUS_OK) {
            err = RADIO_ERROR_HARDWARE_ERROR;
            break;
        }

        drv_ctx.radio_state = SID_PAL_RADIO_RX;
    } while (0);

    return err;
}

int32_t sid_pal_radio_set_rx_duty_cycle(uint32_t rx_time, uint32_t sleep_time)
{
    int32_t err;

    do {
        if (rx_time == 0 || sleep_time == 0) {
            err = RADIO_ERROR_INVALID_PARAMS;
            break;
        }

        if ((err = radio_lr1110_set_radio_mode(true, false, false)) != RADIO_ERROR_NONE) {
            break;
        }

        if ((err = radio_clear_irq_status_all()) != RADIO_ERROR_NONE) {
            break;
        }

        if (lr1110_radio_set_rx_duty_cycle(&drv_ctx, rx_time, sleep_time, LR1110_RADIO_RX_DUTY_CYCLE_MODE_CAD) != LR1110_STATUS_OK) {
            err = RADIO_ERROR_HARDWARE_ERROR;
            break;
        }

        drv_ctx.radio_state = SID_PAL_RADIO_RX_DC;
    } while(0);

    return err;
}

int32_t sid_pal_radio_lora_start_cad(void)
{
    int32_t err;

    do {
        if ((err = radio_lr1110_set_radio_mode(true, false, false)) != RADIO_ERROR_NONE) {
            break;
        }

        if ((err = radio_clear_irq_status_all()) != RADIO_ERROR_NONE) {
            break;
        }

        if (lr1110_radio_set_cad(&drv_ctx) != LR1110_STATUS_OK) {
            err = RADIO_ERROR_HARDWARE_ERROR;
            break;
        }
        drv_ctx.radio_state = SID_PAL_RADIO_CAD;
    } while(0);

    return err;
}

int16_t sid_pal_radio_rssi(void)
{
    int8_t rssi = 0;

    if (lr1110_radio_get_rssi_inst(&drv_ctx, &rssi) != LR1110_STATUS_OK) {
        return INT16_MAX;
    }

    rssi -= drv_ctx.config->lna_gain;
    return rssi;

}

int32_t sid_pal_radio_is_channel_free(uint32_t freq, int16_t threshold, uint32_t delay_us, bool *is_channel_free)
{

    int32_t err, irq_err;
    *is_channel_free = false;

    if ((err = radio_disable_irq(&drv_ctx)) != RADIO_ERROR_NONE) {
        goto ret;
    }

    if ((irq_err = sid_pal_radio_standby()) != RADIO_ERROR_NONE) {
        err = irq_err;
    }

    if ((err =sid_pal_radio_set_frequency(freq)) != RADIO_ERROR_NONE) {
        goto ret;
    }

    if ((err = sid_pal_radio_start_continuous_rx() != RADIO_ERROR_NONE)) {
        goto enable_irq;
    }

    struct sid_timespec t_start, t_cur, t_threshold;
    int16_t rssi;

    if (delay_us < LR1110_MIN_CHANNEL_FREE_DELAY_US) {
        delay_us = LR1110_MIN_CHANNEL_FREE_DELAY_US;
    }

    sid_us_to_timespec(delay_us, &t_threshold);

    if (sid_clock_now(SID_CLOCK_SOURCE_UPTIME, &t_start, NULL) != SID_ERROR_NONE) {
        err = RADIO_ERROR_GENERIC;
        goto enable_irq;
    }

    do {
        sid_pal_delay_us(LR1110_MIN_CHANNEL_FREE_DELAY_US);
        rssi = sid_pal_radio_rssi();
        if (sid_clock_now(SID_CLOCK_SOURCE_UPTIME, &t_cur, NULL) !=
            SID_ERROR_NONE) {
            err = RADIO_ERROR_GENERIC;
            goto enable_irq;
        }
        sid_time_sub(&t_cur, &t_start);
        if (rssi > threshold) {
            goto enable_irq;
        }
      // The minimum time needed in between measurements is about 300
      // micro secs.
    } while(sid_time_gt(&t_threshold, &t_cur));

    *is_channel_free = true;

enable_irq:

    if ((irq_err = sid_pal_radio_standby()) != RADIO_ERROR_NONE) {
        err = irq_err;
    }

    // Do not update err on success of the function calls below
    if ((irq_err = radio_enable_irq(&drv_ctx)) != RADIO_ERROR_NONE) {
        err = irq_err;
        goto ret;
    }
ret:
    return err;
}

int32_t sid_pal_radio_random(uint32_t *random)
{
    int32_t err, irq_err;

    *random = UINT32_MAX;

    if ((err = radio_disable_irq(&drv_ctx)) != RADIO_ERROR_NONE) {
        return err;
    }

    if (lr1110_system_get_random_number(&drv_ctx, random) != LR1110_STATUS_OK) {
        err = RADIO_ERROR_HARDWARE_ERROR;
    }

    if ((irq_err = radio_enable_irq(&drv_ctx)) != RADIO_ERROR_NONE) {
        err = irq_err; // update err only on failure
    }

    return err;
}

int16_t sid_pal_radio_get_ant_dbi(void)
{
    return drv_ctx.regional_radio_param.ant_dbi;
}

int32_t sid_pal_radio_get_cca_level_adjust(sid_pal_radio_data_rate_t data_rate, int8_t *adj_level)
{
    if (data_rate <= SID_PAL_RADIO_DATA_RATE_INVALID || data_rate > SID_PAL_RADIO_DATA_RATE_MAX_NUM) {
        return RADIO_ERROR_INVALID_PARAMS;
    }

    *adj_level = drv_ctx.regional_radio_param.cca_level_adjust[data_rate - 1];
    return RADIO_ERROR_NONE;
}

int32_t sid_pal_radio_get_chan_noise(uint32_t freq, int16_t *noise)
{
    int32_t err, irq_err;

    if ((err = radio_disable_irq(&drv_ctx)) != RADIO_ERROR_NONE) {
        goto ret;
    }

    if ((err = sid_pal_radio_set_frequency(freq)) != RADIO_ERROR_NONE) {
        goto ret;
    }

    if ((err = sid_pal_radio_start_continuous_rx()) != RADIO_ERROR_NONE) {
        goto enable_irq;
    }

    for (uint8_t i = 0; i < LR1110_NOISE_SAMPLE_SIZE; i++) {
        *noise += sid_pal_radio_rssi();
        sid_pal_delay_us(LR1110_MIN_CHANNEL_FREE_DELAY_US);
    }

    *noise /= LR1110_NOISE_SAMPLE_SIZE;

enable_irq:
    // Do not update err on success of the function calls below
    if ((irq_err = radio_enable_irq(&drv_ctx)) != RADIO_ERROR_NONE) {
        err = irq_err;
        goto ret;
    }

    if ((irq_err = sid_pal_radio_standby()) != RADIO_ERROR_NONE) {
        err = irq_err;
    }

ret:
    return err;
}

int32_t sid_pal_radio_get_radio_state_transition_delays(sid_pal_radio_state_transition_timings_t *state_delay)
{
    *state_delay = drv_ctx.config->state_timings;
    if (drv_ctx.config->tcxo_config.ctrl != LR1110_TCXO_CTRL_NONE) {
        state_delay->tcxo_delay_us = LR1110_TUS_TO_US(drv_ctx.config->tcxo_config.timeout);
    }
    return RADIO_ERROR_NONE;
}

int32_t sid_pal_radio_init(sid_pal_radio_event_notify_t notify, sid_pal_radio_irq_handler_t dio_irq_handler, sid_pal_radio_rx_packet_t *rx_packet)
{
    int32_t err;
    int8_t tx_power;

    do {
        if (notify == NULL || dio_irq_handler == NULL || rx_packet == NULL) {
            err = RADIO_ERROR_INVALID_PARAMS;
            break;
        }

        drv_ctx.radio_rx_packet = rx_packet;
        drv_ctx.report_radio_event = notify;
        drv_ctx.irq_handler = dio_irq_handler;
        drv_ctx.modem = SID_PAL_RADIO_MODEM_MODE_LORA;

        sid_pal_radio_set_region(drv_ctx.config->regional_config.radio_region);

        if ((err = radio_lr1110_platform_init()) != RADIO_ERROR_NONE) {
            break;
        }

        if ((err = radio_lr1110_set_radio_mode(false, false, false)) != RADIO_ERROR_NONE) {
            break;
        }

        if (lr1110_system_reset(&drv_ctx) != LR1110_STATUS_OK) {
            err = RADIO_ERROR_IO_ERROR;
            break;
        }

        if (lr1110_system_get_version(&drv_ctx,&drv_ctx.ver) != LR1110_STATUS_OK) {
            err = RADIO_ERROR_IO_ERROR;
            break;
        };
        SID_HAL_LOG_INFO("LR1110: VER HW 0x%.2X FW 0x%.4X", drv_ctx.ver.hw, drv_ctx.ver.fw);

        if (drv_ctx.config->tcxo_config.ctrl != LR1110_TCXO_CTRL_NONE) {
            lr1110_status_t status = lr1110_system_set_tcxo_mode(&drv_ctx, drv_ctx.config->tcxo_config.tune,
                    drv_ctx.config->tcxo_config.timeout);
            if (status != LR1110_STATUS_OK) {
                err = RADIO_ERROR_IO_ERROR;
                break;
            }

            /* TCXO usage automatically generates error. Clear it */
            if (lr1110_system_clear_errors(&drv_ctx) != LR1110_STATUS_OK) {
                err = RADIO_ERROR_IO_ERROR;
                break;
            }

            /* Calibrate HF_XOSC that failed on startup due to TCXO usage */
            const uint8_t callibrate_all = LR1110_SYSTEM_CALIB_LF_RC_MASK | LR1110_SYSTEM_CALIB_HF_RC_MASK |
                                               LR1110_SYSTEM_CALIB_PLL_MASK | LR1110_SYSTEM_CALIB_ADC_MASK |
                                               LR1110_SYSTEM_CALIB_IMG_MASK | LR1110_SYSTEM_CALIB_PLL_TX_MASK;
            if (lr1110_system_calibrate(&drv_ctx, callibrate_all) != LR1110_STATUS_OK) {
                err = RADIO_ERROR_IO_ERROR;
                break;
            }

            if (lr1110_radio_set_rx_tx_fallback_mode(&drv_ctx, LR1110_RADIO_FALLBACK_STDBY_XOSC) != LR1110_STATUS_OK) {
                err = RADIO_ERROR_IO_ERROR;
                break;
            }
        }

        if (lr1110_system_cfg_lfclk(&drv_ctx, drv_ctx.config->lfclock_cfg, true) != LR1110_STATUS_OK) {
            err = RADIO_ERROR_IO_ERROR;
            break;
        }

        drv_ctx.irq_mask = LR1110_DEFAULT_LORA_IRQ_MASK;
        if (sid_pal_gpio_set_irq(drv_ctx.config->gpios.int1,
            SID_PAL_GPIO_IRQ_TRIGGER_EDGE, radio_irq, NULL) != SID_ERROR_NONE) {
            err = RADIO_ERROR_IO_ERROR;
            break;
        }

        // Force Standby RC mode
        if (lr1110_system_set_standby(&drv_ctx, LR1110_SYSTEM_STANDBY_CFG_RC) != LR1110_STATUS_OK) {
            break;
        }

        err = RADIO_ERROR_HARDWARE_ERROR;
        if (lr1110_system_set_reg_mode(&drv_ctx, drv_ctx.config->regulator_mode) != LR1110_STATUS_OK) {
            break;
        }

        if (drv_ctx.config->rfswitch.enable &&
            lr1110_system_set_dio_as_rf_switch(&drv_ctx, &drv_ctx.config->rfswitch) != RADIO_ERROR_NONE) {
            err =  RADIO_ERROR_HARDWARE_ERROR;
            break;
        }

        // Set Standby mode from config
        drv_ctx.radio_state = SID_PAL_RADIO_UNKNOWN;
        if ((err = sid_pal_radio_standby()) != RADIO_ERROR_NONE) {
            break;
        }

        if (lr1110_radio_cfg_rx_boosted(&drv_ctx, drv_ctx.config->rx_boost) != LR1110_STATUS_OK) {
            break;
        }

        if ((err = sid_pal_radio_get_max_tx_power(SID_PAL_RADIO_DATA_RATE_50KBPS, &tx_power)) != RADIO_ERROR_NONE) {
            break;
        }

        if ((err =sid_pal_radio_set_tx_power(tx_power)) != RADIO_ERROR_NONE) {
            break;
        }

        lr1110_system_irq_mask_t irq_status = 0;
        if (lr1110_system_get_and_clear_irq_status(&drv_ctx, &irq_status) != LR1110_STATUS_OK) {
            err =  RADIO_ERROR_HARDWARE_ERROR;
            break;
        }

        if ((err = radio_enable_irq(&drv_ctx)) != RADIO_ERROR_NONE) {
            break;
        }

        (void)sid_hal_set_sleep_start_notify_cb(NULL);

    } while (0);

    return err;
}

int32_t sid_pal_radio_deinit(void)
{
    return RADIO_ERROR_NONE;
}
