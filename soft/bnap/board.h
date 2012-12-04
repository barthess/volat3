/*
    ChibiOS/RT - Copyright (C) 2006,2007,2008,2009,2010,
                 2011,2012 Giovanni Di Sirio.

    This file is part of ChibiOS/RT.

    ChibiOS/RT is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    ChibiOS/RT is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _BOARD_H_
#define _BOARD_H_

/*
 * Board identifier.
 */
#define BOARD_BNAP_SAM7_EX256
#define BOARD_NAME "Bnap SAM7A3"

/*
 * Select your platform by modifying the following line.
 */
#if !defined(SAM7A3_PLATFORM)
#define SAM7_PLATFORM   SAM7A3
#endif

#include "at91sam7.h"

#define CLK             18432000
#define MCK             48054857

/**
 * Human readable pseudonims for serial drivers
 */
#define SDGPS                       SDDBG
#define SDMPIOVD                    SD1
#define SDGSM                       SD2
#define SDDM                        SD3 /* display module */
//#define SDGSM                       SD3
//#define SDDM                        SD2 /* display module */

/*
 * I/O definitions.
 */
//#define _MASK (1 << )
/*** PORT A ***/
#define PIOA_I2C_SDA                0
#define PIOA_I2C_SDA_MASK           (1 << PIOA_I2C_SDA)
#define PIOA_I2C_SCK                1
#define PIOA_I2C_SCK_MASK           (1 << PIOA_I2C_SCK)

#define PIOA_USART0_RX              2
#define PIOA_USART0_RX_MASK         (1 << PIOA_USART0_RX)
#define PIOA_USART0_TX              3
#define PIOA_USART0_TX_MASK         (1 << PIOA_USART0_TX)

#define PIOA_USART1_RX              7
#define PIOA_USART1_RX_MASK         (1 << PIOA_USART1_RX)
#define PIOA_USART1_TX              8
#define PIOA_USART1_TX_MASK         (1 << PIOA_USART1_TX)

#define PIOA_USART2_RX              9
#define PIOA_USART2_RX_MASK         (1 << PIOA_USART2_RX)
#define PIOA_USART2_TX              10
#define PIOA_USART2_TX_MASK         (1 << PIOA_USART2_TX)
// mmc-spi
#define PIOA_SPI0_NSS               14
#define PIOA_SPI0_NSS_MASK          (1 << PIOA_SPI0_NSS)
#define PIOA_SPI0_MISO              15
#define PIOA_SPI0_MISO_MASK         (1 << PIOA_SPI0_MISO)
#define PIOA_SPI0_MOSI              16
#define PIOA_SPI0_MOSI_MASK         (1 << PIOA_SPI0_MOSI)
#define PIOA_SPI0_CLK               17
#define PIOA_SPI0_CLK_MASK          (1 << PIOA_SPI0_CLK)
#define PIOA_MMC_CP                 28  /* card present */
#define PIOA_MMC_CP_MASK            (1 << PIOA_MMC_CP)
#define PIOA_MMC_WP                 29 /* write protected */
#define PIOA_MMC_WP_MASK            (1 << PIOA_MMC_WP)

#define PIOA_UARTDBG_RX             30
#define PIOA_UARTDBG_RX_MASK        (1 << PIOA_UARTDBG_RX)
#define PIOA_UARTDBG_TX             31
#define PIOA_UARTDBG_TX_MASK        (1 << PIOA_UARTDBG_TX)

// GSM driving pins
#define PIOA_GSM_ON                 24  // set 1 to power modem on
#define PIOA_GSM_ON_MASK            (1 << PIOA_GSM_ON)
#define PIOA_GSM_RESET              19
#define PIOA_GSM_RESET_MASK         (1 << PIOA_GSM_RESET)
#define PIOA_GPS_RESET              18
#define PIOA_GPS_RESET_MASK         (1 << PIOA_GPS_RESET)

/*** PORT B ***/
// GSM - USART1 (continue)
#define PIOB_PPS                    11
#define PIOB_PPS_MASK               (1 << PIOB_PPS)
#define PIOB_LED_GSM                3
#define PIOB_LED_GSM_MASK           (1 << PIOB_LED_GSM)
#define PIOB_LED_GPS                13
#define PIOB_LED_GPS_MASK           (1 << PIOB_LED_GPS)

#define PIOB_BTN1                   19
#define PIOB_BTN1_MASK              (1 << PIOB_BTN1)

#define PIOB_GSM_RTS                24
#define PIOB_GSM_RTS_MASK           (1 << PIOB_GSM_RTS)
#define PIOB_GSM_CTS                25
#define PIOB_GSM_CTS_MASK           (1 << PIOB_GSM_CTS)
#define PIOB_GSM_SCK                26
#define PIOB_GSM_SCK_MASK           (1 << PIOB_GSM_SCK)

/*
 * Initial I/O setup.
 */
/* Output data. */
//#define VAL_PIOA_ODSR           (0x00000000)
#define VAL_PIOA_ODSR           (0x00000000 | PIOA_GSM_RESET_MASK | PIOA_GPS_RESET_MASK)
/* Direction. */
#define VAL_PIOA_OSR            (0x00000000 | PIOA_GSM_RESET_MASK | PIOA_GPS_RESET_MASK)
//#define VAL_PIOA_OSR            (0x00000000 | PIOA_GSM_ON_MASK | PIOA_GSM_RESET_MASK | PIOA_GPS_RESET_MASK)
/* Pull-up. */
#define VAL_PIOA_PUSR           (0xFFFFFFFF & (~(PIOA_MMC_CP_MASK | PIOA_MMC_WP_MASK)))

/* Output data. */
#define VAL_PIOB_ODSR           0x00000000
/* Direction. */
#define VAL_PIOB_OSR            (0x00000000 | PIOB_GSM_RTS_MASK | PIOB_LED_GSM_MASK | PIOB_LED_GPS_MASK)
/* Pull-up. */
#define VAL_PIOB_PUSR           0xFFFFFFFF


#if !defined(_FROM_ASM_)
#ifdef __cplusplus
extern "C" {
#endif
  void boardInit(void);
#ifdef __cplusplus
}
#endif
#endif /* _FROM_ASM_ */

#endif /* _BOARD_H_ */
