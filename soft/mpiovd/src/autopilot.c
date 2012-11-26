#include "mavlink.h"

#include "main.h"
#include "autopilot.h"
#include "message.h"
#include "eeprom.h"
#include "persistant.h"

/*
 ******************************************************************************
 * EXTERNS
 ******************************************************************************
 */
extern Mailbox mavlink_command_long_mb;
extern Mailbox tolink_mb;

extern mavlink_system_t mavlink_system_struct;

/*
 ******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************
 */
mavlink_command_ack_t mavlink_command_ack_struct;
Mail command_ack_mail = {NULL, MAVLINK_MSG_ID_COMMAND_ACK, NULL};

/*
 *******************************************************************************
 *******************************************************************************
 * LOCAL FUNCTIONS
 *******************************************************************************
 *******************************************************************************
 */

void handle_calibration_cmd(mavlink_command_long_t *mavlink_command_long_struct){
  (void)mavlink_command_long_struct;
}


/* helper funcion */
void confirmation(enum MAV_RESULT result, enum MAV_CMD cmd){
  mavlink_command_ack_struct.result = result;
  mavlink_command_ack_struct.command = cmd;
  command_ack_mail.payload = &mavlink_command_ack_struct;
  chMBPostAhead(&tolink_mb, (msg_t)&command_ack_mail, TIME_IMMEDIATE);
}
/* helper macros */
#define command_accepted() (confirmation(MAV_RESULT_ACCEPTED, mavlink_command_long_struct->command))
#define command_denied() (confirmation(MAV_RESULT_DENIED, mavlink_command_long_struct->command))


/* прием и обработка комманд с земли*/
void process_cmd(mavlink_command_long_t *mavlink_command_long_struct){

  /* all this flags defined in MAV_CMD enum */
  switch(mavlink_command_long_struct->command){
  case MAV_CMD_DO_SET_MODE:
    /* Set system mode. |Mode, as defined by ENUM MAV_MODE| Empty| Empty| Empty| Empty| Empty| Empty|  */
    mavlink_system_struct.mode = mavlink_command_long_struct->param1;
    command_accepted();
    break;

  /*
   * (пере)запуск калибровки
   */
  case MAV_CMD_PREFLIGHT_CALIBRATION:
    command_denied();
    return;
    break;

  case MAV_CMD_PREFLIGHT_REBOOT_SHUTDOWN:
    command_denied();
    return;
    break;

    /* Команды для загрузки/вычитки параметров из EEPROM */
    case MAV_CMD_PREFLIGHT_STORAGE:
      if (mavlink_system_struct.mode != MAV_MODE_PREFLIGHT)
        return;

      if (mavlink_command_long_struct->param1 == 0)
        load_params_from_eeprom();
      else if (mavlink_command_long_struct->param1 == 1)
        save_params_to_eeprom();

      break;

  default:
    return;
    break;
  }
}


/**
 * Поток, принимающий и обрабатывающий команды с земли.
 */
static WORKING_AREA(CmdThreadWA, 512);
static msg_t CmdThread(void* arg){
  chRegSetThreadName("CMD_excutor");
  (void)arg;
  msg_t tmp = 0;
  msg_t status = 0;
  Mail *input_mail = NULL;

  while (TRUE) {
    status = chMBFetch(&mavlink_command_long_mb, &tmp, TIME_INFINITE);
    (void)status;
    input_mail = (Mail*)tmp;
    process_cmd((mavlink_command_long_t *)input_mail->payload);
    input_mail->payload = NULL;
  }

  return 0;
}



/*
 *******************************************************************************
 * EXPORTED FUNCTIONS
 *******************************************************************************
 */
void AutopilotInit(void){

  chThdCreateStatic(CmdThreadWA,
        sizeof(CmdThreadWA),
        NORMALPRIO,
        CmdThread,
        NULL);
}
