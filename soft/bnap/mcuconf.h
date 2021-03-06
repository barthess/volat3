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

/*
 * AT91SAM7 drivers configuration.
 * The following settings override the default settings present in
 * the various device driver implementation headers.
 * Note that the settings for each driver only have effect if the driver
 * is enabled in halconf.h.
 */

/*
Note:
AT91C_AIC_PRIOR_LOWEST               (0x0) // (AIC) Lowest priority level
AT91C_AIC_PRIOR_HIGHEST              (0x7) // (AIC) Highest priority level
*/

/*
 * ADC driver system settings.
 */

/*
 * CAN driver system settings.
 */

/*
 * MAC driver system settings.
 */
#define MAC_TRANSMIT_BUFFERS        2
#define MAC_RECEIVE_BUFFERS         2
#define MAC_BUFFERS_SIZE            1518
#define EMAC_INTERRUPT_PRIORITY     (AT91C_AIC_PRIOR_HIGHEST - 3)

/*
 * PWM driver system settings.
 */

/*
 * SERIAL driver system settings.
 */
#define USE_SAM7_USART0             TRUE
#define USE_SAM7_USART1             TRUE
#define USE_SAM7_USART2             TRUE
#define USE_SAM7_DBGU_UART          TRUE
#define SAM7_USART0_PRIORITY        3
#define SAM7_USART1_PRIORITY        2
#define SAM7_USART2_PRIORITY        1
#define SAM7_DBGU_UART_PRIORITY     0
//#define SAM7_USART0_PRIORITY        (AT91C_AIC_PRIOR_HIGHEST - 2)
//#define SAM7_USART1_PRIORITY        (AT91C_AIC_PRIOR_HIGHEST - 2)
//#define SAM7_USART2_PRIORITY        (AT91C_AIC_PRIOR_HIGHEST - 2)
//#define SAM7_DBGU_UART_PRIORITY     (AT91C_AIC_PRIOR_HIGHEST - 2)
/*
 * SPI driver system settings.
 */
#define AT91SAM7_SPI_USE_SPI0       TRUE
#define AT91SAM7_SPI_USE_SPI1       FALSE
#define AT91SAM7_SPI0_PRIORITY      4
#define AT91SAM7_SPI1_PRIORITY      4

/*
 * I2C driver system settings.
 */
#define SAM7_I2C_USE_I2C1           TRUE
#define SAM7_I2C1_PRIORITY          5
