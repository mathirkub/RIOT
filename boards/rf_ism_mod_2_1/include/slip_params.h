/*
 * slip_params.h
 *
 *  Created on: 19 maj 2016
 *      Author: mateusz
 */

#ifndef BOARDS_RF_ISM_MOD_2_1_INCLUDE_SLIP_PARAMS_H_
#define BOARDS_RF_ISM_MOD_2_1_INCLUDE_SLIP_PARAMS_H_

#include "board.h"

/**
 * @name UART device for TUNSLIP
 * @{
 */
#define SLIP_UART           UART_0
#define SLIP_BAUDRATE       115200

/**
 * @brief   auto_init struct holding SLIP initalization params
 */
const gnrc_slip_params_t gnrc_slip_params[] = {
    {
            .uart = SLIP_UART,
            .baudrate = SLIP_BAUDRATE
    },
};

#endif /* BOARDS_RF_ISM_MOD_2_1_INCLUDE_SLIP_PARAMS_H_ */
