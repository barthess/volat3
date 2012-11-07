#include <time.h>
#include <stdio.h>

#include "ch.h"
#include "hal.h"

#include "main.h"
#include "message.h"
#include "sanity.h"
#include "sensors.h"
#include "timekeeping.h"

#include "mavlink.h"

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
extern RawData raw_data;
extern BinarySemaphore rtc_sem;
extern struct tm gps_timp;

/*
 ******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************
 */

/* Boot timestamp (microseconds since UNIX epoch). Inits in boot time. Latter
 * uses to calculate current time using TIME_BOOT_MS. Periodically synced with
 * GPS to correct systick drift */
static int64_t BootTimestamp = 0;

/* Можно вносить поправку прямо в метку времени, но точное время запуска
 * может понадобиться позднее, поэтому заведем отдельную переменную */
static int64_t Correction = 0;

/* количество переполнений системного таймера. Фактически для программного
 * расширения разрядности счетчика на 32 бита */
static uint32_t WrapCount = 0;

/* последнее значение счетчика для отлова момента переполнения */
static uint32_t LastTimeBootMs = 0;

/*
 *******************************************************************************
 *******************************************************************************
 * LOCAL FUNCTIONS
 *******************************************************************************
 *******************************************************************************
 */

static uint64_t rtcGetTimeUnixUsec_bnap(void){
  // TODO: use RTC over I2C here
  return 0;
}

static WORKING_AREA(TimekeeperThreadWA, 512);
static msg_t TimekeeperThread(void *arg){
  chRegSetThreadName("Timekeeper");
  (void)arg;

  int64_t  gps_time = 0;
  int64_t  pns_time = 0;

  msg_t sem_status = RDY_RESET;

  while (TRUE) {
    sem_status = chBSemWaitTimeout(&rtc_sem, MS2ST(2000));
    if (sem_status == RDY_OK && raw_data.gps_valid){
      pns_time = pnsGetTimeUnixUsec();

      gps_time = (int64_t)mktime(&gps_timp) * 1000000;
      Correction += gps_time - pns_time;

      /* now correct time in internal RTC (if needed) */
      // TODO
    }
  }
  return 0;
}

/*
 *******************************************************************************
 * EXPORTED FUNCTIONS
 *******************************************************************************
 */

void TimekeepingInit(void){

  BootTimestamp = rtcGetTimeUnixUsec_bnap();
  /* поскольку вычитывание метки можеть происходить не сразу же после запуска -
   * внесем коррективы */
  BootTimestamp -= (int64_t)TIME_BOOT_MS * 1000;

  chThdCreateStatic(TimekeeperThreadWA,
          sizeof(TimekeeperThreadWA),
          TIMEKEEPER_THREAD_PRIO,
          TimekeeperThread,
          NULL);
}

/**
 * Return current time using lightweight approximation to avoid calling
 * of heavy time conversion (from hardware RTC) functions.
 */
uint64_t pnsGetTimeUnixUsec(void){

  chSysLock();
  if (TIME_BOOT_MS < LastTimeBootMs)
    WrapCount++;

  LastTimeBootMs = TIME_BOOT_MS;
  chSysUnlock();
  return BootTimestamp
         + (int64_t)(TIME_BOOT_MS) * 1000
         + Correction
         + 0x100000000ull * WrapCount;
}

/**
 * @brief   Return number of system ticks since last call.
 * @note    Function modifies the last value itself.
 *
 * @param[in] last      pointer to the value containing last call time
 * @return              The number of ticks.
 *
 * @api
 */
systime_t GetTimeInterval(systime_t *last){
  systime_t t = 0;

  if (chTimeNow() >= *last)
    t = chTimeNow() - *last;
  else /* overflow happens */
    t = chTimeNow() + (0xFFFFFFFF - *last);
  /* refresh last value */
  *last = chTimeNow();
  return t;
}

