/*
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup   board_rf_ism_mod_2_1
 * @{
 *
 * @file
 * @brief     cc112x board specific configuration
 *
 * @author      Mateusz Kubaszek <mathir.km.riot@gmail.com>
 */

#ifndef CC1125_PARAMS_H_
#define CC1125_PARAMS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "cc112x.h"
#include "board.h"
#include "periph_cpu.h"

#define USE_BOARD_PARAMETERS

#define CC_SPI		SPI_CC1125
#define CC_CS		GPIO_T(GPIO_PORT_C, 5)
#define CC_GPIO0	GPIO_T(GPIO_PORT_C, 1)
#define CC_GPIO1	GPIO_T(GPIO_PORT_C, 3)
#define CC_GPIO2	GPIO_T(GPIO_PORT_A, 6)
#define CC_GPIO3	GPIO_T(GPIO_PORT_C, 0)
#define CC_RESET	GPIO_T(GPIO_PORT_A, 5)

/**
 * @name CC112X configuration
 */
const cc112x_params_t cc112x_params[] = {
    {
        .spi  = CC_SPI,
        .cs   = CC_CS,
        .gpio0 = CC_GPIO0,
		.gpio1 = CC_GPIO1,
        .gpio2 = CC_GPIO2,
		.gpio3 = CC_GPIO3,
		.reset = CC_RESET
    },
};

#ifdef __cplusplus
}
#endif

#endif /* CC1125_PARAMS_H_ */
