#include "ch.h"
#include "hal.h"

#include "mavlink.h"
#include "mpiovd.h" /* MAV_CMD enum here */

#include "link.h"
#include "message.h"
#include "main.h"
#include "param_persistant.h"
#include "utils.h"

/*
 ******************************************************************************
 * DEFINES
 ******************************************************************************
 */
#define CONFIRM_TMO           MS2ST(500)

/*
 ******************************************************************************
 * EXTERNS
 ******************************************************************************
 */
extern GlobalFlags_t GlobalFlags;

extern mavlink_system_t mavlink_system_struct;
extern mavlink_command_long_t mavlink_command_long_struct;
extern mavlink_command_ack_t mavlink_command_ack_struct;

extern EventSource event_mavlink_command_long;
extern EventSource event_mavlink_command_ack;

/*
 ******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************
 */

/*
 *******************************************************************************
 *******************************************************************************
 * LOCAL FUNCTIONS
 *******************************************************************************
 *******************************************************************************
 */

/**
 *
 */
static void cmd_confirm(enum MAV_RESULT result, enum MAV_CMD cmd){
  mavlink_command_ack_struct.result = result;
  mavlink_command_ack_struct.command = cmd;
  chEvtBroadcastFlags(&event_mavlink_command_ack, EVMSK_MAVLINK_COMMAND_ACK);
}

/**
 *
 */
static enum MAV_RESULT cmd_preflight_storage_handler(mavlink_command_long_t *p){
  bool_t status = CH_FAILED;

  if (mavlink_system_struct.mode != MAV_MODE_PREFLIGHT)
    return MAV_RESULT_TEMPORARILY_REJECTED;

  if (p->param1 == 0)
    status = load_params_from_eeprom();
  else if (p->param1 == 1)
    status = save_all_params_to_eeprom();

  if (status != CH_SUCCESS)
    return MAV_RESULT_FAILED;
  else
    return MAV_RESULT_ACCEPTED;
}

/**
 * parsing and execution of commands from ground
 */
static void process_command_long(mavlink_command_long_t *p){
  enum MAV_RESULT result = MAV_RESULT_DENIED;

  /* all this flags defined in MAV_CMD enum */
  switch(p->command){

  case MAV_CMD_PREFLIGHT_STORAGE:
    /* Data (storing to)/(loding from) EEPROM */
    result = cmd_preflight_storage_handler(p);
    cmd_confirm(result, p->command);
    break;

  default:
    cmd_confirm(MAV_RESULT_UNSUPPORTED, p->command);
    break;
  }
}

/**
 * Command processing thread.
 */
static WORKING_AREA(CmdThreadWA, 512);
static msg_t CmdThread(void* arg){
  chRegSetThreadName("MAVCommand");
  (void)arg;
  mavlink_command_long_t p;

  while(GlobalFlags.messaging_ready == 0)
    chThdSleepMilliseconds(50);

  struct EventListener el_command_long;
  chEvtRegisterMask(&event_mavlink_command_long, &el_command_long, EVMSK_MAVLINK_COMMAND_LONG);

  while (!chThdShouldTerminate()) {
    chEvtWaitOne(EVMSK_MAVLINK_COMMAND_LONG);
    if (CH_SUCCESS == memcpy_ts(&p, &mavlink_command_long_struct, sizeof(p), 4))
      process_command_long(&p);
  }
  return 0;
}

/*
 *******************************************************************************
 * EXPORTED FUNCTIONS
 *******************************************************************************
 */
void MavCmdInitLocal(void){

  chThdCreateStatic(CmdThreadWA,
        sizeof(CmdThreadWA),
        NORMALPRIO,
        CmdThread,
        NULL);
}
