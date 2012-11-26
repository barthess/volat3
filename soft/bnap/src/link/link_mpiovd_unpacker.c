#include "ch.h"
#include "hal.h"

#include "mavlink.h"

#include "link.h"
#include "link_cc.h"
#include "message.h"
#include "main.h"

/*
 ******************************************************************************
 * EXTERNS
 ******************************************************************************
 */
extern GlobalFlags_t GlobalFlags;

extern mavlink_mpiovd_sensors_t        mavlink_mpiovd_sensors_struct;
extern mavlink_mpiovd_sensors_dbg_t    mavlink_mpiovd_sensors_dbg_struct;

extern EventSource event_mavlink_mpiovd_sensors;
extern EventSource event_mavlink_mpiovd_sensors_dbg;
extern EventSource event_mpiovd_heartbeat;

/*
 *******************************************************************************
 * EXPORTED FUNCTIONS
 *******************************************************************************
 */

#define MPIOVD_ID 21

/**
 *
 */
void MpiovdUnpackCycle(SerialDriver *sdp){
  mavlink_message_t msg;
  mavlink_status_t status;
  msg_t c = 0;
  uint8_t n = 0;

  while(GlobalFlags.messaging_ready == 0)
    chThdSleepMilliseconds(50);

  memset(buf, 0, sizeof(buf));

  while (!chThdShouldTerminate()) {
    c = sdGet((SerialDriver *)sdp);

    if (mavlink_parse_char(MAVLINK_COMM_1, (uint8_t)c, &msg, &status)) {
      if (msg.sysid == MPIOVD_ID){
        switch(msg.msgid){
        case MAVLINK_MSG_ID_MPIOVD_SENSORS:
          mavlink_msg_mpiovd_sensors_decode(&msg, &mavlink_mpiovd_sensors_struct);
          chEvtBroadcastFlags(&event_mavlink_mpiovd_sensors, EVMSK_MAVLINK_MPIOVD_SENSORS);
          break;

        case MAVLINK_MSG_ID_MPIOVD_SENSORS_DBG:
          mavlink_msg_mpiovd_sensors_dbg_decode(&msg, &mavlink_mpiovd_sensors_dbg_struct);
          chEvtBroadcastFlags(&event_mavlink_mpiovd_sensors_dbg, EVMSK_MAVLINK_MPIOVD_SENSORS_DBG);
          break;

        case MAVLINK_MSG_ID_HEARTBEAT:
          chEvtBroadcastFlags(&event_mpiovd_heartbeat, EVMSK_MPIOVD_HEARTBEAT);
          break;

        default:
          break;
        }
      }
    }
  }
}
