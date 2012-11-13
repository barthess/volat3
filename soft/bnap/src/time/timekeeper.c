#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "ch.h"
#include "hal.h"

#include "main.h"
#include "message.h"
#include "sanity.h"
#include "sensors.h"
#include "ds1338.h"
#include "timekeeper.h"

#include "mavlink.h"

/*
 ******************************************************************************
 * DEFINES
 ******************************************************************************
 */
/* adjust RTC if time difference between RTC and GPS more than this threshold */
#define MAX_TIME_DIFF         5 // sec

/* wait of PSS interrupt timeout */
#define PPS_TMO               MS2ST(2000)

/* wait untile time be parsed by gps code. Must be less than second */
#define GPS_TIME_TMO          MS2ST(900)

/*
 ******************************************************************************
 * EXTERNS
 ******************************************************************************
 */
extern BinarySemaphore pps_sem;
extern struct tm gps_timp;

extern EventSource event_gps_time_got;
extern EventSource event_system_time;

extern mavlink_system_time_t mavlink_system_time_struct;

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

/**
 * Perform periodic corrections of time.
 */
static WORKING_AREA(TimekeeperThreadWA, 128);
static msg_t TimekeeperThread(void *arg){
  chRegSetThreadName("Timekeeper");
  (void)arg;

  struct EventListener el_gps_time_got;
  chEvtRegisterMask(&event_gps_time_got, &el_gps_time_got, EVMSK_GPS_TIME_GOT);
  eventmask_t evt = 0;

  int64_t  gps_time = 0;
  int64_t  bnap_time = 0;

  while (TRUE) {
    chBSemWait(&pps_sem);
    //TODO: save timestamp now to correct it later

    evt = chEvtWaitOneTimeout(EVMSK_GPS_TIME_GOT, GPS_TIME_TMO);
    if (evt == EVMSK_GPS_TIME_GOT){
      bnap_time = fastGetTimeUnixUsec();

      gps_time = (int64_t)mktime(&gps_timp) * 1000000;
      Correction += gps_time - bnap_time;

      mavlink_system_time_struct.time_boot_ms = TIME_BOOT_MS;
      mavlink_system_time_struct.time_unix_usec = fastGetTimeUnixUsec();
      chEvtBroadcastFlags(&event_system_time, EVMSK_SYSTEM_TIME);

      /* now correct time in RTC cell */
      ds1338_set_time(&gps_timp);
    }
  }
  return 0;
}

/*
 *******************************************************************************
 * EXPORTED FUNCTIONS
 *******************************************************************************
 */

void TimekeeperInit(void){

  BootTimestamp = ds1338GetTimeUnixUsec();
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
uint64_t fastGetTimeUnixUsec(void){

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

