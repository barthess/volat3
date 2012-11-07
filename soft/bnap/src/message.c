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
Mailbox tolinkcc_mb; /* control center */
Mailbox tolinkdm_mb; /* display module */

/* variable for storing system state */
mavlink_system_t                mavlink_system_struct;

/* mavlink messages */
mavlink_status_t                mavlink_status_struct;
mavlink_sys_status_t            mavlink_sys_status_struct;
mavlink_global_position_int_t   mavlink_global_position_int_struct;
mavlink_heartbeat_t             mavlink_heartbeat_struct;
mavlink_param_value_t           mavlink_param_value_struct;
mavlink_gps_raw_int_t           mavlink_gps_raw_int_struct;
mavlink_statustext_t            mavlink_statustext_struct;

/*
 ******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************
 */
/* mailbox buffers */
static msg_t tolinkcc_mb_buf[8];
static msg_t tolinkdm_mb_buf[8];

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
  chMBInit(&tolinkcc_mb,
      tolinkcc_mb_buf,
      (sizeof(tolinkcc_mb_buf)/sizeof(msg_t)));
  chMBInit(&tolinkdm_mb,
      tolinkdm_mb_buf,
      (sizeof(tolinkdm_mb_buf)/sizeof(msg_t)));
}

/**
 *
 */
void MavInit(void){
  /* initial mavlink values */
  mavlink_system_struct.sysid  = 20;
  mavlink_system_struct.compid = MAV_COMP_ID_ALL;
  mavlink_system_struct.state  = MAV_STATE_BOOT;
  mavlink_system_struct.mode   = MAV_MODE_PREFLIGHT;
  mavlink_system_struct.type   = MAV_TYPE_GROUND_ROVER;
}

