#include "ch.h"
#include "hal.h"

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

/* ������ �������� ����� */
Mailbox tolink_mb;                /* ��������� ��� �������� ����� ����� */
Mailbox mavlink_param_set_mb;     /* ��������� � ����������� */
Mailbox mavlink_command_long_mb;  /* ��������� � ��������� */

/* ����������, ���������� �������� */
mavlink_system_t              mavlink_system_struct;
mavlink_command_long_t        mavlink_command_long_struct;
mavlink_set_mode_t            mavlink_set_mode_struct;

/*
 ******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************
 */
/* ������ ��� �������� ������ */
static msg_t tolink_mb_buf[10];
static msg_t param_mb_buf[2];
static msg_t mavlinkcmd_mb_buf[2];

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

void MsgInit(void){
  /* ������������� �������� ������ */
  chMBInit(&tolink_mb,                tolink_mb_buf,          (sizeof(tolink_mb_buf)/sizeof(msg_t)));
  chMBInit(&mavlink_param_set_mb,     param_mb_buf,           (sizeof(param_mb_buf)/sizeof(msg_t)));
  chMBInit(&mavlink_command_long_mb,  mavlinkcmd_mb_buf,      (sizeof(mavlinkcmd_mb_buf)/sizeof(msg_t)));
}

void MavInit(void){
  /* �������������� ��������� �������� */
  mavlink_system_struct.sysid  = 20;                   ///< ID 20 for this airplane
  mavlink_system_struct.compid = MAV_COMP_ID_ALL;     ///< The component sending the message, it could be also a Linux process
  mavlink_system_struct.state  = MAV_STATE_BOOT;
  mavlink_system_struct.mode   = MAV_MODE_PREFLIGHT;
}

