#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "ch.h"
#include "hal.h"
#include "chprintf.h"

#include "main.h"
#include "message.h"
#include "sanity.h"
#include "sensors.h"
#include "ds1338.h"
#include "timekeeper.h"
#include "cli.h"
//#include "storage.h"
#include "bnap_ui.h"

#include "mavlink.h"

/*
 ******************************************************************************
 * DEFINES
 ******************************************************************************
 */

#define INTERRUPT_LATENCY_US  19  // 18.8uS for -O0, CH_DBG_ENABLE_CHECKS TRUE
//#define INTERRUPT_LATENCY_US  16  // 16.0uS for -O0, CH_DBG_ENABLE_CHECKS FALSE
//#define INTERRUPT_LATENCY_US  11  // 11.2uS for -O1, CH_DBG_ENABLE_CHECKS TRUE
//#define INTERRUPT_LATENCY_US  9   // 8.8uS  for -O1, CH_DBG_ENABLE_CHECKS FALSE

#define TIME_PERIOD_TICKS     1

/* microseconds between systicks */
#define TIME_INCREMENT_US     ((1000000 / CH_FREQUENCY) * TIME_PERIOD_TICKS)

/* wait of PSS interrupt timeout */
#define PPS_TMO               MS2ST(2000)

/* wait until time be parsed by gps code. Must be less than second */
#define GPS_TIME_TMO          MS2ST(900)

/* Timestamp of project compilation. Usec since Unix epoch.
 * Used to one more check of time correctness. */
#define BUILD_TIMESTAMP       ((int64_t)1352971162164000)

/* string to (re)start virtual timer updating system time */
#define start_timekeeper_vt() {chVTSetI(&timekeeper_vt, TIME_PERIOD_TICKS, &timekeeper_vt_cb, NULL);}

/*
 ******************************************************************************
 * EXTERNS
 ******************************************************************************
 */
extern GlobalFlags_t GlobalFlags;

extern BinarySemaphore pps_sem;
extern struct tm gps_timp;

extern EventSource event_gps_time_got;
extern EventSource event_mavlink_system_time;

extern mavlink_system_time_t mavlink_system_time_struct;
extern mavlink_gps_raw_int_t mavlink_gps_raw_int_struct;

/*
 ******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************
 */

static int64_t SysTimeUsec  = 0;
static int64_t SysTimeShift = 0;
static int64_t PpsTimeStamp = 0;

/**/
static VirtualTimer timekeeper_vt;

/*
 *******************************************************************************
 *******************************************************************************
 * LOCAL FUNCTIONS
 *******************************************************************************
 *******************************************************************************
 */
/*
 * Update system time every tick
 */
static void timekeeper_vt_cb(void *par){
  (void)par;
  chSysLockFromIsr();
  SysTimeUsec += SysTimeShift + TIME_INCREMENT_US;
  SysTimeShift = 0;
  start_timekeeper_vt();
  chSysUnlockFromIsr();
}

/**
 * Perform periodic corrections of time.
 */
static WORKING_AREA(TimekeeperThreadWA, 192);
static msg_t TimekeeperThread(void *arg){
  chRegSetThreadName("Timekeeper");
  (void)arg;
  int64_t  gps_time = 0;

  struct EventListener el_gps_time_got;
  chEvtRegisterMask(&event_gps_time_got, &el_gps_time_got, EVMSK_GPS_TIME_GOT);
  eventmask_t evt = 0;

  while (TRUE) {
    chBSemWait(&pps_sem);

    /* now wait until time been parsed from NMEA message */
    evt = chEvtWaitOneTimeout(EVMSK_GPS_TIME_GOT, GPS_TIME_TMO);

    if (evt == EVMSK_GPS_TIME_GOT){
      gps_time =  1000000;
      gps_time *= mktime(&gps_timp);

      chSysLock();
      SysTimeShift = gps_time - PpsTimeStamp - INTERRUPT_LATENCY_US;
      chSysUnlock();

      if (GlobalFlags.time_good != 1)
        setGlobalFlag(GlobalFlags.time_good);
      if (GlobalFlags.time_proved != 1)
        setGlobalFlag(GlobalFlags.time_proved);

      /**/
      mavlink_gps_raw_int_struct.time_usec = gps_time;
      mavlink_system_time_struct.time_boot_ms = TIME_BOOT_MS;
      if (GlobalFlags.time_proved == 1)
        mavlink_system_time_struct.time_unix_usec = SysTimeUsec;
      else
        mavlink_system_time_struct.time_unix_usec = 0;
      chEvtBroadcastFlags(&event_mavlink_system_time, EVMSK_MAVLINK_SYSTEM_TIME);

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

  SysTimeUsec = ds1338GetTimeUnixUsec();
  if (SysTimeUsec < BUILD_TIMESTAMP)
    clearGlobalFlag(GlobalFlags.time_good);
  else
    setGlobalFlag(GlobalFlags.time_good);

  /* virtual timer for ticker */
  chSysLock();
  start_timekeeper_vt();
  chSysUnlock();

  /**/
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
int64_t fastGetTimeUnixUsec(void){
  return SysTimeUsec;
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

/**
 * Command to handle RTC.
 */
Thread* date_clicmd(int argc, const char * const * argv, SerialDriver *sdp){
  (void)argc;
  (void)argv;

  struct tm timp;
  size_t n = 32;
  char str[n];

  ds1338GetTimeTm(&timp);
  cli_print("Current UTC time is: ");
  strftime(str, n, "%F %H:%M:%S", &timp);
  cli_println(str);

  chprintf((BaseSequentialStream *)sdp, "%d", ds1338GetTimeUnixSec());
  cli_println(" seconds since Unix epoch");

  return NULL; /* stub */
}

/**
 *
 */
void exti_pps_cb(EXTDriver *extp, expchannel_t channel){
  (void)extp;
  (void)channel;

  if (palReadPad(IOPORT2, PIOB_PPS) == 1){
    PpsTimeStamp = SysTimeUsec;
    //gps_led_on();
    chBSemSignalI(&pps_sem);
  }
  else{
    //gps_led_off();
    ;
  }
}
