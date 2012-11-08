

// TODO: add EXTI

#include <time.h>

#include "ch.h"
#include "hal.h"
#include "mavlink.h"

#include "main.h"
#include "sensors.h"
#include "message.h"
#include "timekeeping.h"
#include "sanity.h"
#include "gps.h"
#include "link.h"

/*
 ******************************************************************************
 * EXTERNS
 ******************************************************************************
 */

RawData raw_data;

CompensatedData comp_data;

/* RTC-GPS sync */
BinarySemaphore rtc_sem;

/* store here time from GPS */
struct tm gps_timp;

/* some global flags */
GlobalFlags_t GlobalFlags = {0,0,0,0,0,0,0,0,
                             0,0,0,0,0,0,0,0,
                             0,0,0,0,0,0,0,0,
                             0,0,0,0,0,0,0,0};

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

int main(void) {
  halInit();
  chSysInit();

  chBSemInit(&rtc_sem, TRUE);

  MavInit();
  MsgInit();
  GPSInit();
  LinkInit();
  TimekeepingInit();
  SanityControlInit();


  while (TRUE) {
    chThdSleepMilliseconds(666);
  }
  return 0;
}
