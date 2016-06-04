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
#include "reboot.h"
#include "timex.h"
#include "xtimer.h"
#include "thread.h"

#include "include/udp_test.h"

#include "log.h"

namespace riot{
namespace udp{

    void udp_test::default_handler(gnrc_pktsnip_t *pkt){
        (void)pkt;
    }

    void *udp_test::thread_function(void *arg){
        (void)arg;
        puts("started");
        msg_t msg, reply;
        msg_t msg_queue[GNRC_PKTDUMP_MSG_QUEUE_SIZE];

        /* setup the message queue */
        msg_init_queue(msg_queue, GNRC_PKTDUMP_MSG_QUEUE_SIZE);

        reply.content.value = (uint32_t)(-ENOTSUP);
        reply.type = GNRC_NETAPI_MSG_TYPE_ACK;
        while(1){
            msg_receive(&msg);
                switch (msg.type) {
                    case GNRC_NETAPI_MSG_TYPE_RCV:
                        puts("PKTDUMP: data received:");
                        break;
                    case GNRC_NETAPI_MSG_TYPE_SND:
                        puts("PKTDUMP: data to send:");
                        break;
                    case GNRC_NETAPI_MSG_TYPE_GET:
                    case GNRC_NETAPI_MSG_TYPE_SET:
                        msg_reply(&msg, &reply);
                        break;
                    default:
                        puts("PKTDUMP: received something unexpected");
                        break;
                }
        }

        return NULL;
    }

    udp_test::udp_test() {
        /* initializing udp data handler */
        handler = default_handler;
        /* initializing netreg entry */
        server = { NULL, GNRC_NETREG_DEMUX_CTX_ALL, KERNEL_PID_UNDEF };
        if(!start_server()){
            LOG_ERROR("%s:%u Fatal error, cannot start UDP server...\n", __func__, __LINE__);
            reboot();
        }
    }

    int udp_test::start_server(){
        /* check if server is already running */
        if(server.pid!=KERNEL_PID_UNDEF) {
            return 1;
        }
        /* start server (which means registering pktdump for the chosen port) */
        server.pid = thread_create(t_stack, sizeof(t_stack), 1, THREAD_CREATE_STACKTEST, thread_function, NULL, "udp_test");
        server.demux_ctx = (uint32_t)port;
        if(gnrc_netreg_register(GNRC_NETTYPE_UDP, &server))
            return -1;
        LOG_INFO("Success: started UDP server on port %" PRIu16 "\n", port);
        return 0;
    }
} // udp
} // riot

//
//int udp_test_server_init(void)
//{
//
//    /* check if server is already running */
//    if(server.pid!=KERNEL_PID_UNDEF) {
//        return 1;
//    }
//    /* start server (which means registering pktdump for the chosen port) */
//    server.pid = gnrc_pktdump_pid;
//    server.demux_ctx = (uint32_t)port;
//    if(gnrc_netreg_register(GNRC_NETTYPE_UDP, &server))
//        return -1;
//    printf("Success: started UDP server on port %" PRIu16 "\n", port);
//    return 0;
//}
//
//int udp_test_server_deinit(void)
//{
//    /* check if server is running at all */
//    if(server.pid==KERNEL_PID_UNDEF) {
//        printf("Error: server was not running\n");
//        return -1;
//    }
//    /* stop server */
//    gnrc_netreg_unregister(GNRC_NETTYPE_UDP, &server);
//    server.pid = KERNEL_PID_UNDEF;
//    puts("Success: stopped UDP server");
//    return 0;
//}
//
//int udp_test_send(char *addr_str, char *data)
//{
//    ipv6_addr_t addr;
//
//    /* parse destination address */
//    if(ipv6_addr_from_str(&addr, addr_str)==NULL) {
//        puts("Error: unable to parse destination address");
//        return -1;
//    }
//
//    gnrc_pktsnip_t *payload, *udp, *ip;
//    /* allocate payload */
//    payload = gnrc_pktbuf_add(NULL, data, strlen(data), GNRC_NETTYPE_UNDEF);
//    if(payload==NULL) {
//        puts("Error: unable to copy data to packet buffer");
//        return -1;
//    }
//    /* allocate UDP header, set source port := destination port */
//    udp = gnrc_udp_hdr_build(payload, port, port);
//    if(udp==NULL) {
//        puts("Error: unable to allocate UDP header");
//        gnrc_pktbuf_release(payload);
//        return -1;
//    }
//    /* allocate IPv6 header */
//    ip = gnrc_ipv6_hdr_build(udp, NULL, &addr);
//    if(ip==NULL) {
//        puts("Error: unable to allocate IPv6 header");
//        gnrc_pktbuf_release(udp);
//        return -1;
//    }
//    /* send packet */
//    if(!gnrc_netapi_dispatch_send(GNRC_NETTYPE_UDP, GNRC_NETREG_DEMUX_CTX_ALL, ip)) {
//        puts("Error: unable to locate UDP thread");
//        gnrc_pktbuf_release(ip);
//        return -1;
//    }
//    return 0;
//}
