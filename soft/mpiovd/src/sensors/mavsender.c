#include <stdlib.h>

#include "ch.h"
#include "hal.h"

#include "mavsender.h"
#include "param.h"
#include "message.h"
#include "main.h"
#include "sanity.h"
#include "sensors.h"
#include "adc_local.h"
#include "freq.h"
#include "storage.h"

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
extern mavlink_mpiovd_sensors_t      mpiovd_sensors_struct;

/*
 ******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************
 */
static const uint32_t *T_tlm;

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
static WORKING_AREA(TLM_SenderThreadWA, 256);
static msg_t TLM_SenderThread(void *arg) {
  chRegSetThreadName("TLM_Sender");
  (void)arg;

  Mail raw_mail    = {NULL, MAVLINK_MSG_ID_MPIOVD_SENSORS, NULL};

  while (TRUE) {
    chThdSleepMilliseconds(*T_tlm);

    adc_process(raw_data.analog, &mpiovd_sensors_struct);

    mpiovd_sensors_struct.time_usec     = 0;
    mpiovd_sensors_struct.relay         = raw_data.discrete;
    mpiovd_sensors_struct.speed         = raw_data.analog[15] / 50;
    mpiovd_sensors_struct.rpm           = get_engine_rpm();
    mpiovd_sensors_struct.engine_uptime = GetUptime();
    mpiovd_sensors_struct.trip          = GetTrip();

    if (*T_tlm != SEND_OFF){
      raw_mail.payload = &mpiovd_sensors_struct;
      chMBPost(&tolink_mb, (msg_t)&raw_mail, TIME_IMMEDIATE);
//      dbg_mail.payload = &mpiovd_sensors_dbg_struct;
//      chMBPost(&tolink_mb, (msg_t)&dbg_mail, TIME_IMMEDIATE);
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


