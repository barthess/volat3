#include "ch.h"
#include "hal.h"

#include "mavlink.h"

#include "link.h"
#include "link_cc.h"
#include "message.h"
#include "main.h"
#include "timekeeper.h"

/*
 ******************************************************************************
 * EXTERNS
 ******************************************************************************
 */
extern GlobalFlags_t GlobalFlags;

extern mavlink_mpiovd_sensors_t       mavlink_mpiovd_sensors_struct;
extern mavlink_heartbeat_mpiovd_t     mavlink_heartbeat_mpiovd_struct;

extern EventSource event_mavlink_mpiovd_sensors;
extern EventSource event_mavlink_heartbeat_mpiovd;

/*
 *******************************************************************************
 * EXPORTED FUNCTIONS
 *******************************************************************************
 */

/**
 *
 */
void MpiovdUnpackCycle(SerialDriver *sdp){
  mavlink_message_t msg;
  mavlink_status_t status;
  msg_t c = 0;

  while(GlobalFlags.messaging_ready == 0)
    chThdSleepMilliseconds(50);

  while (!chThdShouldTerminate()) {
    c = sdGetTimeout((SerialDriver *)sdp, MS2ST(50));
    if (c == Q_TIMEOUT)
      continue;

    if (mavlink_parse_char(MAVLINK_COMM_1, (uint8_t)c, &msg, &status)) {
      if (msg.compid == MAV_COMP_ID_MPIOVD){
        switch(msg.msgid){

        case MAVLINK_MSG_ID_MPIOVD_SENSORS:
          mavlink_msg_mpiovd_sensors_decode(&msg, &mavlink_mpiovd_sensors_struct);
          /* mpiovd has no RTC, so we must set it here */
          mavlink_mpiovd_sensors_struct.time_usec = fastGetTimeUnixUsec();
          chEvtBroadcastFlags(&event_mavlink_mpiovd_sensors, EVMSK_MAVLINK_MPIOVD_SENSORS);
          break;

        case MAVLINK_MSG_ID_HEARTBEAT:
          mavlink_msg_heartbeat_decode(&msg, &mavlink_heartbeat_mpiovd_struct);
          chEvtBroadcastFlags(&event_mavlink_heartbeat_mpiovd, EVMSK_MAVLINK_HEARTBEAT_MPIOVD);
          break;

        default:
          break;
        }
      }
    }
  }
}
