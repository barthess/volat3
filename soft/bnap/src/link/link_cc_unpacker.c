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

extern mavlink_system_t                mavlink_system_struct;

extern mavlink_command_long_t          mavlink_command_long_struct;
extern mavlink_param_set_t             mavlink_param_set_struct;
extern mavlink_param_request_list_t    mavlink_param_request_list_struct;
extern mavlink_param_request_read_t    mavlink_param_request_read_struct;
extern mavlink_heartbeat_t             mavlink_heartbeat_cc_struct;

extern EventSource event_mavlink_command_long;
extern EventSource event_mavlink_param_set;
extern EventSource event_mavlink_param_request_list;
extern EventSource event_mavlink_param_request_read;
extern EventSource event_mavlink_heartbeat_cc;

/*
 *******************************************************************************
 * EXPORTED FUNCTIONS
 *******************************************************************************
 */


/**
 *
 */
void CcUnpackCycle(SerialDriver *sdp){
  mavlink_message_t msg;
  mavlink_status_t status;
  msg_t c = 0;
  msg_t prev_c = 0;

  while(GlobalFlags.messaging_ready == 0)
    chThdSleepMilliseconds(50);

  while (!chThdShouldTerminate()) {
    /* Try to get an escaped with DLE symbols message */
    c = sdGet((SerialDriver *)sdp);
    prev_c = c;
    if (prev_c == DLE){
      prev_c = 0; /* set it to any just not DLE nor ETX */
      c = sdGet((SerialDriver *)sdp);
    }

    if (mavlink_parse_char(MAVLINK_COMM_0, (uint8_t)c, &msg, &status)) {
      if (msg.sysid == GROUND_STATION_ID){ /* нас запрашивает наземная станция */
        switch(msg.msgid){
        case MAVLINK_MSG_ID_COMMAND_LONG:
          mavlink_msg_command_long_decode(&msg, &mavlink_command_long_struct);
          if (mavlink_command_long_struct.target_system == mavlink_system_struct.sysid)
            chEvtBroadcastFlags(&event_mavlink_command_long, EVMSK_MAVLINK_COMMAND_LONG);
          break;

        case MAVLINK_MSG_ID_PARAM_SET:
          mavlink_msg_param_set_decode(&msg, &mavlink_param_set_struct);
          if (mavlink_param_set_struct.target_system == mavlink_system_struct.sysid)
            chEvtBroadcastFlags(&event_mavlink_param_set, EVMSK_MAVLINK_PARAM_SET);
          break;

        case MAVLINK_MSG_ID_PARAM_REQUEST_LIST:
          mavlink_msg_param_request_list_decode(&msg, &mavlink_param_request_list_struct);
          if (mavlink_param_request_list_struct.target_system == mavlink_system_struct.sysid)
            chEvtBroadcastFlags(&event_mavlink_param_request_list, EVMSK_MAVLINK_PARAM_REQUEST_LIST);
          break;

        case MAVLINK_MSG_ID_PARAM_REQUEST_READ:
          mavlink_msg_param_request_read_decode(&msg, &mavlink_param_request_read_struct);
          if (mavlink_param_request_read_struct.target_system == mavlink_system_struct.sysid)
            chEvtBroadcastFlags(&event_mavlink_param_request_read, EVMSK_MAVLINK_PARAM_REQUEST_READ);
          break;

        case MAVLINK_MSG_ID_HEARTBEAT:
          mavlink_msg_heartbeat_decode(&msg, &mavlink_heartbeat_cc_struct);
          chEvtBroadcastFlags(&event_mavlink_heartbeat_cc, EVMSK_MAVLINK_HEARTBEAT_CC);
          break;

        default:
          break;
        }
      }
    }
  }
}
