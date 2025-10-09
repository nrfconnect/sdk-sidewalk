/*
 * Copyright (c) 2019-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * This file supports radio hal interface  specific to FSK
 * Some parts of the the code are based on Semtech reference driver
 */
#include <assert.h>

#include "halo_lr1110_radio.h"
#include "lr1110_radio.h"
#include "lr1110_regmem.h"
#include "lr1110_halo.h"

#define FSK_MICRO_SECS_PER_SYMBOL               250

#define RADIO_FSK_SYNC_WORD_VALID_MARKER        0xABBA
#define RADIO_FSK_PACKET_TYPE_OFFSET            0
#define RADIO_FSK_PACKET_LENGTH_OFFSET          1
#define RADIO_FSK_SYNC_WORD_VALID_MARKER_OFFSET 2

#define RADIO_FSK_BR_50KBPS                     50000
#define RADIO_FSK_BR_150KBPS                    150000
#define RADIO_FSK_BR_250KBPS                    250000

#define RADIO_FSK_FDEV_19KHZ                    19000
#define RADIO_FSK_FDEV_25KHZ                    25000
#define RADIO_FSK_FDEV_37_5KHZ                  37500
#define RADIO_FSK_FDEV_62_5KHZ                  62500

#define LR1110_FSK_PHY_HEADER_LENGTH            2
#define LR1110_FSK_WHITENING_SEED               0x01FF
#define LR1110_FSK_MAX_PAYLOAD_LENGTH           255
#define LR1110_FSK_SYNC_WORD_LENGTH_IN_RX       3
#define MAX_PAYLOAD_LENGTH_WITH_FCS_TYPE_0      251
#define MAX_PAYLOAD_LENGTH_WITH_FCS_TYPE_1      253
#define LR1110_FSK_PROCESS_DELAY_US_MIN 1000
#define LR1110_FSK_PROCESS_DELAY_US_MAX 5000
#define LR1110_FSK_TX_PROCESS_DELAY_US 1000
#define LR1110_FSK_RX_PROCESS_DELAY_US 1000

static void radio_mp_to_lr1110_mp(lr1110_radio_mod_params_gfsk_t *fsk_mp, const sid_pal_radio_fsk_modulation_params_t *mod_params)
{
    fsk_mp->br_in_bps    = mod_params->bit_rate;
    fsk_mp->fdev_in_hz   = mod_params->freq_dev;

    _Static_assert(LR1110_RADIO_GFSK_BW_93800  == SID_PAL_RADIO_FSK_BW_93800  , "Invalid convertion");
    _Static_assert(LR1110_RADIO_GFSK_BW_117300 == SID_PAL_RADIO_FSK_BW_117300 , "Invalid convertion");
    _Static_assert(LR1110_RADIO_GFSK_BW_156200 == SID_PAL_RADIO_FSK_BW_156200 , "Invalid convertion");
    _Static_assert(LR1110_RADIO_GFSK_BW_312000 == SID_PAL_RADIO_FSK_BW_312000 , "Invalid convertion");
    _Static_assert(LR1110_RADIO_GFSK_BW_467000 == SID_PAL_RADIO_FSK_BW_467000 , "Invalid convertion");
    fsk_mp->bw_dsb_param = (lr1110_radio_gfsk_bw_t)mod_params->bandwidth;

    _Static_assert(SID_PAL_RADIO_FSK_MOD_SHAPING_OFF     == LR1110_RADIO_GFSK_PULSE_SHAPE_OFF  , "Invalid convertion");
    _Static_assert(SID_PAL_RADIO_FSK_MOD_SHAPING_G_BT_03 == LR1110_RADIO_GFSK_PULSE_SHAPE_BT_03, "Invalid convertion");
    _Static_assert(SID_PAL_RADIO_FSK_MOD_SHAPING_G_BT_05 == LR1110_RADIO_GFSK_PULSE_SHAPE_BT_05, "Invalid convertion");
    _Static_assert(SID_PAL_RADIO_FSK_MOD_SHAPING_G_BT_07 == LR1110_RADIO_GFSK_PULSE_SHAPE_BT_07, "Invalid convertion");
    _Static_assert(SID_PAL_RADIO_FSK_MOD_SHAPING_G_BT_1  == LR1110_RADIO_GFSK_PULSE_SHAPE_BT_1 , "Invalid convertion");
    fsk_mp->pulse_shape  = (lr1110_radio_gfsk_pulse_shape_t)mod_params->mod_shaping;
}

static void radio_pp_to_lr1110_pp(lr1110_radio_pkt_params_gfsk_t *fsk_pp, const sid_pal_radio_fsk_packet_params_t *packet_params)
{
    if(packet_params->preamble_length > 1) {
        fsk_pp->preamble_len_in_bits       = (packet_params->preamble_length - 1) << 3;
    } else {
        fsk_pp->preamble_len_in_bits       = 0;
    }

    _Static_assert(LR1110_RADIO_GFSK_PREAMBLE_DETECTOR_OFF        == SID_PAL_RADIO_FSK_PREAMBLE_DETECTOR_OFF, "Invalid convertion");
    _Static_assert(LR1110_RADIO_GFSK_PREAMBLE_DETECTOR_MIN_8BITS  == SID_PAL_RADIO_FSK_PREAMBLE_DETECTOR_08_BITS, "Invalid convertion");
    _Static_assert(LR1110_RADIO_GFSK_PREAMBLE_DETECTOR_MIN_16BITS == SID_PAL_RADIO_FSK_PREAMBLE_DETECTOR_16_BITS, "Invalid convertion");
    _Static_assert(LR1110_RADIO_GFSK_PREAMBLE_DETECTOR_MIN_24BITS == SID_PAL_RADIO_FSK_PREAMBLE_DETECTOR_24_BITS, "Invalid convertion");
    _Static_assert(LR1110_RADIO_GFSK_PREAMBLE_DETECTOR_MIN_32BITS == SID_PAL_RADIO_FSK_PREAMBLE_DETECTOR_32_BITS, "Invalid convertion");
    fsk_pp->preamble_detector     = (lr1110_radio_gfsk_preamble_detector_t)packet_params->preamble_min_detect;
    fsk_pp->sync_word_len_in_bits = packet_params->sync_word_length << 3;

    _Static_assert(LR1110_RADIO_GFSK_ADDRESS_FILTERING_DISABLE      == SID_PAL_RADIO_FSK_ADDRESSCOMP_FILT_OFF, "Invalid convertion");
    _Static_assert(LR1110_RADIO_GFSK_ADDRESS_FILTERING_NODE_ADDRESS == SID_PAL_RADIO_FSK_ADDRESSCOMP_FILT_NODE, "Invalid convertion");
    _Static_assert(LR1110_RADIO_GFSK_ADDRESS_FILTERING_NODE_AND_BROADCAST_ADDRESSES == SID_PAL_RADIO_FSK_ADDRESSCOMP_FILT_NODE_BROAD, "Invalid convertion");
    fsk_pp->address_filtering     = (lr1110_radio_gfsk_address_filtering_t)packet_params->addr_comp;

    _Static_assert(LR1110_RADIO_GFSK_PKT_FIX_LEN == SID_PAL_RADIO_FSK_RADIO_PACKET_FIXED_LENGTH, "Invalid convertion");
    _Static_assert(LR1110_RADIO_GFSK_PKT_VAR_LEN == SID_PAL_RADIO_FSK_RADIO_PACKET_VARIABLE_LENGTH, "Invalid convertion");
    fsk_pp->header_type           = (lr1110_radio_gfsk_pkt_len_modes_t)packet_params->header_type;
    fsk_pp->pld_len_in_bytes      = packet_params->payload_length;

    _Static_assert(LR1110_RADIO_GFSK_CRC_OFF         == SID_PAL_RADIO_FSK_CRC_OFF,         "Invalid convertion");
    _Static_assert(LR1110_RADIO_GFSK_CRC_1_BYTE      == SID_PAL_RADIO_FSK_CRC_1_BYTES,     "Invalid convertion");
    _Static_assert(LR1110_RADIO_GFSK_CRC_2_BYTES     == SID_PAL_RADIO_FSK_CRC_2_BYTES,     "Invalid convertion");
    _Static_assert(LR1110_RADIO_GFSK_CRC_1_BYTE_INV  == SID_PAL_RADIO_FSK_CRC_1_BYTES_INV, "Invalid convertion");
    _Static_assert(LR1110_RADIO_GFSK_CRC_2_BYTES_INV == SID_PAL_RADIO_FSK_CRC_2_BYTES_INV, "Invalid convertion");
    assert(packet_params->crc_type != SID_PAL_RADIO_FSK_CRC_2_BYTES_IBM);  // Not supported
    assert(packet_params->crc_type != SID_PAL_RADIO_FSK_CRC_2_BYTES_CCIT); // Not supported
    fsk_pp->crc_type              = (lr1110_radio_gfsk_crc_type_t)packet_params->crc_type;

    _Static_assert(LR1110_RADIO_GFSK_DC_FREE_OFF       == SID_PAL_RADIO_FSK_DC_FREE_OFF,      "Invalid convertion");
    _Static_assert(LR1110_RADIO_GFSK_DC_FREE_WHITENING == SID_PAL_RADIO_FSK_DC_FREEWHITENING, "Invalid convertion");
    fsk_pp->dc_free               = (lr1110_radio_gfsk_dc_free_t)packet_params->radio_whitening_mode;
}

int32_t radio_fsk_process_sync_word_detected(halo_drv_semtech_ctx_t *drv_ctx)
{
    int32_t err =  RADIO_ERROR_NONE;
    sid_pal_radio_rx_packet_t *radio_rx_packet = drv_ctx->radio_rx_packet;
    uint8_t *buffer = radio_rx_packet->rcv_payload;

    do {
        uint32_t rx_buffer_ptr_position = 0;

        /* Below code is taken from Semtech Halo driver. It's purpose to handle non standrat GFSK
           RingNet (Sidewalk) Radio Packet. Don't ask me about those magic values - I don't know
           what they are */
        while( rx_buffer_ptr_position <= LR1110_FSK_PHY_HEADER_LENGTH + 2 )
        {
            lr1110_regmem_read_regmem32( drv_ctx, 0x00F20384, &rx_buffer_ptr_position, 1 );
            rx_buffer_ptr_position &= 0x0FFF0000;
            rx_buffer_ptr_position = rx_buffer_ptr_position >> 16;
        }

        lr1110_regmem_read_buffer8( drv_ctx, buffer, 0x00, LR1110_FSK_PHY_HEADER_LENGTH + 2 );

        rx_buffer_ptr_position = buffer[1] + LR1110_FSK_PHY_HEADER_LENGTH;
        lr1110_regmem_write_regmem32_mask( drv_ctx, 0x00F20368, 0xFFF00000, rx_buffer_ptr_position << 20 );

        buffer[RADIO_FSK_SYNC_WORD_VALID_MARKER_OFFSET] = (RADIO_FSK_SYNC_WORD_VALID_MARKER >> 8 & 0xFF);
        buffer[RADIO_FSK_SYNC_WORD_VALID_MARKER_OFFSET + 1] = (RADIO_FSK_SYNC_WORD_VALID_MARKER & 0xFF);

    } while(0);

    return err;
}

int32_t radio_fsk_process_rx_done(halo_drv_semtech_ctx_t *drv_ctx, radio_fsk_rx_done_status_t *rx_done_status)
{
    sid_pal_radio_rx_packet_t          *radio_rx_packet      = drv_ctx->radio_rx_packet;
    sid_pal_radio_fsk_rx_packet_status_t     *fsk_rx_packet_status = &radio_rx_packet->fsk_rx_packet_status;
    sid_pal_radio_fsk_phy_hdr_t             phy_hdr;
    lr1110_radio_pkt_status_gfsk_t  pkt_status            = {0};
    int32_t                         err                   = RADIO_ERROR_NONE;
    uint8_t                         *buffer               = radio_rx_packet->rcv_payload;
    uint8_t                         length_temp           = 0;
    uint32_t                        crc                   = 0x00000000;
    uint8_t                         crc_length            = 0;
    lr1110_radio_rx_buffer_status_t rx_buffer_status;

    do {
        if ((buffer[RADIO_FSK_SYNC_WORD_VALID_MARKER_OFFSET] << 8 |
             buffer[RADIO_FSK_SYNC_WORD_VALID_MARKER_OFFSET + 1] ) !=
            RADIO_FSK_SYNC_WORD_VALID_MARKER) {
            *rx_done_status = RADIO_FSK_RX_DONE_STATUS_SW_MARK_NOT_PRESENT;
            err = RADIO_ERROR_GENERIC;
            break;
        }

        buffer[RADIO_FSK_SYNC_WORD_VALID_MARKER_OFFSET] = 0x00;
        buffer[RADIO_FSK_SYNC_WORD_VALID_MARKER_OFFSET + 1] = 0x00;

        phy_hdr.fcs_type = buffer[RADIO_FSK_PACKET_TYPE_OFFSET] >> 4;
        phy_hdr.is_data_whitening_enabled =
          (buffer[RADIO_FSK_PACKET_TYPE_OFFSET] & (1 << 3)) ? true : false;
        length_temp = buffer[RADIO_FSK_PACKET_LENGTH_OFFSET];

        if ((phy_hdr.fcs_type == RADIO_FSK_FCS_TYPE_0 && length_temp <= sizeof(uint32_t)) ||
            (phy_hdr.fcs_type == RADIO_FSK_FCS_TYPE_1 && length_temp <= sizeof(uint16_t))) {
            *rx_done_status =  RADIO_FSK_RX_DONE_STATUS_BAD_CRC;
            err = RADIO_ERROR_IO_ERROR;
            break;
        }

        if (lr1110_radio_get_rx_buffer_status(drv_ctx, &rx_buffer_status) != LR1110_STATUS_OK) {
            err = RADIO_ERROR_IO_ERROR;
            break;
        }

        // coverity[tainted_data] length_temp is a uint8 so will always be <=
        // LR1110_FSK_MAX_PAYLOAD_LENGTH
        if (lr1110_regmem_read_buffer8(drv_ctx, buffer, rx_buffer_status.buffer_start_pointer + LR1110_FSK_PHY_HEADER_LENGTH,
              length_temp) != LR1110_STATUS_OK) {
            err = RADIO_ERROR_IO_ERROR;
            *rx_done_status = RADIO_FSK_RX_DONE_STATUS_TIMEOUT;
            break;
        }

        if (phy_hdr.is_data_whitening_enabled) {
            perform_data_whitening(LR1110_FSK_WHITENING_SEED, buffer, buffer, length_temp);
        }

        switch( phy_hdr.fcs_type ) {
            case RADIO_FSK_FCS_TYPE_0:
                crc_length = sizeof(uint32_t);
                crc        = compute_crc32(buffer, length_temp - crc_length);
                break;
            case RADIO_FSK_FCS_TYPE_1:
                crc_length = sizeof(uint16_t);
                crc        = compute_crc16(buffer, length_temp - crc_length);
                break;
            default:
                radio_rx_packet->payload_len = 0;
                err = RADIO_ERROR_GENERIC;
                *rx_done_status = RADIO_FSK_RX_DONE_STATUS_UNKNOWN_ERROR;
        }

        if (*rx_done_status != RADIO_FSK_RX_DONE_STATUS_UNKNOWN_ERROR) {
            radio_rx_packet->payload_len = length_temp - crc_length;
            for( uint8_t i = 0; i < crc_length; i++) {
                if( buffer[radio_rx_packet->payload_len + i] != (uint8_t)(crc >> (8*(crc_length - i - 1 )))) {
                    *rx_done_status =  RADIO_FSK_RX_DONE_STATUS_BAD_CRC;
                    err = RADIO_ERROR_GENERIC;
                    break;
                }
            }
        } else {
            break;
        }

        if (lr1110_radio_get_gfsk_pkt_status(drv_ctx, &pkt_status) != LR1110_STATUS_OK) {
            err = RADIO_ERROR_IO_ERROR;
            *rx_done_status = RADIO_FSK_RX_DONE_STATUS_TIMEOUT;
            break;
        }

        memset(fsk_rx_packet_status, 0, sizeof(*fsk_rx_packet_status));
        fsk_rx_packet_status->rssi_sync = pkt_status.rssi_sync_in_dbm - drv_ctx->config->lna_gain;
        fsk_rx_packet_status->rssi_avg  = pkt_status.rssi_avg_in_dbm - drv_ctx->config->lna_gain;

    } while(0);


    return err;
}

sid_pal_radio_data_rate_t sid_pal_radio_fsk_mod_params_to_data_rate(const sid_pal_radio_fsk_modulation_params_t *mp)
{
    uint8_t data_rate = SID_PAL_RADIO_DATA_RATE_INVALID;

    /* The modulation params are not yet finalized and is tracked in JIRA-5059.
     * These changes here will unblock the testing for GEN2 lights.
     */
    if (mp->bit_rate == RADIO_FSK_BR_50KBPS && mp->freq_dev == RADIO_FSK_FDEV_25KHZ &&
        mp->bandwidth == (uint8_t)LR1110_RADIO_GFSK_BW_156200) {
        data_rate =  SID_PAL_RADIO_DATA_RATE_50KBPS;
    } else if (mp->bit_rate == RADIO_FSK_BR_150KBPS && mp->freq_dev == RADIO_FSK_FDEV_37_5KHZ &&
               mp->bandwidth == (uint8_t)LR1110_RADIO_GFSK_BW_312000) {
        data_rate = SID_PAL_RADIO_DATA_RATE_150KBPS;
    } else if (mp->bit_rate == RADIO_FSK_BR_250KBPS && mp->freq_dev == RADIO_FSK_FDEV_62_5KHZ &&
               mp->bandwidth == (uint8_t)LR1110_RADIO_GFSK_BW_467000) {
        data_rate = SID_PAL_RADIO_DATA_RATE_250KBPS;
    }

    return data_rate;
}

int32_t sid_pal_radio_fsk_data_rate_to_mod_params(sid_pal_radio_fsk_modulation_params_t *mod_params,
                                          sid_pal_radio_data_rate_t data_rate)
{
    if (mod_params == NULL) {
       return RADIO_ERROR_INVALID_PARAMS;
    }

    switch (data_rate) {
        case SID_PAL_RADIO_DATA_RATE_50KBPS:
            mod_params->bit_rate     = RADIO_FSK_BR_50KBPS;
            mod_params->freq_dev     = RADIO_FSK_FDEV_25KHZ;
            mod_params->bandwidth    = (uint8_t)LR1110_RADIO_GFSK_BW_156200;
            mod_params->mod_shaping  = (uint8_t)LR1110_RADIO_GFSK_PULSE_SHAPE_BT_1;
            break;
        case SID_PAL_RADIO_DATA_RATE_150KBPS:
            mod_params->bit_rate     = RADIO_FSK_BR_150KBPS;
            mod_params->freq_dev     = RADIO_FSK_FDEV_37_5KHZ;
            mod_params->bandwidth    = (uint8_t)LR1110_RADIO_GFSK_BW_312000;
            mod_params->mod_shaping  = (uint8_t)LR1110_RADIO_GFSK_PULSE_SHAPE_BT_05;
            break;
        case SID_PAL_RADIO_DATA_RATE_250KBPS:
            mod_params->bit_rate     = RADIO_FSK_BR_250KBPS;
            mod_params->freq_dev     = RADIO_FSK_FDEV_62_5KHZ;
            mod_params->bandwidth    = (uint8_t)LR1110_RADIO_GFSK_BW_467000;
            mod_params->mod_shaping  = (uint8_t)LR1110_RADIO_GFSK_PULSE_SHAPE_BT_05;
            break;
        default:
            return RADIO_ERROR_INVALID_PARAMS;
    }

    return RADIO_ERROR_NONE;
}

int32_t sid_pal_radio_prepare_fsk_for_rx(sid_pal_radio_fsk_pkt_cfg_t *rx_pkt_cfg)
{
    int32_t err = RADIO_ERROR_INVALID_PARAMS;

    do {
        if (rx_pkt_cfg == NULL) {
            break;
        }

        if (rx_pkt_cfg->phy_hdr == NULL || rx_pkt_cfg->packet_params == NULL ||
            rx_pkt_cfg->sync_word == NULL) {
            break;
        }

        sid_pal_radio_fsk_packet_params_t *f_pp  =  rx_pkt_cfg->packet_params;
        sid_pal_radio_fsk_phy_hdr_t      *phr   =  rx_pkt_cfg->phy_hdr;
        uint8_t                  *sw    =  rx_pkt_cfg->sync_word;

        f_pp->preamble_min_detect       = (uint8_t)LR1110_RADIO_GFSK_PREAMBLE_DETECTOR_MIN_16BITS;
        f_pp->sync_word_length          = LR1110_FSK_SYNC_WORD_LENGTH_IN_RX;
        f_pp->addr_comp                 = (uint8_t)LR1110_RADIO_GFSK_ADDRESS_FILTERING_DISABLE;
        f_pp->header_type               = (uint8_t)LR1110_RADIO_GFSK_PKT_FIX_LEN;
        f_pp->payload_length            = LR1110_FSK_MAX_PAYLOAD_LENGTH;
        f_pp->crc_type                  = (uint8_t)LR1110_RADIO_GFSK_CRC_OFF;
        f_pp->radio_whitening_mode      = (uint8_t)LR1110_RADIO_GFSK_DC_FREE_OFF;

        /* For C41s lights rx needs 3 bytes sync word.
         * The sync word length must be greater than preamble min detect.
         * As preamble min detect is 2 bytes set sync word length to 3 bytes.
         * https://issues.labcollab.net/browse/HALO-5059
         */
        sw[0]                           = 0x55;
        sw[1]                           = phr->is_fec_enabled ? 0x6F : 0x90;
        sw[2]                           = 0x4E;

        err = RADIO_ERROR_NONE;

    } while(0);

    return err;
}

int32_t sid_pal_radio_prepare_fsk_for_tx(sid_pal_radio_fsk_pkt_cfg_t *tx_pkt_cfg)
{
    int32_t err = RADIO_ERROR_INVALID_PARAMS;

    do {
        if (tx_pkt_cfg == NULL) {
            break;
        }

        if (tx_pkt_cfg->phy_hdr == NULL || tx_pkt_cfg->packet_params == NULL ||
            tx_pkt_cfg->sync_word == NULL || tx_pkt_cfg->payload == NULL) {
            break;
        }

        sid_pal_radio_fsk_packet_params_t *f_pp  =  tx_pkt_cfg->packet_params;
        if (f_pp->payload_length == 0 || f_pp->preamble_length == 0) {
            err = RADIO_ERROR_INVALID_PARAMS;
            break;
        }

        sid_pal_radio_fsk_phy_hdr_t  *phr  = tx_pkt_cfg->phy_hdr;
        if (((phr->fcs_type == RADIO_FSK_FCS_TYPE_0) &&
              (f_pp->payload_length > MAX_PAYLOAD_LENGTH_WITH_FCS_TYPE_0)) ||
            ((phr->fcs_type == RADIO_FSK_FCS_TYPE_1) &&
              (f_pp->payload_length > MAX_PAYLOAD_LENGTH_WITH_FCS_TYPE_1))) {
            err = RADIO_ERROR_INVALID_PARAMS;
            break;
        }

        uint8_t   psdu_length      = f_pp->payload_length;
        uint8_t   *tx_buffer       = tx_pkt_cfg->payload;
        uint32_t  crc              = 0x00000000;

        if (phr->fcs_type == RADIO_FSK_FCS_TYPE_0) {
            crc = compute_crc32(tx_pkt_cfg->payload, f_pp->payload_length);

            tx_buffer[LR1110_FSK_PHY_HEADER_LENGTH + psdu_length++] = (uint8_t)(crc >> 24);
            tx_buffer[LR1110_FSK_PHY_HEADER_LENGTH + psdu_length++] = (uint8_t)(crc >> 16);
            tx_buffer[LR1110_FSK_PHY_HEADER_LENGTH + psdu_length++] = (uint8_t)(crc >> 8);
            tx_buffer[LR1110_FSK_PHY_HEADER_LENGTH + psdu_length++] = (uint8_t)(crc >> 0);
        } else if (phr->fcs_type == RADIO_FSK_FCS_TYPE_1) {
            crc = compute_crc16(tx_pkt_cfg->payload, f_pp->payload_length);

            tx_buffer[LR1110_FSK_PHY_HEADER_LENGTH + psdu_length++] = (uint8_t)(crc >> 8);
            tx_buffer[LR1110_FSK_PHY_HEADER_LENGTH + psdu_length++] = (uint8_t)(crc >> 0);
        } else {
            err = RADIO_ERROR_NOT_SUPPORTED;
            break;
        }

        uint8_t  *sync_word               = tx_pkt_cfg->sync_word;
        uint8_t  sync_word_length_in_byte = 0;
        memmove(tx_buffer + LR1110_FSK_PHY_HEADER_LENGTH, tx_buffer, f_pp->payload_length);
        // Build the PHR and put it in the Tx buffer
        tx_buffer[0] = (phr->fcs_type << 4);
        tx_buffer[0] += ((phr->is_data_whitening_enabled == true ) ? 1 : 0) << 3;
        tx_buffer[1] = psdu_length;

         if (phr->is_data_whitening_enabled == true) {
             perform_data_whitening(LR1110_FSK_WHITENING_SEED, tx_buffer + LR1110_FSK_PHY_HEADER_LENGTH,
                                    tx_buffer + LR1110_FSK_PHY_HEADER_LENGTH, psdu_length);
         }

         // Build the syncword
         sync_word[sync_word_length_in_byte++] = 0x55;  // Added to force the preamble polarity to a real "0x55"
         sync_word[sync_word_length_in_byte++] = (phr->is_fec_enabled == true) ? 0x6F : 0x90;
         sync_word[sync_word_length_in_byte++] = 0x4E;

          f_pp->sync_word_length     = sync_word_length_in_byte;
         f_pp->addr_comp            = (uint8_t)LR1110_RADIO_GFSK_ADDRESS_FILTERING_DISABLE;
         f_pp->header_type          = (uint8_t)LR1110_RADIO_GFSK_PKT_FIX_LEN;
         f_pp->payload_length       = LR1110_FSK_PHY_HEADER_LENGTH + psdu_length;
         f_pp->crc_type             = (uint8_t)LR1110_RADIO_GFSK_CRC_OFF;
         f_pp->radio_whitening_mode = (uint8_t)LR1110_RADIO_GFSK_DC_FREE_OFF;


         err = RADIO_ERROR_NONE;
    } while(0);

    return err;
}

int32_t sid_pal_radio_set_fsk_sync_word(const uint8_t *sync_word, uint8_t sync_word_length)
{
    if (lr1110_radio_set_gfsk_sync_word(lr1110_get_drv_ctx(), sync_word)
        != LR1110_STATUS_OK) {
        return RADIO_ERROR_HARDWARE_ERROR;
    }
    sync_word_length = 8;

    return RADIO_ERROR_NONE;
}

int32_t sid_pal_radio_set_fsk_whitening_seed(uint16_t seed)
{
    return RADIO_ERROR_NONE;
}

int32_t sid_pal_radio_set_fsk_modulation_params(const sid_pal_radio_fsk_modulation_params_t *mod_params)
{
    if (mod_params == NULL) {
        return RADIO_ERROR_INVALID_PARAMS;
    }

    lr1110_radio_mod_params_gfsk_t fsk_mp;
    radio_mp_to_lr1110_mp(&fsk_mp, mod_params);

    halo_drv_semtech_ctx_t* drv = lr1110_get_drv_ctx();
    assert(drv);
    if (lr1110_radio_set_gfsk_mod_params(drv, &fsk_mp) != LR1110_STATUS_OK) {
        return RADIO_ERROR_HARDWARE_ERROR;
    }
    drv->settings_cache.fsk_mod_params = *mod_params;

    return RADIO_ERROR_NONE;
}

int32_t sid_pal_radio_set_fsk_packet_params(const sid_pal_radio_fsk_packet_params_t *packet_params)
{
    if (packet_params == NULL) {
        return RADIO_ERROR_INVALID_PARAMS;
    }

    lr1110_radio_pkt_params_gfsk_t fsk_pp;
    radio_pp_to_lr1110_pp(&fsk_pp, packet_params);
    halo_drv_semtech_ctx_t* drv = lr1110_get_drv_ctx();
    assert(drv);
    if (lr1110_radio_set_gfsk_pkt_params(drv, &fsk_pp) != LR1110_STATUS_OK) {
        return RADIO_ERROR_HARDWARE_ERROR;
    }
    drv->settings_cache.fsk_pkt_params = *packet_params;

    return RADIO_ERROR_NONE;
}

uint32_t sid_pal_radio_fsk_time_on_air(const sid_pal_radio_fsk_modulation_params_t *mod_params,
                              const sid_pal_radio_fsk_packet_params_t *packet_params,
                              uint8_t packetLen)
{
    if (mod_params == NULL || packet_params == NULL) {
        return 0;
    }

    lr1110_radio_pkt_params_gfsk_t fsk_pp;
    lr1110_radio_mod_params_gfsk_t fsk_mp;

    radio_mp_to_lr1110_mp(&fsk_mp, mod_params);

    fsk_pp.pld_len_in_bytes = packetLen;
    radio_pp_to_lr1110_pp(&fsk_pp, packet_params);

    return lr1110_radio_get_gfsk_time_on_air_in_ms(&fsk_pp, &fsk_mp);
}

uint32_t sid_pal_radio_fsk_get_fsk_number_of_symbols(const sid_pal_radio_fsk_modulation_params_t *mod_params,
                                             uint32_t delay_micro_secs)
{
    uint32_t num_symb = LR1110_US_TO_SYMBOLS(delay_micro_secs, mod_params->bit_rate);
    return num_symb;
}

uint32_t sid_pal_radio_get_fsk_tx_process_delay(void)
{
    halo_drv_semtech_ctx_t *drv_ctx = lr1110_get_drv_ctx();
    return (drv_ctx->config->state_timings.tx_delay_us < LR1110_FSK_PROCESS_DELAY_US_MIN
            || drv_ctx->config->state_timings.tx_delay_us > LR1110_FSK_PROCESS_DELAY_US_MAX)
               ? LR1110_FSK_TX_PROCESS_DELAY_US
               : drv_ctx->config->state_timings.tx_delay_us;
}

uint32_t sid_pal_radio_get_fsk_rx_process_delay(void)
{
    halo_drv_semtech_ctx_t *drv_ctx = lr1110_get_drv_ctx();
    return (drv_ctx->config->state_timings.rx_delay_us < LR1110_FSK_PROCESS_DELAY_US_MIN
            || drv_ctx->config->state_timings.rx_delay_us > LR1110_FSK_PROCESS_DELAY_US_MAX)
               ? LR1110_FSK_RX_PROCESS_DELAY_US
               : drv_ctx->config->state_timings.rx_delay_us;
}
