/*
    ChibiOS/RT - Copyright (C) 2006,2007,2008,2009,2010,
                 2011 Giovanni Di Sirio.

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ch.h"
#include "hal.h"

#include "shell.h"
#include "chprintf.h"

#include <mavlink.h>
#include <common.h>
#include <mpiovd.h>

#include "board.h"

#define MPIOVD_DEFAULT_BAUDRATE 115200

static SerialConfig mpiovd_ser_cfg = {
    MPIOVD_DEFAULT_BAUDRATE,
    0,
    0,
    0,
};

/*===========================================================================*/
/* Generic code.                                                             */
/*===========================================================================*/

/*
 * Red LED blinker thread, times are in milliseconds.
 */
static WORKING_AREA(waThread1, 128);
static msg_t Thread1(void *arg) {

  (void)arg;
  chRegSetThreadName("blinker");
  while (TRUE) {
//    palClearPad(IOPORT3, GPIOC_LED);
//    chThdSleepMilliseconds(500);
//    palSetPad(IOPORT3, GPIOC_LED);
    chThdSleepMilliseconds(500);
  }
  return 0;
}

/*
 * Application entry point.
 */
int main(void) {

  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  halInit();
  chSysInit();

  /*
   * Activates the USB driver and then the USB bus pull-up on D+.
   */
  sdStart(&SD2, &mpiovd_ser_cfg);

  /*
   * Shell manager initialization.
   */
  shellInit();

  /*
   * Creates the blinker thread.
   */
  chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);

  /*
   * Normal main() thread activity, in this demo it does nothing except
   * sleeping in a loop and check the button state.
   */
  chThdSleepMilliseconds(2000);

  mavlink_system_t mavlink_system;

  mavlink_system.sysid  = 20;                   ///< ID 20 for this airplane
  mavlink_system.compid = MAV_COMP_ID_IMU;     ///< The component sending the message is the IMU, it could be also a Linux process
  mavlink_system.type   = MAV_TYPE_FIXED_WING;   ///< This system is an airplane / fixed wing

  // Define the system type, in this case an airplane
  uint8_t system_type     = MAV_TYPE_GROUND_ROVER;
  uint8_t autopilot_type  = MAV_AUTOPILOT_GENERIC;
  uint8_t system_mode     = MAV_MODE_PREFLIGHT; ///< Booting up
  uint32_t custom_mode    = 0;                 ///< Custom mode, can be defined by user/adopter
  uint8_t system_state    = MAV_STATE_STANDBY; ///< System ready for flight

  // Initialize the required buffers
  mavlink_message_t msg;
  uint8_t buf[MAVLINK_MAX_PACKET_LEN];

  mavlink_mpiovd_sensors_raw_t raw;

  uint16_t len = 0;
  while (TRUE) {
    chThdSleepMilliseconds(40);
    raw.speed++;

    mavlink_msg_mpiovd_sensors_raw_encode(mavlink_system.sysid, mavlink_system.compid, &msg, &raw);
    len = mavlink_msg_to_send_buffer(buf, &msg);
    sdWrite(&SD2, buf, len);

    if (raw.speed == 255){
      mavlink_msg_heartbeat_pack(mavlink_system.sysid, mavlink_system.compid, &msg, system_type, autopilot_type, system_mode, custom_mode, system_state);
      len = mavlink_msg_to_send_buffer(buf, &msg);
      sdWrite(&SD2, buf, len);
    }
  }

  while (TRUE) {
    chThdSleepMilliseconds(666);
  }
  return 0;
}



