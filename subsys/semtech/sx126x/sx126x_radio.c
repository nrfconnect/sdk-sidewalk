/*
 * Copyright (c) 2019-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * This file supports radio HAL interface
 */

#include "sx126x_radio.h"

#include <sid_error.h>

#include <sid_clock_ifc.h>
#include <sid_pal_delay_ifc.h>
#include <sid_time_ops.h>
#include <sid_time_types.h>

#ifdef MARS_SPI_BUS_WORKAROUND
#include "board_hal.h"
#endif

#define SX126X_DEFAULT_LORA_IRQ_MASK       (RADIO_IRQ_ALL & ~(RADIO_IRQ_PREAMBLE_DETECT | \
                                            RADIO_IRQ_VALID_SYNC_WORD))

#define SX126X_DEFAULT_FSK_IRQ_MASK        (RADIO_IRQ_ALL & ~RADIO_IRQ_PREAMBLE_DETECT)

#define SX1261_DEFAULT_OCP_VAL             0x18

#define SX1262_DEFAULT_OCP_VAL             0x38

#define SX126X_DEFAULT_TRIM_CAP_VAL        0x1212

#define SX126X_RX_CONTINUOUS_VAL           0xFFFFFF
#define SX126X_MIN_CHANNEL_FREE_DELAY_US   1
#define SX126X_MIN_CHANNEL_NOISE_DELAY_US  30
#define SX126X_NOISE_SAMPLE_SIZE           32

// Delay time to allow for any external PA/FEM turn ON/OFF
#define SEMTECH_STDBY_STATE_DELAY_US       10
#define SEMTECH_MAX_WAIT_ON_BUSY_CNT_US    2000

#define SX126X_TCXO_VDD_TIMEOUT_DURATION   1

#if defined (RADIO_SX1262_TXPWR_WORKAROUND) && RADIO_SX1262_TXPWR_WORKAROUND

#define SX1262_BAND_EDGE_LIMIT_FREQ        903000000
#define SX1262_REG_VAL_FREQ_LOW            0x1F
#define SX1262_REG_VAL_FREQ_HIGH           0x18

#endif // defined (RADIO_SX1262_TXPWR_WORKAROUND) && RADIO_SX1262_TXPWR_WORKAROUND

#define SX126X_CAD_DEFAULT_TX_TIMEOUT      0 // disable Tx timeout for CAD

static halo_drv_semtech_ctx_t              drv_ctx = {0};

static int32_t radio_sx126x_platform_init(void)
{
    int32_t err = RADIO_ERROR_INVALID_PARAMS;

#ifdef BOARD_HAL_IO_EXPANDER_SUBG_BAND_PIN
        if (sid_pal_gpio_set_direction(BOARD_HAL_EXP_GPIO( BOARD_HAL_IO_EXPANDER_SUBG_BAND_PIN),
            SID_PAL_GPIO_DIRECTION_OUTPUT) != SID_ERROR_NONE) {
            goto ret;
        }

        if (sid_pal_gpio_write(BOARD_HAL_EXP_GPIO( BOARD_HAL_IO_EXPANDER_SUBG_BAND_PIN), 0) != SID_ERROR_NONE) {
            goto ret;
        }
#endif

    if (drv_ctx.config->gpio_radio_busy != HALO_GPIO_NOT_CONNECTED) {
        if (sid_pal_gpio_set_direction(drv_ctx.config->gpio_radio_busy,
            SID_PAL_GPIO_DIRECTION_INPUT) != SID_ERROR_NONE) {
            goto ret;
        }
    }

    if (drv_ctx.config->gpio_tx_bypass != HALO_GPIO_NOT_CONNECTED) {
        if (sid_pal_gpio_set_direction(drv_ctx.config->gpio_tx_bypass,
            SID_PAL_GPIO_DIRECTION_OUTPUT) != SID_ERROR_NONE) {
            goto ret;
        }
    }

    if (drv_ctx.config->gpio_rf_sw_ena != HALO_GPIO_NOT_CONNECTED) {
        if (sid_pal_gpio_set_direction(drv_ctx.config->gpio_rf_sw_ena,
            SID_PAL_GPIO_DIRECTION_OUTPUT) != SID_ERROR_NONE) {
            goto ret;
        }
    }

    if (drv_ctx.config->gpio_power != HALO_GPIO_NOT_CONNECTED) {
        if (sid_pal_gpio_set_direction(drv_ctx.config->gpio_power,
            SID_PAL_GPIO_DIRECTION_OUTPUT) != SID_ERROR_NONE) {
            goto ret;
        }
    }

    if (drv_ctx.config->pa_cfg_callback == NULL) {
        goto ret;
    }

    if (drv_ctx.config->trim_cap_val_callback != NULL) {
        drv_ctx.config->trim_cap_val_callback(&drv_ctx.trim);
    } else {
        drv_ctx.trim = SX126X_DEFAULT_TRIM_CAP_VAL;
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

static int32_t radio_set_irq_mask(uint16_t irq_mask)
{
    if (sx126x_set_dio_irq_params(&drv_ctx, irq_mask, irq_mask,
          RADIO_IRQ_NONE, RADIO_IRQ_NONE) != SX126X_STATUS_OK) {
        return RADIO_ERROR_IO_ERROR;
    }
    return RADIO_ERROR_NONE;
}

static int32_t radio_enable_irq(void)
{
    if (sx126x_set_dio_irq_params(&drv_ctx, drv_ctx.irq_mask, drv_ctx.irq_mask,
          RADIO_IRQ_NONE, RADIO_IRQ_NONE) != SX126X_STATUS_OK) {
        return RADIO_ERROR_IO_ERROR;
    }
    return RADIO_ERROR_NONE;
}

static int32_t radio_disable_irq(void)
{
    if (sx126x_set_dio_irq_params(&drv_ctx, RADIO_IRQ_NONE, RADIO_IRQ_NONE,
          RADIO_IRQ_NONE, RADIO_IRQ_NONE) != SX126X_STATUS_OK) {
        return RADIO_ERROR_IO_ERROR;
    }
    return RADIO_ERROR_NONE;
}

static int32_t sx126x_check_status(void)
{
    uint8_t is_radio_busy;
    if (sid_pal_gpio_read(drv_ctx.config->gpio_radio_busy,
                          &is_radio_busy) != SID_ERROR_NONE) {
        return RADIO_ERROR_HARDWARE_ERROR;
    }

    return is_radio_busy ? RADIO_ERROR_BUSY: RADIO_ERROR_NONE;
}

static int32_t radio_set_modem_to_lora_mode(void)
{
    if (sx126x_set_pkt_type(&drv_ctx, SX126X_PKT_TYPE_LORA) != SX126X_STATUS_OK) {
        return RADIO_ERROR_HARDWARE_ERROR;
    }
    drv_ctx.modem = SID_PAL_RADIO_MODEM_MODE_LORA;
    drv_ctx.irq_mask = SX126X_DEFAULT_LORA_IRQ_MASK;

    if (radio_enable_irq() != RADIO_ERROR_NONE) {
        return RADIO_ERROR_HARDWARE_ERROR;
    }
    return RADIO_ERROR_NONE;
}

static int32_t radio_set_modem_to_fsk_mode(void)
{
    if (sx126x_set_pkt_type(&drv_ctx, SX126X_PKT_TYPE_GFSK) != SX126X_STATUS_OK) {
        return RADIO_ERROR_HARDWARE_ERROR;
    }
    drv_ctx.modem = SID_PAL_RADIO_MODEM_MODE_FSK;
    drv_ctx.irq_mask = SX126X_DEFAULT_FSK_IRQ_MASK;

    if (radio_enable_irq() != RADIO_ERROR_NONE) {
        return RADIO_ERROR_HARDWARE_ERROR;
    }
    return RADIO_ERROR_NONE;
}

static int32_t set_trim_cap_val_to_radio(uint8_t xta, uint8_t xtb)
{
    int32_t err = RADIO_ERROR_NONE;

    // Trim value will be taken care after calling sx126x_set_dio3_as_tcxo_ctrl()
    if (drv_ctx.config->tcxo.ctrl != SX126X_TCXO_CTRL_NONE) {
        goto ret;
    }

    if (drv_ctx.radio_state == SID_PAL_RADIO_SLEEP || drv_ctx.radio_state == SID_PAL_RADIO_UNKNOWN) {
        if (sx126x_wakeup(&drv_ctx) != SX126X_STATUS_OK) {
            err = RADIO_ERROR_HARDWARE_ERROR;
            goto ret;
        }
    }

    // trim cappacitors can be set only in standby_xosc mode. switching standby
    // mode to xosc
    if (sx126x_set_standby(&drv_ctx, SX126X_STANDBY_CFG_XOSC) != SX126X_STATUS_OK) {
        err =  RADIO_ERROR_HARDWARE_ERROR;
        goto ret;
    }

    sid_pal_delay_us(SEMTECH_STDBY_STATE_DELAY_US);

    if (sx126x_set_xtal_trim(&drv_ctx, xta, xtb) != SX126X_STATUS_OK) {
        err = RADIO_ERROR_IO_ERROR;
        goto ret;
    }

ret:
    return err;
}

static int32_t radio_clear_irq_status_all(void)
{
    if (sx126x_clear_irq_status(&drv_ctx, SX126X_IRQ_ALL) != SX126X_STATUS_OK) {
        return RADIO_ERROR_HARDWARE_ERROR;
    }
    return RADIO_ERROR_NONE;
}

const halo_drv_semtech_ctx_t* sx126x_get_drv_ctx(void)
{
    return &drv_ctx;
}

int32_t sx126x_wait_on_busy(void)
{
    uint16_t cnt = 0;

    while(cnt++ < SEMTECH_MAX_WAIT_ON_BUSY_CNT_US) {
        if (sx126x_check_status() == RADIO_ERROR_NONE) {
            break;
        }

        sid_pal_delay_us(SEMTECH_STDBY_STATE_DELAY_US);
    }

    if (cnt >= SEMTECH_MAX_WAIT_ON_BUSY_CNT_US) {
        return RADIO_ERROR_HARDWARE_ERROR;
    }

    return RADIO_ERROR_NONE;
}

void set_lora_exit_mode(sid_pal_radio_cad_param_exit_mode_t cad_exit_mode)
{
    drv_ctx.cad_exit_mode = cad_exit_mode;
}

int32_t sx126x_radio_bus_xfer(const uint8_t *cmd_buffer, uint16_t cmd_buffer_size, uint8_t *buffer,
                                   uint16_t size, uint8_t read_offset)
{

    if (drv_ctx.config->internal_buffer.p == NULL || cmd_buffer == NULL) {
        return RADIO_ERROR_INVALID_PARAMS;
    }

    if (drv_ctx.config->internal_buffer.size < (size + cmd_buffer_size)) {
        return RADIO_ERROR_NOMEM;
    }

    memcpy(drv_ctx.config->internal_buffer.p, cmd_buffer, cmd_buffer_size);
    if (buffer != NULL) {
        memcpy(&drv_ctx.config->internal_buffer.p[cmd_buffer_size], buffer, size);
    }

    const struct sid_pal_serial_bus_iface *bus_iface = drv_ctx.bus_iface;

    if (bus_iface->xfer(bus_iface, &drv_ctx.config->bus_selector,
        drv_ctx.config->internal_buffer.p,
#if MARS_SPI_BUS_WORKAROUND
        &drv_ctx.config->internal_buffer.p[read_offset? read_offset: 0],
#else
        drv_ctx.config->internal_buffer.p,
#endif
        size + cmd_buffer_size) != SID_ERROR_NONE) {
        return RADIO_ERROR_IO_ERROR;
    }

    if (read_offset != 0 && buffer != NULL) {
        memcpy(buffer, &drv_ctx.config->internal_buffer.p[read_offset], size);
    }

    return RADIO_ERROR_NONE;
}

int32_t radio_sx126x_set_radio_mode(bool rf_en, bool tx_en)
{
    int32_t err = RADIO_ERROR_NONE;

    do {
        if (rf_en == false && tx_en == true) {
            /* Invalid pin config */
            err = RADIO_ERROR_INVALID_PARAMS;
            break;
        }

        if (drv_ctx.config->gpio_rf_sw_ena != HALO_GPIO_NOT_CONNECTED) {
            if (sid_pal_gpio_write(drv_ctx.config->gpio_rf_sw_ena, rf_en) != SID_ERROR_NONE) {
                err = RADIO_ERROR_IO_ERROR;
                break;
            }
        }

        if (drv_ctx.config->gpio_tx_bypass != HALO_GPIO_NOT_CONNECTED) {
            if (sid_pal_gpio_write(drv_ctx.config->gpio_tx_bypass, tx_en) != SID_ERROR_NONE) {
                err = RADIO_ERROR_IO_ERROR;
                break;
            }
        }
    } while (0);

    return err;
}

void set_radio_sx126x_device_config(const radio_sx126x_device_config_t *cfg)
{
    drv_ctx.config = cfg;
}

uint8_t sid_pal_radio_get_status(void)
{
    return drv_ctx.radio_state;
}

sid_pal_radio_modem_mode_t sid_pal_radio_get_modem_mode(void)
{
    return (drv_ctx.modem == SID_PAL_RADIO_MODEM_MODE_FSK) ? SID_PAL_RADIO_MODEM_MODE_FSK
                                                : SID_PAL_RADIO_MODEM_MODE_LORA;
}

int32_t sid_pal_radio_set_modem_mode(sid_pal_radio_modem_mode_t mode)
{
    if (mode == SID_PAL_RADIO_MODEM_MODE_LORA) {
        return radio_set_modem_to_lora_mode();
    }

    if (mode ==  SID_PAL_RADIO_MODEM_MODE_FSK) {
        return radio_set_modem_to_fsk_mode();
    }

    return RADIO_ERROR_NOT_SUPPORTED;
}

int32_t sid_pal_radio_irq_process(void)
{
    sid_pal_radio_events_t radio_event = SID_PAL_RADIO_EVENT_UNKNOWN;
    sx126x_irq_mask_t irq_status;
    int32_t err;

    do {
        if ((err = radio_disable_irq()) != RADIO_ERROR_NONE) {
            break;
        }

        if (sx126x_get_and_clear_irq_status(&drv_ctx, &irq_status) != SX126X_STATUS_OK) {
            break;
        }

        if (irq_status & SX126X_IRQ_TX_DONE) {
            radio_event = SID_PAL_RADIO_EVENT_TX_DONE;
            break;
        }

        if (irq_status & SX126X_IRQ_CRC_ERROR) {
            radio_event = SID_PAL_RADIO_EVENT_RX_ERROR;
            break;
        }

        if (irq_status & SX126X_IRQ_TIMEOUT) {
            radio_event =
                    (drv_ctx.radio_state == SID_PAL_RADIO_TX) ?
                            SID_PAL_RADIO_EVENT_TX_TIMEOUT : SID_PAL_RADIO_EVENT_RX_TIMEOUT;
            if (drv_ctx.cad_exit_mode == SID_PAL_RADIO_CAD_EXIT_MODE_CS_LBT
                    && radio_event == SID_PAL_RADIO_EVENT_RX_TIMEOUT) {
                drv_ctx.cad_exit_mode = SID_PAL_RADIO_CAD_EXIT_MODE_NONE;
#if HALO_ENABLE_DIAGNOSTICS
                radio_event = SID_PAL_RADIO_EVENT_CS_TIMEOUT;
#else
                radio_event = SID_PAL_RADIO_EVENT_UNKNOWN;
                sid_pal_radio_start_tx(SX126X_CAD_DEFAULT_TX_TIMEOUT);
#endif
            }
            break;
        }

        if (irq_status & SX126X_IRQ_HEADER_ERROR) {
            radio_event = SID_PAL_RADIO_EVENT_HEADER_ERROR; // HEADER CRC
            break;
        }

        if (drv_ctx.modem == SID_PAL_RADIO_MODEM_MODE_LORA) {
            if (irq_status & SX126X_IRQ_SYNC_WORD_VALID) {
                radio_event = SID_PAL_RADIO_EVENT_UNKNOWN;
                break;
            }

            if (irq_status & SX126X_IRQ_CAD_DONE) {
                radio_event = (irq_status & SX126X_IRQ_CAD_DET) ?
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

            if ((irq_status & SX126X_IRQ_RX_DONE) && !(irq_status & SX126X_IRQ_CRC_ERROR) && DUAL_LINK_SUPPORT) {
                if ((err = radio_lora_process_rx_done(&drv_ctx)) == RADIO_ERROR_NONE) {
                    memset(&drv_ctx.radio_rx_packet->fsk_rx_packet_status, 0, sizeof(sid_pal_radio_fsk_rx_packet_status_t));
                    radio_event = SID_PAL_RADIO_EVENT_RX_DONE;
                }
                break;
            }
        } else if (drv_ctx.modem == SID_PAL_RADIO_MODEM_MODE_FSK) {
            if (irq_status & SX126X_IRQ_SYNC_WORD_VALID) {
#ifndef MARS_FSK_SHORT_PACKET_WORKAROUND
                // Temporary solution for short packets
                // Moved to SX126X_IRQ_RX_DONE
                radio_fsk_process_sync_word_detected(&drv_ctx);
#endif
                break;
            }

            if (irq_status & SX126X_IRQ_PBL_DET) {
                if (drv_ctx.cad_exit_mode == SID_PAL_RADIO_CAD_EXIT_MODE_CS_LBT) {
                    drv_ctx.radio_rx_packet->fsk_rx_packet_status.rssi_sync = (int8_t) sid_pal_radio_rssi();
                    radio_event = SID_PAL_RADIO_EVENT_CS_DONE;
                    drv_ctx.cad_exit_mode = SID_PAL_RADIO_CAD_EXIT_MODE_NONE;
                    sid_pal_radio_standby();
                }
                break;
            }

            // CRC check not necessary for fsk
            if (irq_status & SX126X_IRQ_RX_DONE) {
                radio_fsk_rx_done_status_t fsk_rx_done_status;
                sid_clock_now(SID_CLOCK_SOURCE_UPTIME, &drv_ctx.radio_rx_packet->rcv_tm, NULL);
#ifdef MARS_FSK_SHORT_PACKET_WORKAROUND
                // Temporary solution for short packets
                radio_fsk_process_sync_word_detected(&drv_ctx);
#endif
                if ((err = radio_fsk_process_rx_done(&drv_ctx, &fsk_rx_done_status)) == RADIO_ERROR_NONE) {
                    memset(&drv_ctx.radio_rx_packet->lora_rx_packet_status, 0, sizeof(sid_pal_radio_lora_rx_packet_status_t));
                    radio_event = SID_PAL_RADIO_EVENT_RX_DONE;
                } else if (err == RADIO_ERROR_GENERIC) {
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
                        case RADIO_FSK_RX_DONE_STATUS_OK:
                        case RADIO_FSK_RX_DONE_STATUS_INVALID_PARAMETER:
                        case RADIO_FSK_RX_DONE_STATUS_INVALID_LENGTH:
                        case RADIO_FSK_RX_DONE_STATUS_TIMEOUT:
                            // Do nothing
                            break;
                    }
                }
            }
        }
    } while(0);

    if (SID_PAL_RADIO_EVENT_UNKNOWN != radio_event) {
        drv_ctx.report_radio_event(radio_event);
    }

    if (drv_ctx.radio_state != SID_PAL_RADIO_SLEEP) {
        err = radio_enable_irq();
    }

    return err;
}

int32_t set_radio_sx126x_trim_cap_val(uint16_t trim)
{
    int32_t err = RADIO_ERROR_NONE;

    if ((err = set_trim_cap_val_to_radio(trim >> 8, trim & 0xFF)) != RADIO_ERROR_NONE) {
        return err;
    }

    drv_ctx.trim = trim;

    return err;
}

uint16_t get_radio_sx126x_trim_cap_val(void)
{
    return drv_ctx.trim;
}

int32_t sid_pal_radio_set_frequency(uint32_t freq)
{
    int32_t err = RADIO_ERROR_NONE;

    do {
       if (drv_ctx.radio_state != SID_PAL_RADIO_STANDBY) {
           err =  RADIO_ERROR_INVALID_STATE;
           break;
       }

       sx126x_freq_cal_band cur_freq_band, freq_band;
       cur_freq_band = sx126x_get_freq_band(drv_ctx.radio_freq_hz);
       freq_band = sx126x_get_freq_band(freq);

       if (cur_freq_band != freq_band) {
           if(sx126x_cal_img(&drv_ctx, freq) != SX126X_STATUS_OK) {
               err = RADIO_ERROR_HARDWARE_ERROR;
               break;
           }
       }

#if defined (RADIO_SX1262_TXPWR_WORKAROUND) && RADIO_SX1262_TXPWR_WORKAROUND
       if (freq_band == SX126X_BAND_900M) {
            uint8_t reg_val;

            if (freq <= SX1262_BAND_EDGE_LIMIT_FREQ) {
                reg_val = SX1262_REG_VAL_FREQ_LOW;
            } else {
                reg_val = SX1262_REG_VAL_FREQ_HIGH;
            }

            if (sx126x_write_register(&drv_ctx, SX126X_REG_LR_CFG_FREQ, &reg_val, 1)
                != SX126X_STATUS_OK) {
                break;
            }
       }
#endif // defined (RADIO_SX1262_TXPWR_WORKAROUND) && RADIO_SX1262_TXPWR_WORKAROUND

       if (sx126x_set_rf_freq(&drv_ctx, freq) != SX126X_STATUS_OK) {
           err = RADIO_ERROR_HARDWARE_ERROR;
           break;
       }

#ifdef BOARD_HAL_IO_EXPANDER_SUBG_BAND_PIN
        if (cur_freq_band != freq_band) {
            if (freq_band == SX126X_BAND_900M) {
                if (sid_pal_gpio_write(BOARD_HAL_EXP_GPIO( BOARD_HAL_IO_EXPANDER_SUBG_BAND_PIN), 0) != SID_ERROR_NONE) {
                    err = RADIO_ERROR_HARDWARE_ERROR;
                    break;
                }
            } else if (freq_band == SX126X_BAND_850M) {
                if (sid_pal_gpio_write(BOARD_HAL_EXP_GPIO( BOARD_HAL_IO_EXPANDER_SUBG_BAND_PIN), 1) != SID_ERROR_NONE) {
                    err = RADIO_ERROR_HARDWARE_ERROR;
                    break;
                }
            }
        }
#endif

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

    radio_sx126x_pa_cfg_t cur_cfg;

    cur_cfg.pa_duty_cycle = cfg->pa_duty_cycle;
    cur_cfg.hp_max = cfg->hp_max;
    cur_cfg.device_sel = cfg->device_sel;
    cur_cfg.pa_lut = cfg->pa_lut;
    cur_cfg.tx_power = cfg->tx_power;
    cur_cfg.ramp_time = cfg->ramp_time;
    cur_cfg.enable_ext_pa = cfg->enable_ext_pa;

    drv_ctx.pa_cfg = cur_cfg;
    drv_ctx.pa_cfg_configured = true;
    return RADIO_ERROR_NONE;
}
#endif

int32_t get_radio_sx126x_pa_config(radio_sx126x_pa_cfg_t *cfg)
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

    radio_sx126x_pa_cfg_t *pa_cfg = &drv_ctx.pa_cfg;
    sx126x_pa_cfg_params_t cfg;
    uint8_t reg_val;

    cfg.pa_duty_cycle = pa_cfg->pa_duty_cycle;
    cfg.hp_max = pa_cfg->hp_max;
    cfg.device_sel = pa_cfg->device_sel;
    cfg.pa_lut = pa_cfg->pa_lut;

    if (drv_ctx.config->id == SEMTECH_ID_SX1261) {
        reg_val = SX1261_DEFAULT_OCP_VAL;
    } else if (drv_ctx.config->id == SEMTECH_ID_SX1262) {
        reg_val = SX1262_DEFAULT_OCP_VAL;
    } else {
        err = RADIO_ERROR_NOT_SUPPORTED;
    }

    if (err == RADIO_ERROR_NONE) {
        err = RADIO_ERROR_IO_ERROR;
        do {
            if (sx126x_set_pa_cfg(&drv_ctx, &cfg) != SX126X_STATUS_OK) {
                break;
            }

            if (sx126x_write_register(&drv_ctx, SX126X_REG_OCP, &reg_val, 1)
                != SX126X_STATUS_OK) {
                break;
            }

#if defined (RADIO_SX1262_TXPWR_WORKAROUND) && RADIO_SX1262_TXPWR_WORKAROUND
            uint8_t ovp_reg;
            if (sx126x_read_register(&drv_ctx, SX126X_REG_OVP, &ovp_reg, 1)
                != SX126X_STATUS_OK) {
                break;
            }

            ovp_reg &= 0xF9; // Set bits 1 and 2 to 0
            if (sx126x_write_register(&drv_ctx, SX126X_REG_OVP, &ovp_reg, 1)
                != SX126X_STATUS_OK) {
                break;
            }
#endif // defined (RADIO_SX1262_TXPWR_WORKAROUND) && RADIO_SX1262_TXPWR_WORKAROUND

            if (sx126x_set_tx_params(&drv_ctx, pa_cfg->tx_power,
                  pa_cfg->ramp_time) != SX126X_STATUS_OK) {
                break;
            }
            err = RADIO_ERROR_NONE;
        } while (0);
    }
    return err;
}

int32_t sid_pal_radio_sleep(uint32_t sleep_us)
{
    int32_t err;

    do {
        if (drv_ctx.radio_state == SID_PAL_RADIO_SLEEP) {
           err = RADIO_ERROR_NONE;
           break;
        }

        if ((err = radio_clear_irq_status_all()) != RADIO_ERROR_NONE) {
            break;
        }


        if ((err = radio_sx126x_set_radio_mode(false, false)) != RADIO_ERROR_NONE) {
            break;
        }

        if (sx126x_set_sleep(&drv_ctx, SX126X_SLEEP_CFG_WARM_START)
                      != SX126X_STATUS_OK) {
            err = RADIO_ERROR_HARDWARE_ERROR;
            break;
        }

        set_gpio_cfg_sleep(&drv_ctx);
        drv_ctx.radio_state = SID_PAL_RADIO_SLEEP;
    } while(0);

    return err;
}

int32_t sid_pal_radio_standby(void)
{
    int32_t err = RADIO_ERROR_NONE;

    do {
        if (drv_ctx.radio_state == SID_PAL_RADIO_STANDBY) {
           break;
        }

        if (drv_ctx.radio_state == SID_PAL_RADIO_SLEEP || drv_ctx.radio_state == SID_PAL_RADIO_UNKNOWN) {
            if (sx126x_wakeup(&drv_ctx) != SX126X_STATUS_OK) {
                err = RADIO_ERROR_HARDWARE_ERROR;
                break;
            }

            if (drv_ctx.radio_state == SID_PAL_RADIO_SLEEP) {
                // after wake up Semtech will be in STDBY_RC mode
                // this prevent unnecessary checks in sx126x_hal_write()->sx126x_wait_for_device_ready()
                // and tries to wakeup Semtech
                drv_ctx.radio_state = SID_PAL_RADIO_STANDBY;
            }
        }

        if ((err = radio_clear_irq_status_all()) != RADIO_ERROR_NONE) {
            break;
        }

        if ((err = radio_sx126x_set_radio_mode(false, false)) != RADIO_ERROR_NONE) {
            break;
        }

        sid_pal_delay_us(SEMTECH_STDBY_STATE_DELAY_US);

        // if we move from SID_PAL_RADIO_SLEEP state we already in STDBY_RC mode
        if (drv_ctx.radio_state != SID_PAL_RADIO_STANDBY) {
            // TODO: Revisit when supporting TCXO
            if (sx126x_set_standby(&drv_ctx, SX126X_STANDBY_CFG_RC) != SX126X_STATUS_OK) {
                err = RADIO_ERROR_HARDWARE_ERROR;
                break;
            }
        }

        drv_ctx.radio_state = SID_PAL_RADIO_STANDBY;
    } while(0);

    return err;
}

int32_t sid_pal_radio_set_tx_payload(const uint8_t *buffer, uint8_t size)
{
    if (buffer == NULL || size == 0) {
        return RADIO_ERROR_INVALID_PARAMS;
    }
    if (sx126x_write_buffer(&drv_ctx, 0x00, buffer, size) != SX126X_STATUS_OK) {
        return RADIO_ERROR_IO_ERROR;
    }
    return RADIO_ERROR_NONE;
}

int32_t sid_pal_radio_start_tx(uint32_t timeout)
{
    int32_t err;

    do {
        if ((err = set_trim_cap_val_to_radio(drv_ctx.trim >> 8, drv_ctx.trim & 0xFF))
                   != RADIO_ERROR_NONE) {
            break;
        }

        if ((err = radio_sx126x_set_radio_mode(true, drv_ctx.pa_cfg.enable_ext_pa)) != RADIO_ERROR_NONE) {
            break;
        }

        if ((err = radio_clear_irq_status_all()) != RADIO_ERROR_NONE) {
            break;
        }

        if (sx126x_set_tx(&drv_ctx, US_TO_SEMTEC_TICKS(timeout)) != SX126X_STATUS_OK) {
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

        if ((err = set_trim_cap_val_to_radio(drv_ctx.trim >> 8, drv_ctx.trim & 0xFF))
                   != RADIO_ERROR_NONE) {
            break;
        }

        if ((err = radio_sx126x_set_radio_mode(true, drv_ctx.pa_cfg.enable_ext_pa)) != RADIO_ERROR_NONE) {
            break;
        }

        if ((err = radio_clear_irq_status_all()) != RADIO_ERROR_NONE) {
            break;
        }

        if (sx126x_set_tx_cw(&drv_ctx) != SX126X_STATUS_OK) {
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

        if (sx126x_stop_tmr_on_pbl(&drv_ctx, pbl_det_timer) != SX126X_STATUS_OK) {
            err = RADIO_ERROR_HARDWARE_ERROR;
            break;
        }

        if ((err = set_trim_cap_val_to_radio(drv_ctx.trim >> 8, drv_ctx.trim & 0xFF))
                   != RADIO_ERROR_NONE) {
            break;
        }

        if ((err = radio_sx126x_set_radio_mode(true, false)) != RADIO_ERROR_NONE) {
            break;
        }

        if ((err = radio_clear_irq_status_all()) != RADIO_ERROR_NONE) {
            break;
        }

        if (sx126x_set_rx(&drv_ctx, US_TO_SEMTEC_TICKS(timeout)) != SX126X_STATUS_OK) {
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

        if (sx126x_stop_tmr_on_pbl(&drv_ctx, true) != SX126X_STATUS_OK) {
            err = RADIO_ERROR_HARDWARE_ERROR;
            break;
        }

        if ((err = set_trim_cap_val_to_radio(drv_ctx.trim >> 8, drv_ctx.trim & 0xFF))
                   != RADIO_ERROR_NONE) {
            break;
        }

        if ((err = radio_sx126x_set_radio_mode(true, false)) != RADIO_ERROR_NONE) {
            break;
        }

        if ((err = radio_clear_irq_status_all()) != RADIO_ERROR_NONE) {
            break;
        }

        if ((err = radio_set_irq_mask(
                RADIO_IRQ_ALL & (RADIO_IRQ_TXRX_TIMEOUT | RADIO_IRQ_PREAMBLE_DETECT)))
                != RADIO_ERROR_NONE) {
            break;
        }

        if (sx126x_set_rx(&drv_ctx, US_TO_SEMTEC_TICKS(cad_params->fsk_cs_duration_us)) != SX126X_STATUS_OK) {
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

        if (sx126x_stop_tmr_on_pbl(&drv_ctx, pbl_det_timer) != SX126X_STATUS_OK) {
            err = RADIO_ERROR_HARDWARE_ERROR;
            break;
        }

        if ((err = set_trim_cap_val_to_radio(drv_ctx.trim >> 8, drv_ctx.trim & 0xFF))
                != RADIO_ERROR_NONE) {
            break;
        }

        if ((err = radio_sx126x_set_radio_mode(true, false)) != RADIO_ERROR_NONE) {
            break;
        }

        if ((err = radio_clear_irq_status_all()) != RADIO_ERROR_NONE) {
            break;
        }

        if (sx126x_set_rx(&drv_ctx, SX126X_RX_CONTINUOUS_VAL) != SX126X_STATUS_OK) {
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

        if ((err = set_trim_cap_val_to_radio(drv_ctx.trim >> 8, drv_ctx.trim & 0xFF))
                   != RADIO_ERROR_NONE) {
            break;
        }

        if ((err = radio_sx126x_set_radio_mode(true, false)) != RADIO_ERROR_NONE) {
            break;
        }

        if ((err = radio_clear_irq_status_all()) != RADIO_ERROR_NONE) {
            break;
        }

        if (sx126x_set_rx_duty_cycle(&drv_ctx, rx_time, sleep_time) != SX126X_STATUS_OK) {
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
        if ((err = set_trim_cap_val_to_radio(drv_ctx.trim >> 8, drv_ctx.trim & 0xFF))
                   != RADIO_ERROR_NONE) {
            break;
        }

        if ((err = radio_sx126x_set_radio_mode(true, false)) != RADIO_ERROR_NONE) {
            break;
        }

        if ((err = radio_clear_irq_status_all()) != RADIO_ERROR_NONE) {
            break;
        }

        if (sx126x_set_cad(&drv_ctx) != SX126X_STATUS_OK) {
            err = RADIO_ERROR_HARDWARE_ERROR;
            break;
        }
        drv_ctx.radio_state = SID_PAL_RADIO_CAD;
     } while(0);

    return err;
}

int16_t sid_pal_radio_rssi(void)
{
    int16_t rssi;
    if (sx126x_get_rssi_inst(&drv_ctx, &rssi) != SX126X_STATUS_OK) {
        return INT16_MAX;
    }

    rssi -= drv_ctx.config->lna_gain;
    return rssi;

}

int32_t sid_pal_radio_is_channel_free(uint32_t freq, int16_t threshold, uint32_t delay_us, bool *is_channel_free)
{
    int32_t err, irq_err;
    *is_channel_free = false;

    if ((err = sid_pal_radio_set_frequency(freq)) != RADIO_ERROR_NONE) {
        goto ret;
    }

    if ((err = radio_disable_irq()) != RADIO_ERROR_NONE) {
        goto ret;
    }

    if ((err = sid_pal_radio_start_continuous_rx() != RADIO_ERROR_NONE)) {
        goto enable_irq;
    }

    struct sid_timespec t_start, t_cur, t_threshold;
    int16_t rssi;

    if (delay_us < SX126X_MIN_CHANNEL_FREE_DELAY_US) {
        delay_us = SX126X_MIN_CHANNEL_FREE_DELAY_US;
    }

    sid_us_to_timespec(delay_us, &t_threshold);

    if (sid_clock_now(SID_CLOCK_SOURCE_UPTIME, &t_start, NULL)
              != SID_ERROR_NONE) {
        err = RADIO_ERROR_GENERIC;
        goto enable_irq;
    }

    do {
        sid_pal_delay_us(SX126X_MIN_CHANNEL_FREE_DELAY_US);
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
    // Do not update err on success of the function calls below
    if ((irq_err = radio_enable_irq()) != RADIO_ERROR_NONE) {
        err = irq_err;
        goto ret;
    }

    if ((irq_err = sid_pal_radio_standby()) != RADIO_ERROR_NONE) {
        err = irq_err;
    }

ret:
    return err;
}

int32_t sid_pal_radio_random(uint32_t *random)
{
    int32_t err, irq_err;
    *random = UINT32_MAX;

    if ((err = radio_disable_irq()) != RADIO_ERROR_NONE) {
       return err;
    }

    if (sx126x_get_random_numbers(&drv_ctx, random, 1) != SX126X_STATUS_OK) {
       err = RADIO_ERROR_HARDWARE_ERROR;
    }

    if ((irq_err = radio_enable_irq()) != RADIO_ERROR_NONE) {
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

    if ((err = sid_pal_radio_set_frequency(freq)) != RADIO_ERROR_NONE) {
        goto ret;
    }

    if ((err = radio_disable_irq()) != RADIO_ERROR_NONE) {
        goto ret;
    }

    if ((err = sid_pal_radio_start_continuous_rx()) != RADIO_ERROR_NONE) {
        goto enable_irq;
    }

    for (uint8_t i = 0; i < SX126X_NOISE_SAMPLE_SIZE; i++) {
        sid_pal_delay_us(SX126X_MIN_CHANNEL_NOISE_DELAY_US);
        *noise += sid_pal_radio_rssi();
    }

    *noise /= SX126X_NOISE_SAMPLE_SIZE;

enable_irq:
    // Do not update err on success of the function calls below
    if ((irq_err = radio_enable_irq()) != RADIO_ERROR_NONE) {
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
    if (drv_ctx.config->tcxo.ctrl == SX126X_TCXO_CTRL_DIO3) {
        state_delay->tcxo_delay_us = SX126X_TUS_TO_US(drv_ctx.config->tcxo.timeout);
    } else if (drv_ctx.config->tcxo.ctrl == SX126X_TCXO_CTRL_VDD) {
        state_delay->tcxo_delay_us = SX126X_TUS_TO_US(SX126X_TCXO_VDD_TIMEOUT_DURATION);
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

        if ((err = radio_sx126x_platform_init()) != RADIO_ERROR_NONE) {
            break;
        }

        if ((err = radio_sx126x_set_radio_mode(false, false)) != RADIO_ERROR_NONE) {
            break;
        }

        if (sx126x_reset(&drv_ctx) != SX126X_STATUS_OK) {
            err = RADIO_ERROR_IO_ERROR;
            break;
        }

        drv_ctx.irq_mask = SX126X_DEFAULT_LORA_IRQ_MASK;
        if (sid_pal_gpio_set_irq(drv_ctx.config->gpio_int1,
            SID_PAL_GPIO_IRQ_TRIGGER_EDGE, radio_irq, NULL) != SID_ERROR_NONE) {
            err = RADIO_ERROR_IO_ERROR;
            break;
        }

        drv_ctx.radio_state = SID_PAL_RADIO_UNKNOWN;
        if ((err = sid_pal_radio_standby()) != RADIO_ERROR_NONE) {
            break;
        }

        err = RADIO_ERROR_HARDWARE_ERROR;
        if (sx126x_set_dio2_as_rf_sw_ctrl(&drv_ctx, true) != SX126X_STATUS_OK) {
            break;
        }

        if (sx126x_set_reg_mode(&drv_ctx, drv_ctx.config->regulator_mode) != SX126X_STATUS_OK) {
            break;
        }

        if (sx126x_set_buffer_base_addr(&drv_ctx, 0x00, 0x00) != SX126X_STATUS_OK) {
            break;
        }

        if (sx126x_cfg_rx_boosted(&drv_ctx, drv_ctx.config->rx_boost) != SX126X_STATUS_OK) {
            break;
        }

        if (drv_ctx.config->tcxo.ctrl != SX126X_TCXO_CTRL_NONE) {
            sx126x_errors_mask_t errors;

            // Follow Semtech comment in https://issues.labcollab.net/browse/MOBILITY-927
            sx126x_clear_device_errors(&drv_ctx);

            if (drv_ctx.config->tcxo.ctrl == SX126X_TCXO_CTRL_DIO3) {
                if (sx126x_set_dio3_as_tcxo_ctrl(&drv_ctx,
                    drv_ctx.config->tcxo.voltage,drv_ctx.config->tcxo.timeout) != SX126X_STATUS_OK) {
                    break;
                }
            } else {
                if (drv_ctx.config->tcxo.dio3_to_mcu_pin != HALO_GPIO_NOT_CONNECTED) {
                    sid_pal_gpio_pull_mode(drv_ctx.config->tcxo.dio3_to_mcu_pin, SID_PAL_GPIO_PULL_NONE);
                    sid_pal_gpio_set_direction(drv_ctx.config->tcxo.dio3_to_mcu_pin, SID_PAL_GPIO_DIRECTION_INPUT);
                }

                if (sx126x_set_dio3_as_tcxo_ctrl(&drv_ctx,
                    SX126X_TCXO_CTRL_1_8V, SX126X_TCXO_VDD_TIMEOUT_DURATION) != SX126X_STATUS_OK) {
                    break;
                }
            }

            if (sx126x_cal(&drv_ctx, SX126X_CAL_ALL) != SX126X_STATUS_OK) {
                break;
            }

            sx126x_get_device_errors(&drv_ctx, &errors);
            sx126x_clear_device_errors(&drv_ctx);
        }

        if (drv_ctx.config->dio3_cfg_callback) {
            if ((err = drv_ctx.config->dio3_cfg_callback()) != RADIO_ERROR_NONE) {
                break;
            }
        }

        if ((err = sid_pal_radio_get_max_tx_power(SID_PAL_RADIO_DATA_RATE_50KBPS, &tx_power)) != RADIO_ERROR_NONE) {
            break;
        }

        if ((err = sid_pal_radio_set_tx_power(tx_power)) != RADIO_ERROR_NONE) {
            break;
        }

        if ((err = radio_enable_irq()) != RADIO_ERROR_NONE) {
            break;
        }

    } while (0);

    return err;
}

int32_t sid_pal_radio_deinit(void)
{
    return RADIO_ERROR_NONE;
}
