// TODO: init functions for packers and unpackers loading parameters (timeouts)
// TODO: different macro wrappers for serial writers (in case of UDP)
// TODO: traffic shaper intergration

// TODO: EXTI
// TODO: при каждой записи в хранилище обновлять время последней доступной записи (in RAM) (?? и общее количество??)
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
#include "wavecom.h"
#include "cross.h"

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
/**/
static const SerialConfig gsm_ser_cfg = {
    GSM_BAUDRATE,
    //9600,
    AT91C_US_USMODE_HWHSH | AT91C_US_CLKS_CLOCK | AT91C_US_CHRL_8_BITS |
                              AT91C_US_PAR_NONE | AT91C_US_NBSTOP_1_BIT
//      AT91C_US_USMODE_NORMAL | AT91C_US_CLKS_CLOCK | AT91C_US_CHRL_8_BITS |
//                                AT91C_US_PAR_NONE | AT91C_US_NBSTOP_1_BIT
};
static const SerialConfig dm_ser_cfg = {
    DM_BAUDRATE,
    //921600,
    AT91C_US_USMODE_NORMAL | AT91C_US_CLKS_CLOCK | AT91C_US_CHRL_8_BITS |
                              AT91C_US_PAR_NONE | AT91C_US_NBSTOP_1_BIT
};
static const SerialConfig mpiovd_ser_cfg = {
    MPIOVD_BAUDRATE,
    AT91C_US_USMODE_NORMAL | AT91C_US_CLKS_CLOCK | AT91C_US_CHRL_8_BITS |
                              AT91C_US_PAR_NONE | AT91C_US_NBSTOP_1_BIT
};

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

  sdStart(&SDGSM, &gsm_ser_cfg);
  sdStart(&SDDM, &dm_ser_cfg);
  sdStart(&SDMPIOVD, &mpiovd_ser_cfg);

  i2cLocalInit();

  MavInit();
  MsgInit();
  GPSInit();
//  ExtiLocalInit();
  LinkInit();

  ds1338Init();
  TimekeeperInit();
//  StorageInit();
  SanityControlInit();
  ModemInit();

  while (TRUE) {
    chThdSleepMilliseconds(666);
  }
  return 0;
}
