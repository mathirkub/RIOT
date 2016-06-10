/*
 * Copyright (C) 2016 AGH
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     examples
 * @{
 *
 * @file
 * @brief       Demonstrating the sending and receiving of UDP data.
 *
 * @author      Kubaszek Mateusz <mathir.km.riot@gmail.com>
 *
 * @}
 */

#include "stdbool.h"
#include "net/gnrc.h"
#include "net/gnrc/ipv6.h"
#include "net/gnrc/udp.h"
#include "net/gnrc/pktdump.h"
#include "random.h"
#include "xtimer.h"
#include "thread.h"
#include "checksum/crc16_ccitt.h"
#include "od.h"

#define LOG_LEVEL LOG_NONE

#include "log.h"

#define UDP_SENT            (0x8325)
#define UDP_TIMEOUT         (0x8326)
#define UDP_RECEIVED_ACK    (0x8327)
#define UDP_TERMINATE       (0x8328)

static const uint16_t port = 6666;
static gnrc_netreg_entry_t server = {NULL, GNRC_NETREG_DEMUX_CTX_ALL, KERNEL_PID_UNDEF};
static char t_stack[THREAD_STACKSIZE_MAIN];

typedef struct {
    uint32_t hash;
    uint16_t crc;
    uint64_t snd_time;
}__attribute__((packed)) udp_data_t;

static udp_data_t udp_data;

/**
 * @brief           Gather port and address from pktsnip and sends ACK message.
 *
 * @param[in]  pkt  Pointer to UDP snip
 * @param[in]  hash Hash of ACK to send
 *
 * @return          0 if no error occured and ACK was successfully sent
 *                  -1 in case if error occurred
 */
static int send_ack(gnrc_pktsnip_t *pkt, uint32_t hash)
{
    LOG_DEBUG("%s:%u\n", __func__, __LINE__);
    udp_hdr_t *udp_hdr;
    /* getting data from packet */
    if(pkt->type==GNRC_NETTYPE_UDP) {
        udp_hdr = pkt->data;
        pkt = pkt->next;
    } else {
        LOG_WARNING("%s:%u:First pktsnip is not an UDP header\n", __func__, __LINE__);
        return -1;
    }
    ipv6_hdr_t *ipv6_hdr = pkt->data;
    if(pkt->type==GNRC_NETTYPE_IPV6) {
        if(!ipv6_hdr_is(ipv6_hdr)) {
            LOG_WARNING("%s:%u:Following pktsnip is not an IPv6 header\n", __func__, __LINE__);
            return -1;
        }
        /* Hash and crc updating  */
        udp_data_t ack;
        ack.hash = hash;
        ack.crc = crc16_ccitt_calc((uint8_t*)(&ack.hash), 4);

        gnrc_pktsnip_t *crc_hash, *udp, *ip;
        /* allocate crc_hash data */
        crc_hash = gnrc_pktbuf_add(NULL, (void*)(&ack.hash), sizeof(ack.hash)+sizeof(ack.crc), GNRC_NETTYPE_UNDEF);
        if(crc_hash==NULL) {
            LOG_ERROR("%s:%u:Unable to copy data to crc_hash packet\n", __func__, __LINE__);
            return -1;
        }
        /* allocate UDP header, set source port := destination port */
        udp = gnrc_udp_hdr_build(crc_hash, byteorder_ntohs(udp_hdr->dst_port), byteorder_ntohs(udp_hdr->src_port));
        if(udp==NULL) {
            LOG_ERROR("%s:%u:Unable to allocate UDP header\n", __func__, __LINE__);
            gnrc_pktbuf_release(crc_hash);
            return -1;
        }
        /* allocate IPv6 header */
        ip = gnrc_ipv6_hdr_build(udp, NULL, &ipv6_hdr->src);
        if(ip==NULL) {
            LOG_ERROR("%s:%u:Unable to allocate IPv6 header\n", __func__, __LINE__);
            gnrc_pktbuf_release(udp);
            return -1;
        }
        /* send packet */
        if(!gnrc_netapi_dispatch_send(GNRC_NETTYPE_UDP, GNRC_NETREG_DEMUX_CTX_ALL, ip)) {
            LOG_ERROR("%s:%u:Unable to locate UDP thread\n", __func__, __LINE__);
            gnrc_pktbuf_release(ip);
            return -1;
        }
        LOG_INFO("%s:%u:ACK sent\n", __func__, __LINE__);
    } else {
        LOG_WARNING("%s:%u:No IPv6 pktsnip\n", __func__, __LINE__);
        return -1;
    }
    return 0;
}

/**
 * @brief           Dispatches data snip and printf its content. Recognizes the ACK frame.
 * @params[in]      pktsnip with data and following netlayers
 *
 * @return          0 if no error ocurred, -1 in case of inappropriate message has been received
 *                  or an error during ACK sending occurred.
 */
static int receive_packet(gnrc_pktsnip_t *pkt)
{
    LOG_DEBUG("%s:%u\n", __func__, __LINE__);
    int ret = -1;
    if(pkt->type==GNRC_NETTYPE_UNDEF) {
        /* first four bytes is message hash */
        uint32_t hash;
        /* nexts two bytes are crc of message */
        uint16_t crc, crc_calc;

        if(pkt->size>=6) {
            /* data pointer */
            uint8_t *data_pointer = (uint8_t*)pkt->data+6;
            uint8_t data_length = pkt->size-6;

            hash = *((uint32_t*)pkt->data);
            crc = *((uint16_t*)((uint8_t*)pkt->data+4));

            crc_calc = crc16_ccitt_calc(data_pointer, data_length);
            crc_calc = crc16_ccitt_update(crc_calc, (uint8_t*)(&hash), 4);
            /* If here is no errors in recieved data */
            if(crc_calc==crc) {
                if(data_length) {
                    /* sending ACK to the sender */
                    ret = send_ack(pkt->next, hash);
//                    if(true == verbose) {
//                        printf("Received message on UDP %d port (sending ack):\n", port);
//                        /* checking if the message is textual or just simple data */
//                        bool onlyChars = true;
//                        for(uint8_t a = 0; a<data_length; ++a) {
//                            if(data_pointer[a]<0x20&&data_pointer[a]!=0&&data_pointer[a]<127) {
//                                onlyChars = false;
//                                break;
//                            }
//                        }
//                        /* printing the message */
//                        if(onlyChars) {
//                            printf("\t");
//                            for(uint8_t a = 0; a<data_length; ++a)
//                                printf("%c", data_pointer[a]);
//                            printf("\n");
//                        } else {
//                            od_hex_dump(data_pointer, data_length, 16);
//                        }
//                    }
                } else {
                    LOG_INFO("%s:%u:Received ACK frame\n", __func__, __LINE__);
                    msg_t msg;
                    msg.sender_pid = thread_getpid();
                    msg.type = UDP_RECEIVED_ACK;
                    msg.content.value = (uint32_t)((xtimer_now64()-udp_data.snd_time));
                    msg_send(&msg, 2);
                    ret = 0;
                }
            } else {
                LOG_INFO("%s:%u:Received message on UDP %d port, CRC mismatch...\n", __func__, __LINE__, port);
            }
        }
    }
    /* deleting the whole snip */
    gnrc_pktbuf_release(pkt);
    return ret;
}

/**
 * @brief           Fills given memory space with random data.
 * @params[in]      data - space to be filled with random data
 * @params[in]      length - length of space in bytes
 */
void get_random_data(uint8_t *data, uint8_t length)
{
    for(uint8_t i = 0; i<length; ++i) {
        data[i] = random_uint32()%0xff;
    }
}

static void *thread_function(void *arg)
{
    (void)arg;

    msg_t msg, reply;
    msg_t msg_queue[GNRC_PKTDUMP_MSG_QUEUE_SIZE];

    gnrc_pktsnip_t *pkt;

    /* setup the message queue */
    msg_init_queue(msg_queue, GNRC_PKTDUMP_MSG_QUEUE_SIZE);

    reply.content.value = (uint32_t)(-ENOTSUP);
    reply.type = GNRC_NETAPI_MSG_TYPE_ACK;

    LOG_INFO("%s:%u:UPD_TEST thread started\n", __func__, __LINE__);

    while(1) {
        msg_receive(&msg);
        switch(msg.type) {
        case GNRC_NETAPI_MSG_TYPE_RCV:
            pkt = (gnrc_pktsnip_t *)msg.content.ptr;
            receive_packet(pkt);
            break;
        case UDP_SENT:
            /* Loop until correct message hasn't been received */
            LOG_INFO("%s:%u:Message has been sent, waiting for ack\n", __func__, __LINE__);
            do {
                msg_receive(&msg);
                if(GNRC_NETAPI_MSG_TYPE_RCV==msg.type) {
                    pkt = (gnrc_pktsnip_t *)msg.content.ptr;
                    receive_packet(pkt);
                }
                if(UDP_TIMEOUT==msg.type) {
                    LOG_INFO("%s:%u:Timeout...\n", __func__, __LINE__);
                    break;
                }
            } while(1);
            break;
        case GNRC_NETAPI_MSG_TYPE_GET:
            case GNRC_NETAPI_MSG_TYPE_SET:
            msg_reply(&msg, &reply);
            break;
        default:
            LOG_WARNING("%s:%u:PKTDUMP: received something unexpected\n", __func__, __LINE__);
            break;
        }
    }

    return NULL;
}

int udp_test_server_init(void)
{
    LOG_INFO("%s:%u:Starting UDP_TEST server\n", __func__, __LINE__);
    /* initializing random generator */
    random_init(123456789);
    /* check if server is already running */
    if(server.pid!=KERNEL_PID_UNDEF) {
        return 1;
    }
    /* start server (which means registering pktdump for the chosen port) */
    server.pid = thread_create(t_stack, sizeof(t_stack), THREAD_PRIORITY_MAIN - 1, THREAD_CREATE_STACKTEST, thread_function, NULL, "udp_test");
    /* causes memory access violation
     * 1# server.pid = thread_create(t_stack, sizeof(t_stack), THREAD_PRIORITY_MAIN, THREAD_CREATE_STACKTEST, thread_function, NULL, "udp_test");
     * 2# server.pid = thread_create(t_stack, sizeof(t_stack), THREAD_PRIORITY_MAIN + 1, THREAD_CREATE_STACKTEST, thread_function, NULL, "udp_test");
     * 3# server.pid = thread_create(t_stack, sizeof(t_stack), THREAD_PRIORITY_MAIN - 1, THREAD_CREATE_STACKTEST | THREAD_CREATE_WOUT_YIELD, thread_function, NULL, "udp_test");
     */

    server.demux_ctx = (uint32_t)port;
    if(gnrc_netreg_register(GNRC_NETTYPE_UDP, &server))
        return -1;
    printf("Success: started UDP server on port %" PRIu16 "\n", port);
    return 0;
}

int udp_test_server_deinit(void)
{
    LOG_INFO("%s:%u:Deinitializing reading thread\n", __func__, __LINE__);
    /* check if server is running at all */
    if(server.pid==KERNEL_PID_UNDEF) {
        printf("Error: server was not running\n");
        return 1;
    }
    /* stop server */
    gnrc_netreg_unregister(GNRC_NETTYPE_UDP, &server);
    server.pid = KERNEL_PID_UNDEF;
    puts("Success: stopped UDP server\n");
    return 0;
}

int32_t udp_test_send(char *addr_str, void *data, uint8_t len)
{
    ipv6_addr_t addr;

    if(server.pid==KERNEL_PID_UNDEF) {
        if(udp_test_server_init())
            return -1;
        thread_yield();
    }

    /* parse destination address */
    if(ipv6_addr_from_str(&addr, addr_str)==NULL) {
        LOG_ERROR("%s:%u:Unable to parse destination address\n", __func__, __LINE__);
        return -1;
    }

    /* preparing payload */
    udp_data.crc = crc16_ccitt_calc(data, len);
    udp_data.hash = random_uint32();
    udp_data.crc = crc16_ccitt_update(udp_data.crc, (uint8_t*)(&udp_data.hash), 4);

    gnrc_pktsnip_t *crc_hash, *payload, *udp, *ip;
    /* allocate payload */
    payload = gnrc_pktbuf_add(NULL, data, len, GNRC_NETTYPE_UNDEF);
    if(payload==NULL) {
        LOG_ERROR("%s:%u:Unable to copy data to packet buffer\n", __func__, __LINE__);
        return -1;
    }
    /* allocate crc_hash data */
    crc_hash = gnrc_pktbuf_add(payload, (void*)(&udp_data.hash), sizeof(udp_data.hash)+sizeof(udp_data.crc), GNRC_NETTYPE_UNDEF);
    if(crc_hash==NULL) {
        LOG_ERROR("%s:%u:Unable to copy data to crc_hash packet\n", __func__, __LINE__);
        gnrc_pktbuf_release(crc_hash);
    }
    /* allocate UDP header, set source port := destination port */
    udp = gnrc_udp_hdr_build(crc_hash, port, port);
    if(udp==NULL) {
        LOG_ERROR("%s:%u:Unable to allocate UDP header\n", __func__, __LINE__);
        gnrc_pktbuf_release(crc_hash);
        return -1;
    }
    /* allocate IPv6 header */
    ip = gnrc_ipv6_hdr_build(udp, NULL, &addr);
    if(ip==NULL) {
        LOG_ERROR("%s:%u:Unable to allocate IPv6 header\n", __func__, __LINE__);
        gnrc_pktbuf_release(udp);
        return -1;
    }

    udp_data.snd_time = xtimer_now64();

    /* sending msg to receiving thread */
    msg_t msg;
    msg.sender_pid = thread_getpid();
    msg.type = UDP_SENT;
    msg_send(&msg, server.pid);

    /* send packet */
    if(!gnrc_netapi_dispatch_send(GNRC_NETTYPE_UDP, GNRC_NETREG_DEMUX_CTX_ALL, ip)) {
        LOG_ERROR("%s:%u:Unable to locate UDP thread\n", __func__, __LINE__);
        gnrc_pktbuf_release(ip);
        msg.sender_pid = thread_getpid();
        msg.type = UDP_TERMINATE;
        msg_send(&msg, server.pid);
        return -1;
    }

    /* wait max 2 secs */
    for(uint8_t i = 0; i<200; ++i) {
        xtimer_usleep(10000);
        if(1==msg_try_receive(&msg)) {
            LOG_INFO("%s:%u:Received ACK after %d ms\n", __func__, __LINE__, msg.content.value);
            return msg.content.value;
        }
    }

    LOG_INFO("%s:%u:No ACK...\n", __func__, __LINE__);
    msg.sender_pid = thread_getpid();
    msg.type = UDP_TERMINATE;
    msg_send(&msg, server.pid);
    return -1;
}

//
//namespace riot{
//namespace udp{
//
//    void udp_test::default_handler(gnrc_pktsnip_t *pkt){
//        (void)pkt;
//    }
//
//    void *udp_test::thread_function(void *arg){
//        (void)arg;
//        puts("started");
//        msg_t msg, reply;
//        msg_t msg_queue[GNRC_PKTDUMP_MSG_QUEUE_SIZE];
//
//        /* setup the message queue */
//        msg_init_queue(msg_queue, GNRC_PKTDUMP_MSG_QUEUE_SIZE);
//
//        reply.content.value = (uint32_t)(-ENOTSUP);
//        reply.type = GNRC_NETAPI_MSG_TYPE_ACK;
//        while(1){
//            msg_receive(&msg);
//                switch (msg.type) {
//                    case GNRC_NETAPI_MSG_TYPE_RCV:
//                        puts("PKTDUMP: data received:");
//                        break;
//                    case GNRC_NETAPI_MSG_TYPE_SND:
//                        puts("PKTDUMP: data to send:");
//                        break;
//                    case GNRC_NETAPI_MSG_TYPE_GET:
//                    case GNRC_NETAPI_MSG_TYPE_SET:
//                        msg_reply(&msg, &reply);
//                        break;
//                    default:
//                        puts("PKTDUMP: received something unexpected");
//                        break;
//                }
//        }
//
//        return NULL;
//    }
//
//    udp_test::udp_test() {
//        /* initializing udp data handler */
//        handler = default_handler;
//        /* initializing netreg entry */
//        server = { NULL, GNRC_NETREG_DEMUX_CTX_ALL, KERNEL_PID_UNDEF };
//        if(!start_server()){
//            LOG_ERROR("%s:%u Fatal error, cannot start UDP server...\n", __func__, __LINE__);
//            reboot();
//        }
//    }
//
//    int udp_test::start_server(){
//        /* check if server is already running */
//        if(server.pid!=KERNEL_PID_UNDEF) {
//            return 1;
//        }
//        /* start server (which means registering pktdump for the chosen port) */
//        server.pid = thread_create(t_stack, sizeof(t_stack), 1, THREAD_CREATE_STACKTEST, thread_function, NULL, "udp_test");
//        server.demux_ctx = (uint32_t)port;
//        if(gnrc_netreg_register(GNRC_NETTYPE_UDP, &server))
//            return -1;
//        LOG_INFO("Success: started UDP server on port %" PRIu16 "\n", port);
//        return 0;
//    }
//} // udp
//} // riot
