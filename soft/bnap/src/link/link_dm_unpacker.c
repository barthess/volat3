#include "link_dm_unpacker.h"

/*
 ******************************************************************************
 * EXTERNS
 ******************************************************************************
 */

/*
 *******************************************************************************
 * EXPORTED FUNCTIONS
 *******************************************************************************
 */

/**
 *
 */
void dm_command_long_handler(mavlink_message_t *msg){
  (void)msg;
}

/**
 *
 */
void DmUnpackCycle(SerialDriver *sdp){
  mavlink_message_t msg;
  mavlink_status_t status;
  msg_t c = 0;

  while (!chThdShouldTerminate()) {
    // Try to get a new message
    c = sdGetTimeout((SerialDriver *)sdp, MS2ST(200));
    if (c != Q_TIMEOUT){
      uint8_t s = 0;
      s = mavlink_parse_char(MAVLINK_COMM_0, (uint8_t)c, &msg, &status);
      if (s) {
        if (msg.sysid == GROUND_STATION_ID){ /* нас запрашивает наземная станция */
          switch(msg.msgid){
          case MAVLINK_MSG_ID_COMMAND_LONG:
            dm_command_long_handler(&msg);
            break;
          default:
            break;
          }
        }
      }
    }
  }
}
