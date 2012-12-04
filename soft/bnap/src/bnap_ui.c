/*
 * Buttons and leds
 */
#include <string.h>

#include "ch.h"
#include "hal.h"
#include "mavlink.h"

#include "main.h"
#include "bnap_ui.h"
#include "message.h"
#include "mavlink_dbg.h"

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
extern EventSource event_mavlink_heartbeat_cc;
extern EventSource event_gps_time_got;

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

static WORKING_AREA(UiThreadWA, 64);
static msg_t UiThread(void *arg) {
  chRegSetThreadName("Ui");
  (void)arg;
  uint32_t cur = 0;
  uint32_t last = 0;

  while (!chThdShouldTerminate()) {
    chThdSleepMilliseconds(100);

    /* buttons */
    last = cur;
    cur = palReadPad(IOPORT2, PIOB_BTN1);
    if (cur != last){
      if (cur == 0){
        while (palReadPad(IOPORT2, PIOB_BTN1) == 0){
          mavlink_dbg_print(MAV_SEVERITY_ALERT, "Button 'Alert' pressed");
          chThdSleepMilliseconds(4);
        }
      }
      else
        mavlink_dbg_print(MAV_SEVERITY_ALERT, "Button 'Alert' released");
    }
  }
  return 0;
}

/**
 *
 */
static WORKING_AREA(GsmLedThreadWA, 56);
static msg_t GsmLedThread(void *arg) {
  chRegSetThreadName("GsmLed");
  (void)arg;

  struct EventListener el_cc_heartbeat;
  chEvtRegisterMask(&event_mavlink_heartbeat_cc, &el_cc_heartbeat, EVMSK_MAVLINK_HEARTBEAT_CC);

  while (!chThdShouldTerminate()) {
    chEvtWaitOne(EVMSK_MAVLINK_HEARTBEAT_CC);
    gsm_led_on();
    chThdSleepMilliseconds(500);
    gsm_led_off();
  }
  return 0;
}

/**
 *
 */
static WORKING_AREA(GpsLedThreadWA, 56);
static msg_t GpsLedThread(void *arg) {
  chRegSetThreadName("GpsLed");
  (void)arg;

  struct EventListener el_gps_time_got;
  chEvtRegisterMask(&event_gps_time_got, &el_gps_time_got, EVMSK_GPS_TIME_GOT);

  while (!chThdShouldTerminate()) {
    chEvtWaitOne(EVMSK_GPS_TIME_GOT);
    gsm_led_on();
    chThdSleepMilliseconds(50);
    gsm_led_off();
  }
  return 0;
}

/*
 ******************************************************************************
 * EXPORTED FUNCTIONS
 ******************************************************************************
 */

void UiInit(void){
  chThdCreateStatic(UiThreadWA,
          sizeof(UiThreadWA),
          UITREAD_PRIO,
          UiThread,
          NULL);
  chThdCreateStatic(GsmLedThreadWA,
          sizeof(GsmLedThreadWA),
          UITREAD_PRIO,
          GsmLedThread,
          NULL);
  chThdCreateStatic(GpsLedThreadWA,
          sizeof(GpsLedThreadWA),
          UITREAD_PRIO,
          GpsLedThread,
          NULL);
}


