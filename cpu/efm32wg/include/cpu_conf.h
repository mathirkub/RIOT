/**
 * @defgroup        cpu_efm32wg EFM32WG
 * @ingroup         cpu
 * @brief           CPU specific implementations for the EFM32WG
 * @{
 *
 * @file
 * @brief           Implementation specific CPU configuration options
 *
 * @author          Hauke Petersen <hauke.peterse@fu-berlin.de>
 */

#ifndef __CPU_CONF_H
#define __CPU_CONF_H

#include "efm32wg990f256.h"
#include "cpu_conf_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define THREAD_EXTRA_STACKSIZE_PRINTF            (512)

/**
 * @brief   ARM Cortex-M specific CPU configuration
 * @{
 */
#define CPU_DEFAULT_IRQ_PRIO            (1U)
#define CPU_IRQ_NUMOF                   (40U)
#define CPU_FLASH_BASE                  FLASH_BASE
/** @} */

/**
 * @name CC110X buffer size definitions for the stm32f4
 * @{
 */
#ifdef MODULE_CC110X
#define TRANSCEIVER_BUFFER_SIZE         (10)
#define RX_BUF_SIZE                     (10)
#endif
/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __CPU_CONF_H */
/** @} */
