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

/**
 * @brief   ARM Cortex-M specific CPU configuration
 * @{
 */
#define CPU_DEFAULT_IRQ_PRIO            (1U)
#define CPU_IRQ_NUMOF                   (40U)
#define CPU_FLASH_BASE                  FLASH_BASE
/** @} */


#ifdef __cplusplus
}
#endif

#endif /* __CPU_CONF_H */
/** @} */
