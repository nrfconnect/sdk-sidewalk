/*
 * Copyright (c) 2019-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * This file supports radio HAL interface specific to LoRa
 */

#include "sx126x_radio.h"
#include "sx126x_timings.h"

/*
* This function returns the tx processing delay of LoRa.
* (refer to https://issues.labcollab.net/browse/HALO-9632)
* Tx processing delay is defined as the time difference between the time
* when mac schedules to send tx and the first bit of tx.
* The measured value of tx processing delay is around 336 us.(refer to the
* value of "t0" in the JIRA).
*/
#define SX126X_TX_PROCESS_DELAY_US 336

static const uint32_t radio_lora_symb_time[3][8] = {{32768, 16384, 8192, 4096, 2048, 1024, 512, 256}, // 125 KHz
                                              {16384, 8192,  4096, 2048, 1024, 512,  256, 128},  // 250 KHz
                                              {8192,  4096,  2048, 1024, 512,  256,  128, 64}};  // 500 KHz

// Radio Rx process time = 286 us
// HALO-9631: Compensate the rx process time
#define SX126X_RX_PROCESS_DELAY_US 286

static bool lora_low_data_rate_optimize(sx126x_lora_sf_t sf, sx126x_lora_bw_t bw)
{
    if (((bw == SX126X_LORA_BW_125) && ((sf == SX126X_LORA_SF11) || (sf == SX126X_LORA_SF12)))
        || ((bw == SX126X_LORA_BW_250) && (sf == SX126X_LORA_SF12))) {
        return true;
    }
    return false;
}

static void radio_to_sx126x_lora_modulation_params( sx126x_mod_params_lora_t *sx_m, const sid_pal_radio_lora_modulation_params_t *rd_m)
{
    sx_m->sf   = (sx126x_lora_sf_t)rd_m->spreading_factor;
    sx_m->bw   = (sx126x_lora_bw_t)rd_m->bandwidth;
    sx_m->cr   = (sx126x_lora_cr_t)rd_m->coding_rate;
    sx_m->ldro = lora_low_data_rate_optimize(rd_m->spreading_factor, rd_m->bandwidth);
}

static void radio_to_sx126x_lora_packet_params(sx126x_pkt_params_lora_t *sx_p, const sid_pal_radio_lora_packet_params_t *rd_p)
{
    sx_p->pbl_len_in_symb = rd_p->preamble_length;
    sx_p->hdr_type =  (sx126x_lora_pkt_len_modes_t)rd_p->header_type;
    sx_p->pld_len_in_bytes = rd_p->payload_length;
    sx_p->crc_is_on = rd_p->crc_mode;
    sx_p->invert_iq_is_on = rd_p->invert_IQ;
}

static int32_t get_payload(halo_drv_semtech_ctx_t *drv_ctx, uint8_t *buffer, uint8_t *size, uint8_t max_size)
{
    int32_t err;
    sx126x_rx_buffer_status_t rx_buffer_status = {0};

    if (sx126x_get_rx_buffer_status(drv_ctx, &rx_buffer_status) != SX126X_STATUS_OK) {
        err = RADIO_ERROR_IO_ERROR;
    } else if (rx_buffer_status.pld_len_in_bytes > max_size) {
        err = RADIO_ERROR_NOMEM;
    } else if (sx126x_read_buffer(drv_ctx, rx_buffer_status.buffer_start_pointer, buffer,
                                  rx_buffer_status.pld_len_in_bytes) != SX126X_STATUS_OK) {
        err = RADIO_ERROR_IO_ERROR;
    } else {
        *size = rx_buffer_status.pld_len_in_bytes;
        err = RADIO_ERROR_NONE;
    }
    return err;
}

static int32_t get_lora_crc_present_in_header(halo_drv_semtech_ctx_t *drv_ctx, sid_pal_radio_lora_crc_present_t *is_crc_present)
{
    sx126x_lora_cr_t cr;
    bool is_crc_on;
    if (sx126x_get_lora_params_from_header(drv_ctx, &cr, &is_crc_on) != SX126X_STATUS_OK) {
        *is_crc_present = SID_PAL_RADIO_CRC_PRESENT_INVALID;
        return RADIO_ERROR_GENERIC;
    }
    if (is_crc_on == true) {
        *is_crc_present = SID_PAL_RADIO_CRC_PRESENT_ON;
    } else {
        *is_crc_present = SID_PAL_RADIO_CRC_PRESENT_OFF;
    }
    return RADIO_ERROR_NONE;
}

int32_t radio_lora_process_rx_done(halo_drv_semtech_ctx_t *drv_ctx)
{
    int32_t err;
    sid_pal_radio_rx_packet_t *radio_rx_packet = drv_ctx->radio_rx_packet;

    do {

        if ((err = get_payload(drv_ctx, radio_rx_packet->rcv_payload, &radio_rx_packet->payload_len,
                             SID_PAL_RADIO_RX_PAYLOAD_MAX_SIZE)) != RADIO_ERROR_NONE) {
            break;
        }

        sx126x_pkt_status_lora_t lora_pkt_status = {0};
        if (sx126x_get_lora_pkt_status(drv_ctx, &lora_pkt_status) != SX126X_STATUS_OK) {
            err = RADIO_ERROR_IO_ERROR;
            break;
        }

        sid_pal_radio_lora_rx_packet_status_t *lora_rx_packet_status = &radio_rx_packet->lora_rx_packet_status;
        lora_rx_packet_status->rssi = lora_pkt_status.rssi_pkt_in_dbm - drv_ctx->config->lna_gain;
        lora_rx_packet_status->snr = lora_pkt_status.snr_pkt_in_db;
        lora_rx_packet_status->signal_rssi = lora_pkt_status.signal_rssi_pkt_in_dbm - drv_ctx->config->lna_gain;
        /*
         * NOTE: By default sx126x radio chip's rssi saturates at -106 dBm (or some cases -107 dBm)
         * After that the signal deterioration is reflected on the snr change.
         * to make the rssi carry more significance to the upper layer consumers, we need to figure out
         * a derived rssi value combining the rssi and snr in a meaningful way.
         *
         * Based on the empirical data, the formula is: <adjusted rssi = raw rssi + snr (when snr is negative)>.
         * This adjusted value is reported and used in all the ring products.
         */
         if(lora_rx_packet_status->snr < 0) {
             lora_rx_packet_status->rssi += lora_rx_packet_status->snr;
         }

        if ((err = get_lora_crc_present_in_header(drv_ctx, &lora_rx_packet_status->is_crc_present)) != RADIO_ERROR_NONE) {
            break;
        }

    } while(0);

    return err;
}

sid_pal_radio_data_rate_t
sid_pal_radio_lora_mod_params_to_data_rate(const sid_pal_radio_lora_modulation_params_t *mod_params)
{
    if (SID_PAL_RADIO_LORA_SF7 == mod_params->spreading_factor
        && SID_PAL_RADIO_LORA_BW_500KHZ == mod_params->bandwidth
        && SID_PAL_RADIO_LORA_CODING_RATE_4_6 == mod_params->coding_rate) {
        return SID_PAL_RADIO_DATA_RATE_22KBPS;
    }
    if (SID_PAL_RADIO_LORA_SF8 == mod_params->spreading_factor
        && SID_PAL_RADIO_LORA_BW_500KHZ == mod_params->bandwidth
        && SID_PAL_RADIO_LORA_CODING_RATE_4_5_LI == mod_params->coding_rate) {
        return SID_PAL_RADIO_DATA_RATE_12_5KBPS;
    }
    if (SID_PAL_RADIO_LORA_SF11 == mod_params->spreading_factor
        && SID_PAL_RADIO_LORA_BW_500KHZ == mod_params->bandwidth
        && ((SID_PAL_RADIO_LORA_CODING_RATE_4_5 == mod_params->coding_rate) || (SID_PAL_RADIO_LORA_CODING_RATE_4_5_LI == mod_params->coding_rate))) {
        return SID_PAL_RADIO_DATA_RATE_2KBPS;
    }

    return SID_PAL_RADIO_DATA_RATE_INVALID;
}

int32_t sid_pal_radio_lora_data_rate_to_mod_params(sid_pal_radio_lora_modulation_params_t *mod_params,
                                           sid_pal_radio_data_rate_t data_rate, uint8_t li_enable)
{
    switch (data_rate) {
    case SID_PAL_RADIO_DATA_RATE_22KBPS:
        mod_params->spreading_factor = SID_PAL_RADIO_LORA_SF7;
        mod_params->bandwidth = SID_PAL_RADIO_LORA_BW_500KHZ;
        mod_params->coding_rate = SID_PAL_RADIO_LORA_CODING_RATE_4_6;
        break;
    case SID_PAL_RADIO_DATA_RATE_12_5KBPS:
        mod_params->spreading_factor = SID_PAL_RADIO_LORA_SF8;
        mod_params->bandwidth = SID_PAL_RADIO_LORA_BW_500KHZ;
        mod_params->coding_rate = SID_PAL_RADIO_LORA_CODING_RATE_4_5_LI;
        break;
    case SID_PAL_RADIO_DATA_RATE_2KBPS:
        mod_params->spreading_factor = SID_PAL_RADIO_LORA_SF11;
        mod_params->bandwidth = SID_PAL_RADIO_LORA_BW_500KHZ;
        if(li_enable) {
            mod_params->coding_rate = SID_PAL_RADIO_LORA_CODING_RATE_4_5_LI;
        } else {
            mod_params->coding_rate = SID_PAL_RADIO_LORA_CODING_RATE_4_5;
        }
        break;
    default:
        return RADIO_ERROR_NOT_SUPPORTED;
    }

    return RADIO_ERROR_NONE;
}

int32_t sid_pal_radio_set_lora_sync_word(uint16_t sync_word)
{
    uint8_t syncword;

    if (sync_word == 0x1424) {
        syncword = 0x12;
    } else {
        syncword = 0x34;
    }

    if (sx126x_set_lora_sync_word(sx126x_get_drv_ctx(), syncword) != SX126X_STATUS_OK) {
        return RADIO_ERROR_HARDWARE_ERROR;
    }

    return RADIO_ERROR_NONE;
}

int32_t sid_pal_radio_set_lora_symbol_timeout(uint8_t num_of_symbols)
{
    if (sx126x_set_lora_symb_nb_timeout(sx126x_get_drv_ctx(),
          num_of_symbols) != SX126X_STATUS_OK) {
        return RADIO_ERROR_HARDWARE_ERROR;
    }

    return RADIO_ERROR_NONE;
}

int32_t sid_pal_radio_set_lora_modulation_params(const sid_pal_radio_lora_modulation_params_t *mod_params)
{
    sx126x_mod_params_lora_t lora_mod_params;

    radio_to_sx126x_lora_modulation_params(&lora_mod_params, mod_params);

    if (sx126x_set_lora_mod_params(sx126x_get_drv_ctx(), &lora_mod_params) != SX126X_STATUS_OK) {
        return RADIO_ERROR_HARDWARE_ERROR;
    }

    return RADIO_ERROR_NONE;
}

int32_t sid_pal_radio_set_lora_packet_params(const sid_pal_radio_lora_packet_params_t *packet_params)
{
    sx126x_pkt_params_lora_t lora_packet_params;

    radio_to_sx126x_lora_packet_params(&lora_packet_params, packet_params);

    if (sx126x_set_lora_pkt_params(sx126x_get_drv_ctx(), &lora_packet_params) != SX126X_STATUS_OK) {
        return RADIO_ERROR_HARDWARE_ERROR;
    }

    /* Work around as per data sheet Section 15.4.2*/
    uint8_t iq_reg;

    if (sx126x_read_register(sx126x_get_drv_ctx(), SX126X_REG_IRQ_POLARITY,
          &iq_reg, 1) != SX126X_STATUS_OK) {
        return RADIO_ERROR_HARDWARE_ERROR;
    }

    if (lora_packet_params.invert_iq_is_on) {
        iq_reg &= ~(1 << 2); // Bit 2 set to 0 when using inverted IQ polarity
    } else {
        iq_reg |= (1 << 2);  // Bit 2 set to 1 when using standard IQ polarity
    }

    if (sx126x_write_register(sx126x_get_drv_ctx(), SX126X_REG_IRQ_POLARITY,
          &iq_reg, 1) != SX126X_STATUS_OK) {
        return RADIO_ERROR_HARDWARE_ERROR;
    }

    return RADIO_ERROR_NONE;
}

int32_t sid_pal_radio_set_lora_cad_params(const sid_pal_radio_lora_cad_params_t *cad_params)
{
    sx126x_lora_cad_params_t lora_cad_params;

    lora_cad_params.cad_symb_nb = (sx126x_lora_cad_symbs_t)cad_params->cad_symbol_num;
    lora_cad_params.cad_det_peak = cad_params->cad_detect_peak;
    lora_cad_params.cad_det_min = cad_params->cad_detect_min;

    switch (cad_params->cad_exit_mode) {
        case SID_PAL_RADIO_CAD_EXIT_MODE_CS_ONLY:
            lora_cad_params.cad_exit_mode = SX126X_LORA_CAD_ONLY;
            break;
        case SID_PAL_RADIO_CAD_EXIT_MODE_CS_RX:
            lora_cad_params.cad_exit_mode = SX126X_LORA_CAD_RX;
            break;
        case SID_PAL_RADIO_CAD_EXIT_MODE_CS_LBT:
            lora_cad_params.cad_exit_mode = SX126X_LORA_CAD_LBT;
            break;
        default:
            return RADIO_ERROR_INVALID_PARAMS;
    }

    set_lora_exit_mode(cad_params->cad_exit_mode);
    lora_cad_params.cad_timeout = cad_params->cad_timeout;

    if (sx126x_set_cad_params(sx126x_get_drv_ctx(), &lora_cad_params) != SX126X_STATUS_OK) {
        return RADIO_ERROR_HARDWARE_ERROR;
    }

    return RADIO_ERROR_NONE;
}

uint32_t sid_pal_radio_lora_cad_duration(uint8_t symbol, const sid_pal_radio_lora_modulation_params_t *mod_params)
{
    sx126x_mod_params_lora_t lora_mod_params;
    radio_to_sx126x_lora_modulation_params(&lora_mod_params, mod_params);
    return sx126x_get_lora_cad_duration_microsecs(symbol, &lora_mod_params);
}

uint32_t sid_pal_radio_lora_time_on_air(const sid_pal_radio_lora_modulation_params_t *mod_params,
                                const sid_pal_radio_lora_packet_params_t *packet_params, uint8_t packet_len)
{
    sx126x_pkt_params_lora_t lora_packet_params;
    sx126x_mod_params_lora_t lora_mod_params;

    radio_to_sx126x_lora_modulation_params(&lora_mod_params, mod_params);
    radio_to_sx126x_lora_packet_params(&lora_packet_params, packet_params);
    lora_packet_params.pld_len_in_bytes = packet_len;

    return sx126x_get_lora_time_on_air_in_ms(&lora_packet_params, &lora_mod_params);
}

/**
 * @brief Calculates the minimum number of symbols that takes more than delay_micro_sec of air time.
 *
 * In the case of a fractional number of symbols, the return value is rounded up to the next integer.
 * Does not affect the radio state and can be executed without radio ownership.
 * In the case of an error, 0 is returned.
 *
 *  @param[in] mod_params Current modulation parameters that phy uses. If null, zero will be returned.
 *  @param[in] delay_micro_sec Input amount of time that will be translated to number of symbols.
 *  @return number of symbols
 *
 */
uint32_t sid_pal_radio_lora_get_lora_number_of_symbols(const sid_pal_radio_lora_modulation_params_t *mod_params,
                                               uint32_t delay_micro_sec)
{
    uint32_t numsymbols = 0;
    if (mod_params) {
        uint32_t ts = radio_lora_symb_time[mod_params->bandwidth - 4][12 - mod_params->spreading_factor];
        numsymbols = delay_micro_sec / ts;
        if (delay_micro_sec > ts * numsymbols) {
            ++numsymbols;
        }
    }
    return numsymbols;
}

uint32_t sid_pal_radio_get_lora_rx_done_delay(const sid_pal_radio_lora_modulation_params_t* mod_params,
                                            const sid_pal_radio_lora_packet_params_t* pkt_params)
{
    sx126x_pkt_params_lora_t lora_packet_params;
    sx126x_mod_params_lora_t lora_mod_params;

    radio_to_sx126x_lora_modulation_params(&lora_mod_params, mod_params);
    radio_to_sx126x_lora_packet_params(&lora_packet_params, pkt_params);
    return sx126x_timings_get_delay_between_last_bit_sent_and_rx_done_in_us(&lora_mod_params, &lora_packet_params);
}

uint32_t sid_pal_radio_get_lora_tx_process_delay(void)
{
    return SX126X_TX_PROCESS_DELAY_US;
}

uint32_t sid_pal_radio_get_lora_rx_process_delay(void)
{
    return SX126X_RX_PROCESS_DELAY_US;
}

/**
 * @brief Calculates the lora symbol timeout in us.
 *
 * In the case of an error, 0 is returned.
 *
 *  @param[in] mod_params Current modulation parameters that phy uses. If null, zero will be returned.
 *  @param[in] number_of_symbol Input number of symbol .
 *  @return lora symbol timeout in us
 *
 */
uint32_t sid_pal_radio_get_lora_symbol_timeout_us(sid_pal_radio_lora_modulation_params_t *mod_params, uint8_t number_of_symbol)
{
    if (mod_params) {
        return radio_lora_symb_time[mod_params->bandwidth - 4][12 - mod_params->spreading_factor] * number_of_symbol;
    }
    return 0;
}
