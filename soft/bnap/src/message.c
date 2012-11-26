#include "ch.h"
#include "hal.h"
#include "mavlink.h"

#include "main.h"
#include "message.h"

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
mavlink_heartbeat_t             mavlink_heartbeat_struct;
mavlink_gps_raw_int_t           mavlink_gps_raw_int_struct;
mavlink_global_position_int_t   mavlink_global_position_int_struct;
mavlink_statustext_t            mavlink_statustext_struct;
mavlink_command_long_t          mavlink_command_long_struct;
mavlink_system_time_t           mavlink_system_time_struct;
mavlink_mpiovd_sensors_t        mavlink_mpiovd_sensors_struct;
mavlink_mpiovd_sensors_dbg_t    mavlink_mpiovd_sensors_dbg_struct;
mavlink_param_value_t           mavlink_param_value_struct;
mavlink_param_set_t             mavlink_param_set_struct;
mavlink_param_request_list_t    mavlink_param_request_list_struct;
mavlink_param_request_read_t    mavlink_param_request_read_struct;
mavlink_command_ack_t           mavlink_command_ack_struct;

/**
 * @brief   Event sources.
 */
EventSource event_gps_time_got;
EventSource event_cc_heartbeat;
EventSource event_mpiovd_heartbeat;
EventSource event_dm_heartbeat;

EventSource event_mavlink_gps_raw_int;
EventSource event_mavlink_heartbeat;
EventSource event_mavlink_global_position_int;
EventSource event_mavlink_system_time;
EventSource event_mavlink_sys_status;
EventSource event_mavlink_statustext;
EventSource event_mavlink_mpiovd_sensors;
EventSource event_mavlink_mpiovd_sensors_dbg;
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
  chEvtInit(&event_cc_heartbeat);
  chEvtInit(&event_mpiovd_heartbeat);
  chEvtInit(&event_dm_heartbeat);

  chEvtInit(&event_mavlink_gps_raw_int);
  chEvtInit(&event_mavlink_heartbeat);
  chEvtInit(&event_mavlink_global_position_int);
  chEvtInit(&event_mavlink_system_time);
  chEvtInit(&event_mavlink_sys_status);
  chEvtInit(&event_mavlink_statustext);
  chEvtInit(&event_mavlink_mpiovd_sensors);
  chEvtInit(&event_mavlink_mpiovd_sensors_dbg);
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
  mavlink_system_struct.sysid  = 20;
  mavlink_system_struct.compid = MAV_COMP_ID_ALL;
  mavlink_system_struct.state  = MAV_STATE_ACTIVE;
  mavlink_system_struct.mode   = MAV_MODE_PREFLIGHT;
  mavlink_system_struct.type   = MAV_TYPE_GROUND_ROVER;
}

