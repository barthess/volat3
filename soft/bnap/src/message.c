#include "ch.h"
#include "hal.h"
#include "mavlink.h"

#include "main.h"
#include "message.h"
#include "param.h"

/*
 ******************************************************************************
 * DEFINES
 ******************************************************************************
 */

/*
 ******************************************************************************
 * EXTERNS
 ******************************************************************************
 */
extern GlobalFlags_t GlobalFlags;

/* variable for storing system state */
mavlink_system_t                mavlink_system_struct;

/* mavlink messages */
mavlink_sys_status_t            mavlink_sys_status_struct;
mavlink_gps_raw_int_t           mavlink_gps_raw_int_struct;
mavlink_global_position_int_t   mavlink_global_position_int_struct;
mavlink_statustext_t            mavlink_statustext_struct;
mavlink_command_long_t          mavlink_command_long_struct;
mavlink_system_time_t           mavlink_system_time_struct;
mavlink_param_value_t           mavlink_param_value_struct;
mavlink_param_set_t             mavlink_param_set_struct;
mavlink_param_request_list_t    mavlink_param_request_list_struct;
mavlink_param_request_read_t    mavlink_param_request_read_struct;
mavlink_command_ack_t           mavlink_command_ack_struct;

mavlink_mpiovd_sensors_t        mavlink_mpiovd_sensors_struct;
mavlink_oblique_agps_t          mavlink_oblique_agps_struct;
mavlink_oblique_rssi_t          mavlink_oblique_rssi_struct;
mavlink_oblique_rssi_t          mavlink_oblique_rssi_struct;

/* storage request handlers */
mavlink_oblique_storage_count_t           mavlink_oblique_storage_count_struct;
mavlink_oblique_storage_request_count_t   mavlink_oblique_storage_request_count_cc_struct;
mavlink_oblique_storage_request_t         mavlink_oblique_storage_request_cc_struct;
mavlink_oblique_storage_request_count_t   mavlink_oblique_storage_request_count_dm_struct;
mavlink_oblique_storage_request_t         mavlink_oblique_storage_request_dm_struct;

/* heartbeats from all components of network */
mavlink_heartbeat_t             mavlink_heartbeat_bnap_struct;
mavlink_heartbeat_t             mavlink_heartbeat_mpiovd_struct;
mavlink_heartbeat_t             mavlink_heartbeat_dm_struct;
mavlink_heartbeat_t             mavlink_heartbeat_cc_struct;

/**
 * @brief   Event sources.
 */
EventSource event_gps_time_got;

EventSource event_mavlink_heartbeat_cc;
EventSource event_mavlink_heartbeat_mpiovd;
EventSource event_mavlink_heartbeat_dm;
EventSource event_mavlink_heartbeat_bnap;

EventSource event_mavlink_oblique_rssi;
EventSource event_mavlink_oblique_agps;
EventSource event_mavlink_mpiovd_sensors;

EventSource event_mavlink_oblique_storage_count;
EventSource event_mavlink_oblique_storage_request_count_cc;
EventSource event_mavlink_oblique_storage_request_cc;
EventSource event_mavlink_oblique_storage_request_count_dm;
EventSource event_mavlink_oblique_storage_request_dm;

EventSource event_mavlink_gps_raw_int;
EventSource event_mavlink_global_position_int;
EventSource event_mavlink_system_time;
EventSource event_mavlink_sys_status;
EventSource event_mavlink_statustext;
EventSource event_mavlink_command_long;
EventSource event_mavlink_param_value;
EventSource event_mavlink_param_set;
EventSource event_mavlink_param_request_list;
EventSource event_mavlink_param_request_read;
EventSource event_mavlink_command_ack;

/*
 ******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************
 */

/*
 ******************************************************************************
 * PROTOTYPES
 ******************************************************************************
 */

/*
 *******************************************************************************
 *******************************************************************************
 * LOCAL FUNCTIONS
 *******************************************************************************
 *******************************************************************************
 */

/*
 *******************************************************************************
 * EXPORTED FUNCTIONS
 *******************************************************************************
 */

/**
 * Signal processing thread about end of data processing
 */
void ReleaseMail(Mail* mailp){
  mailp->payload = NULL;
  if (mailp->semp != NULL)
    chBSemSignal(mailp->semp);
}

/**
 *
 */
void MsgInit(void){
  chEvtInit(&event_gps_time_got);

  chEvtInit(&event_mavlink_heartbeat_cc);
  chEvtInit(&event_mavlink_heartbeat_mpiovd);
  chEvtInit(&event_mavlink_heartbeat_dm);
  chEvtInit(&event_mavlink_heartbeat_bnap);

  chEvtInit(&event_mavlink_mpiovd_sensors);
  chEvtInit(&event_mavlink_oblique_agps);
  chEvtInit(&event_mavlink_oblique_rssi);

  chEvtInit(&event_mavlink_oblique_storage_count);
  chEvtInit(&event_mavlink_oblique_storage_request_count_cc);
  chEvtInit(&event_mavlink_oblique_storage_request_cc);
  chEvtInit(&event_mavlink_oblique_storage_request_count_dm);
  chEvtInit(&event_mavlink_oblique_storage_request_dm);

  chEvtInit(&event_mavlink_gps_raw_int);
  chEvtInit(&event_mavlink_global_position_int);
  chEvtInit(&event_mavlink_system_time);
  chEvtInit(&event_mavlink_sys_status);
  chEvtInit(&event_mavlink_statustext);
  chEvtInit(&event_mavlink_command_long);
  chEvtInit(&event_mavlink_param_value);
  chEvtInit(&event_mavlink_param_set);
  chEvtInit(&event_mavlink_param_request_list);
  chEvtInit(&event_mavlink_param_request_read);
  chEvtInit(&event_mavlink_command_ack);

  setGlobalFlag(GlobalFlags.messaging_ready);
}

/**
 *
 */
void MavInit(void){
  /* initial mavlink values */
  mavlink_system_struct.sysid  = *(uint8_t *)ValueSearch("SYS_ID");
  mavlink_system_struct.compid = MAV_COMP_ID_SYSTEM_CONTROL;//MAV_COMP_ID_BNAP;
  mavlink_system_struct.state  = MAV_STATE_ACTIVE;
  mavlink_system_struct.mode   = MAV_MODE_PREFLIGHT;
  mavlink_system_struct.type   = MAV_TYPE_GROUND_ROVER;
}

