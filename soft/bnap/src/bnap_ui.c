/*
 * Buttons and leds
 */

#include "ch.h"
#include "hal.h"

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

/*
 ******************************************************************************
 * PROTOTYPES
 ******************************************************************************
 */

/*
 ******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************
 */

/*
 ******************************************************************************
 ******************************************************************************
 * LOCAL FUNCTIONS
 ******************************************************************************
 ******************************************************************************
 */

static WORKING_AREA(ButtonThreadWA, 128);
static msg_t SanityControlThread(void *arg) {
  chRegSetThreadName("Sanity");
  (void)arg;

  mavlink_heartbeat_struct.autopilot = MAV_AUTOPILOT_GENERIC;
  mavlink_heartbeat_struct.custom_mode = 0;

  systime_t t = chTimeNow();

  while (TRUE) {
    t += HEART_BEAT_PERIOD;

    mavlink_heartbeat_struct.type           = mavlink_system_struct.type;
    mavlink_heartbeat_struct.base_mode      = mavlink_system_struct.mode;
    mavlink_heartbeat_struct.system_status  = mavlink_system_struct.state;

    mavlink_sys_status_struct.load = get_cpu_load();

    chEvtBroadcastFlags(&event_heartbeat, EVMSK_HEARTBEAT);
    chThdSleepUntil(t);
  }
  return 0;
}

/*
 ******************************************************************************
 * EXPORTED FUNCTIONS
 ******************************************************************************
 */

void BnapUiInit(void){
  return;
}
