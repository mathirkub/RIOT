/*
 * Copyright (C) 2016 AGH
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    udp_test for testing the udp connectivity
 * @ingroup     sys
 * @brief       Sends and wait for acknowledgement.
 *
 * @{
 * @file
 * @brief       udp_test utility
 *
 * @author      Kubaszek Mateusz <mathir.km.riot@gmail.com>
 */

#ifndef UDP_TEST_H
#define UDP_TEST_H

#include "stdbool.h"
#include "net/gnrc.h"

#include <string>
#include <cstdio>
#include <cassert>
#include <system_error>
#include "riot/mutex.hpp"
#include "riot/chrono.hpp"
#include "riot/thread.hpp"
#include "riot/condition_variable.hpp"

namespace riot{
namespace udp{

typedef void (*udp_data_handler)(gnrc_pktsnip_t *pkt);

class udp_test{
public:
    static udp_test& getInstace(){
        static udp_test instance;
        return instance;
    }
    inline void set_handler(udp_data_handler hndlr){
        handler = hndlr;
    }

private:
    static constexpr uint16_t port = 6666;
    static gnrc_netreg_entry_t server;
    static udp_data_handler handler;

    static void *thread_function(void *arg);
    char t_stack[THREAD_STACKSIZE_MAIN];

    static void default_handler(gnrc_pktsnip_t *pkt);
    udp_test();
    ~udp_test();
    int start_server(void);
    int stop_server(void);
};

} // namespace udp
} // namespace riot

#endif /* UDP_TEST_H */
/** @} */

