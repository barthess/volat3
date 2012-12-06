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
#include "param.h"

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
extern EventSource event_mavlink_gps_raw_int;
extern GlobalFlags_t GlobalFlags;

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
static const uint32_t *blink_gps = NULL;
static const uint32_t *blink_gsm = NULL;

/*
 ******************************************************************************
 ******************************************************************************
 * LOCAL FUNCTIONS
 ******************************************************************************
 ******************************************************************************
 */

static WORKING_AREA(UiAlertBtnThreadWA, 56);
static msg_t UiAlertBtnThread(void *arg) {
  chRegSetThreadName("UiAlertBtn");
  (void)arg;

  /* set all state similar to avoid false button event during startup */
  uint32_t cur  = palReadPad(IOPORT2, PIOB_BTN1);
  uint32_t last = palReadPad(IOPORT2, PIOB_BTN1);

  while (!chThdShouldTerminate()) {
    chThdSleepMilliseconds(100);

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
    if (*blink_gsm != 0){
      gsm_led_on();
      chThdSleepMilliseconds(*blink_gsm);
      gsm_led_off();
    }
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

  struct EventListener el_gps_raw_int;
  chEvtRegisterMask(&event_mavlink_gps_raw_int, &el_gps_raw_int, EVMSK_MAVLINK_GPS_RAW_INT);

  while (!chThdShouldTerminate()) {
    chEvtWaitOne(EVMSK_MAVLINK_GPS_RAW_INT);
    if ((*blink_gps != 0) && (GlobalFlags.gps_valid == 1)){
      gps_led_on();
      chThdSleepMilliseconds(*blink_gps);
      gps_led_off();
    }
  }

  chEvtUnregister(&event_mavlink_gps_raw_int, &el_gps_raw_int);
  return 0;
}

/*
 ******************************************************************************
 * EXPORTED FUNCTIONS
 ******************************************************************************
 */

void UiInit(void){
  blink_gps = ValueSearch("Tblink_gps");
  blink_gsm = ValueSearch("Tblink_gsm");

  chThdCreateStatic(UiAlertBtnThreadWA,
          sizeof(UiAlertBtnThreadWA),
          UITREAD_PRIO,
          UiAlertBtnThread,
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


