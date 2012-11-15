// TODO: EXTI
// TODO: при каждой записи в хранилище обновлять время последней доступной записи (?? и общее количество??)
// TODO: обработка битых блоков
// TODO: более высокая точность парсинга координат gps.

#include <time.h>

#include "ch.h"
#include "hal.h"
#include "mavlink.h"

#include "main.h"
#include "sensors.h"
#include "message.h"
#include "i2c_local.h"
#include "timekeeper.h"
#include "sanity.h"
#include "gps.h"
#include "link.h"
#include "ds1338.h"
#include "exti_local.h"
#include "storage.h"

/*
 ******************************************************************************
 * EXTERNS
 ******************************************************************************
 */

RawData raw_data;

CompensatedData comp_data;

/* RTC-GPS sync */
BinarySemaphore pps_sem;

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

  chBSemInit(&pps_sem, TRUE);

  i2cLocalInit();

  MavInit();
  MsgInit();
  GPSInit();
  ExtiLocalInit();
  LinkInit();

  ds1338Init();
  TimekeeperInit();

  StorageInit();

  SanityControlInit();

  while (TRUE) {
    chThdSleepMilliseconds(666);
  }
  return 0;
}
