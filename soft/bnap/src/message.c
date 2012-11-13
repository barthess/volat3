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
/* control center */
Mailbox tocc_mb;
/* display module */
Mailbox todm_mb;

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

/**
 * @brief   Event sources.
 */
EventSource event_gps_raw_int;
EventSource event_heartbeat;
EventSource event_global_position_int;

EventSource event_gps_time_got;

/*
 ******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************
 */
/* mailbox buffers */
static msg_t tocc_mb_buf[4];
static msg_t todm_mb_buf[4];

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

  chEvtInit(&event_gps_raw_int);
  chEvtInit(&event_gps_time_got);
  chEvtInit(&event_heartbeat);
  chEvtInit(&event_global_position_int);

  chMBInit(&tocc_mb,
      tocc_mb_buf,
      (sizeof(tocc_mb_buf)/sizeof(msg_t)));
  chMBInit(&todm_mb,
      todm_mb_buf,
      (sizeof(todm_mb_buf)/sizeof(msg_t)));
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

