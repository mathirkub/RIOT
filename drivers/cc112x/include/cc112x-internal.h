/*
 * Copyright (C) 2015 Kaspar Schleiser <kaspar@schleiser.de>
 * Copyright (C) 2014 Freie Universität Berlin
 * Copyright (C) 2013 INRIA
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_cc112x
 * @{
 *
 * @file
 * @brief       Data structures and variables for the cc112x driver interface
 *
 * @author      Oliver Hahm <oliver.hahm@inria.fr>
 * @author      Kaspar Schleiser <kaspar@schleiser.de>
 */

#ifndef CC112X_INTERNAL_H
#define CC112X_INTERNAL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CC112X_RXBUF_SIZE           (2)
#define CC112X_MAX_DATA_LENGTH      (58+64)

#define CC112X_HEADER_LENGTH        (3)     /**< Header covers SRC, DST and
                                                 FLAGS */
#define CC112X_BROADCAST_ADDRESS    (0x00)  /**< CC112X broadcast address */

#define MIN_UID                     (0x01)  /**< Minimum UID of a node is
                                                 1 */
#define MAX_UID                     (0xFF)  /**< Maximum UID of a node is
                                                 255 */
/**
 * @name	Channels available in a certain country. *
 * @{
 */
#define MIN_CHANNR                  (25)     /**< Minimum channel number */
#define MAX_CHANNR                  (30)    /**< Maximum channel number */

/** @} */

#define CC112X_PACKET_LENGTH        (255)  /**< max packet length = 255b */
#define CC112X_SYNC_WORD_TX_TIME    (90000) /**< loop count (max. timeout ~15ms)
                                                 to wait for sync word to be
                                                 transmitted (GDO2 from low to
                                                 high) */

#define RESET_WAIT_TIME             (610)   /**< Reset wait time (in reset
                                                 procedure) */
#define IDLE_TO_RX_TIME             (122)   /**< Time chip needs to go to RX */
#define CS_SO_WAIT_TIME             (488)   /**< Time to wait for SO to go low
                                                 after CS */
#define CC112X_GDO1_LOW_RETRY       (100)   /**< Max. retries for SO to go low
                                                 after CS */
#define CC112X_DEFAULT_CHANNEL      (MIN_CHANNR)     /**< The default channel number */
#define CC112X_MIN_CHANNR           (0)     /**< lowest possible channel number */
#define CC112X_MAX_CHANNR           (MAX_CHANNR)     /**< highest possible channel number */

/**
 * @name    State values for state machine
 * @{
 */
enum {
    RADIO_UNKNOWN,
    RADIO_IDLE,
    RADIO_TX_BUSY,
    RADIO_RX,
    RADIO_RX_BUSY,
    RADIO_PWD,
};
/** @} */

/**
 * @brief array holding cc112x register values
 */
extern char cc112x_conf[];

/**
 * @brief   CC112X layer 0 protocol
 *
 * <pre>
---------------------------------------------------
|        |         |         |       |            |
| Length | Address | PhySrc  | Flags |    Data    |
|        |         |         |       |            |
---------------------------------------------------
  1 byte   1 byte    1 byte   1 byte   <= 251 bytes

Flags:
        Bit | Meaning
        --------------------
        7:4 | -
        3:1 | Protocol
          0 | Identification
</pre>
Notes:
\li length & address are given by CC112X
\li Identification is increased is used to scan duplicates. It must be increased
    for each new packet and kept for packet retransmissions.
 */
typedef struct __attribute__((packed))
{
    uint8_t length;                         /**< Length of the packet (without length byte) */
    uint8_t address;                        /**< Destination address */
    uint8_t phy_src;                        /**< Source address (physical source) */
    uint8_t flags;                          /**< Flags */
    uint8_t data[CC112X_MAX_DATA_LENGTH];   /**< Data (high layer protocol) */
} cc112x_pkt_t;

/**
 * @brief struct holding cc112x packet + metadata
 */
typedef struct {
    int8_t rssi;                           /**< RSSI value */
    uint8_t lqi;                            /**< link quality indicator */
    cc112x_pkt_t packet;                    /**< whole packet */
} cc112x_pkt_buf_t;

/**
 * @brief enum for holding cc112x radio on/off state */
typedef enum {
    RADIO_MODE_GET  = -1,                   /**< leave mode unchanged */
    RADIO_MODE_OFF  = 0,                    /**< turn radio off */
    RADIO_MODE_ON   = 1                     /**< turn radio on */
} cc112x_radio_mode_t;

/**
 * @brief   CC112x radio configuration
 */
//typedef struct {
//    cc112x_reg_t reg_cfg;       /**< CC112X register configuration */
//    uint8_t pa_power;           /**< Output power setting */
//} cc112x_cfg_t;

/**
 * @brief   Radio Control Flags
 */
typedef struct {
    uint8_t  _RSSI;             /**< The RSSI value of last received packet */
    uint8_t  _LQI;              /**< The LQI value of the last received packet */
} cc112x_flags_t;

/**
 * @brief   Statistic interface for debugging
 */
typedef struct cc112x_statistic {
    uint32_t    packets_in;             /**< total nr of packets received */
    uint32_t    packets_in_crc_fail;    /**< dropped because of invalid crc */
    uint32_t    packets_in_while_tx;    /**< receive while tx */
    uint32_t    raw_packets_out;        /**< packets sent */
} cc112x_statistic_t;

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* CC112X_INTERNAL_H */
