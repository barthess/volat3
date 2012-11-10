#include "link_dm_packer.h"

/*
 ******************************************************************************
 * EXTERNS
 ******************************************************************************
 */
extern mavlink_system_t mavlink_system_struct;

extern mavlink_gps_raw_int_t mavlink_gps_raw_int_struct;
extern mavlink_heartbeat_t mavlink_heartbeat_struct;

extern EventSource event_gps_raw_int;
extern EventSource event_heartbeat;

/*
 *******************************************************************************
 * EXPORTED FUNCTIONS
 *******************************************************************************
 */

/**
 * param[in]  sdp pointer to associated serial driver
 */
void DmPackCycle(SerialDriver *sdp){

  struct EventListener el_gps_raw_int;
  struct EventListener el_heartbeat;
  chEvtRegisterMask(&event_gps_raw_int, &el_gps_raw_int, EVMSK_GPS_RAW_INT);
  chEvtRegisterMask(&event_heartbeat, &el_heartbeat, EVMSK_HERTBEAT);

  eventmask_t evt = 0;
  mavlink_message_t mavlink_message_struct;
  uint8_t sendbuf[MAVLINK_MAX_PACKET_LEN];
  uint16_t len = 0;

  while (!chThdShouldTerminate()) {
    evt = chEvtWaitOne(EVMSK_GPS_RAW_INT | EVMSK_HERTBEAT);

    switch(evt){
    case EVMSK_GPS_RAW_INT:
      do{
        memcpy(sendbuf, &mavlink_gps_raw_int_struct, sizeof(mavlink_gps_raw_int_struct));
      }while (0 != memcmp(sendbuf, &mavlink_gps_raw_int_struct, sizeof(mavlink_gps_raw_int_struct)));
      mavlink_msg_gps_raw_int_encode(mavlink_system_struct.sysid, MAV_COMP_ID_GPS, &mavlink_message_struct, (mavlink_gps_raw_int_t *)sendbuf);
      break;

    case EVMSK_HERTBEAT:
      do{
        memcpy(sendbuf, &mavlink_heartbeat_struct, sizeof(mavlink_heartbeat_struct));
      }while (0 != memcmp(sendbuf, &mavlink_heartbeat_struct, sizeof(mavlink_heartbeat_struct)));
      mavlink_msg_heartbeat_encode(mavlink_system_struct.sysid, MAV_COMP_ID_ALL, &mavlink_message_struct, (mavlink_heartbeat_t *)sendbuf);
      break;

    default:
      break;
    }
    /* write in port after exiting from case */
    len = mavlink_msg_to_send_buffer(sendbuf, &mavlink_message_struct);
    sdWrite(sdp, sendbuf, len);
  }
}

