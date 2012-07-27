#include <stdlib.h>

#include "ch.h"
#include "hal.h"

#include "mavsender.h"
#include "param.h"
#include "message.h"
#include "main.h"
#include "sanity.h"
#include "sensors.h"

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
extern Mailbox tolink_mb;
extern RawData raw_data;

/*
 ******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************
 */
static uint32_t *T_tlm;

static mavlink_mpiovd_sensors_raw_t     mpiovd_sensors_raw_struct;
static mavlink_mpiovd_sensors_scaled_t  mpiovd_sensors_scaled_struct;

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

/**
 *
 */
static WORKING_AREA(TLM_SenderThreadWA, 128);
static msg_t TLM_SenderThread(void *arg) {
  chRegSetThreadName("TLM_Sender");
  (void)arg;

  Mail raw_mail    = {NULL, MAVLINK_MSG_ID_MPIOVD_SENSORS_RAW, NULL};
  Mail scaled_mail = {NULL, MAVLINK_MSG_ID_MPIOVD_SENSORS_SCALED, NULL};

  while (TRUE) {
    chThdSleepMilliseconds(50);

    if (raw_mail.payload == NULL){
      mpiovd_sensors_raw_struct.sec = TIME_BOOT_MS;
      mpiovd_sensors_raw_struct.analog01 = raw_data.analog[0];
      mpiovd_sensors_raw_struct.analog02 = raw_data.analog[1];
      mpiovd_sensors_raw_struct.analog03 = raw_data.analog[2];
      mpiovd_sensors_raw_struct.analog04 = raw_data.analog[3];
      mpiovd_sensors_raw_struct.analog05 = raw_data.analog[4];
      mpiovd_sensors_raw_struct.analog06 = raw_data.analog[5];
      mpiovd_sensors_raw_struct.analog07 = raw_data.analog[6];
      mpiovd_sensors_raw_struct.analog08 = raw_data.analog[7];
      mpiovd_sensors_raw_struct.analog09 = raw_data.analog[8];
      mpiovd_sensors_raw_struct.analog10 = raw_data.analog[9];
      mpiovd_sensors_raw_struct.analog11 = raw_data.analog[10];
      mpiovd_sensors_raw_struct.analog12 = raw_data.analog[11];
      mpiovd_sensors_raw_struct.analog13 = raw_data.analog[12];
      mpiovd_sensors_raw_struct.analog14 = raw_data.analog[13];
      mpiovd_sensors_raw_struct.analog15 = raw_data.analog[14];
      mpiovd_sensors_raw_struct.analog16 = raw_data.analog[15];

      mpiovd_sensors_raw_struct.relay  = raw_data.discrete;

      mpiovd_sensors_raw_struct.speed = raw_data.analog[15] / 50;
      mpiovd_sensors_raw_struct.rpm = raw_data.analog[15];
      mpiovd_sensors_raw_struct.engine_uptime = chTimeNow() / 10000;

      raw_mail.payload = &mpiovd_sensors_raw_struct;
      chMBPost(&tolink_mb, (msg_t)&raw_mail, TIME_IMMEDIATE);
    }

    if (scaled_mail.payload == NULL){
      mpiovd_sensors_scaled_struct.sec = TIME_BOOT_MS;
      scaled_mail.payload = &mpiovd_sensors_scaled_struct;
      chMBPost(&tolink_mb, (msg_t)&scaled_mail, TIME_IMMEDIATE);
    }

  }
  return 0;
}

/*
 *******************************************************************************
 * EXPORTED FUNCTIONS
 *******************************************************************************
 */

/**
 *
 */
void MavSenderInit(void){
  T_tlm = ValueSearch("T_tlm");

  chThdCreateStatic(TLM_SenderThreadWA,
          sizeof(TLM_SenderThreadWA),
          LINK_THREADS_PRIO - 1,
          TLM_SenderThread,
          NULL);
}


