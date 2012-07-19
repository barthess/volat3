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
 * Setup for the MPIOVD board.
 */

/*
 * Board identifier.
 */
#define BOARD_MPIOVD
#define BOARD_NAME              "MPIOVD rev.1 board"

/*
 * Board frequencies.
 */
#define STM32_LSECLK            0
#define STM32_HSECLK            4000000

/*
 * MCU type, supported types are defined in ./os/hal/platforms/hal_lld.h.
 */
#define STM32F10X_HD

/*
 * IO pins assignments.
 */

#define GPIOB_BOOT_1            2
#define GPIOB_SR_IN_NSS         12

#define GPIOC_FREQUENCY         6
#define GPIOC_USB_DISC

#define GPIOE_LED               0
#define GPIOE_SR_OUT_NSS        3
#define GPIOE_SR_SAMPLE         4
#define GPIOE_Z_CHECK           5
#define GPIOE_SPEEDOMETER       8
#define GPIOE_USB_DISCOVERY     14
#define GPIOE_USB_PRESENT       15

#define GPIOD_CAN_RX            0
#define GPIOD_CAN_TX            1
#define GPIOD_TACHOMETER        12

/*
 * I/O ports initial setup, this configuration is established soon after reset
 * in the initialization code.
 *
 * The digits have the following meaning:
 *   0 - Analog input.
 *   1 - Push Pull output 10MHz.
 *   2 - Push Pull output 2MHz.
 *   3 - Push Pull output 50MHz.
 *   4 - Digital input.
 *   5 - Open Drain output 10MHz.
 *   6 - Open Drain output 2MHz.
 *   7 - Open Drain output 50MHz.
 *   8 - Digital input with PullUp or PullDown resistor depending on ODR.
 *   9 - Alternate Push Pull output 10MHz.
 *   A - Alternate Push Pull output 2MHz.
 *   B - Alternate Push Pull output 50MHz.
 *   C - Reserved.
 *   D - Alternate Open Drain output 10MHz.
 *   E - Alternate Open Drain output 2MHz.
 *   F - Alternate Open Drain output 50MHz.
 * Please refer to the STM32 Reference Manual for details.
 */

/*
 * Port A setup.
 * Everything input with pull-up except:
 * PA0..PA7 - analog input.
 */
#define VAL_GPIOACRL            0x00000000      /*  PA7...PA0 */
#define VAL_GPIOACRH            0x888884A8      /* PA15...PA8 */
#define VAL_GPIOAODR            0xFFFFFFFF

/*
 * Port B setup.
 * Everything input with pull-up except:
 * PB0,1 - Analog input.
 * PB10,11 - I2C.
 * PB12 - serial input NSS
 * PB13 - serial clock
 * PB14 - serial input MISO
 * PB15 - serial input MOSI
 */
#define VAL_GPIOBCRL            0xEE888800      /*  PB7...PB0 */
#define VAL_GPIOBCRH            0xA4A2EE88      /* PB15...PB8 */
#define VAL_GPIOBODR            0xFFFFFFFF

/*
 * Port C setup.
 * Everything input with pull-up except:
 * PC0..PC5 - analog input
 * PC6 - frequency meter
 *
 */
#define VAL_GPIOCCRL            0x88400000      /*  PC7...PC0 */
#define VAL_GPIOCCRH            0x88863388      /* PC15...PC8 */
#define VAL_GPIOCODR            0xFFFFFFFF

/*
 * Port D setup.
 * Everything input with pull-up except:
 * PD0  - CAN RX.
 * PD1  - CAN TX.
 * PD12 - tachometer
 */
#define VAL_GPIODCRL            0x888888A4      /*  PD7...PD0 */
#define VAL_GPIODCRH            0x88848888      /* PD15...PD8 */
#define VAL_GPIODODR            0xFFFFFFFF

/*
 * Port E setup.
 * Everything input with pull-up except:
 * PE0 - LED
 * PE2 - serial out ENABLE
 * PE3 - serial out NSS
 * PE4 - serial SAMPLE
 * PE5 - serial Z_CHECK_TOGGLE
 * PE9 - spedometer input
 * PE14 - usb discovery
 * PE15 - usb present
 */
#define VAL_GPIOECRL            0x88222286      /*  PE7...PE0 */
#define VAL_GPIOECRH            0x42888848      /* PE15...PE8 */
#define VAL_GPIOEODR            0xFFFFFFFF

/*
 * Port F setup. Stub
 */
#define VAL_GPIOFCRL            0x88888888      /*  PE7...PE0 */
#define VAL_GPIOFCRH            0x88888888      /* PE15...PE8 */
#define VAL_GPIOFODR            0xFFFFFFFF

/*
 * Port G setup. Stub
 */
#define VAL_GPIOGCRL            0x88888888      /*  PE7...PE0 */
#define VAL_GPIOGCRH            0x88888888      /* PE15...PE8 */
#define VAL_GPIOGODR            0xFFFFFFFF

/*
 * USB bus activation macro, required by the USB driver.
 */
#define usb_lld_connect_bus(usbp) palClearPad(GPIOE, GPIOE_USB_DISCOVERY)

/*
 * USB bus de-activation macro, required by the USB driver.
 */
#define usb_lld_disconnect_bus(usbp) palSetPad(GPIOE, GPIOE_USB_DISCOVERY)

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
