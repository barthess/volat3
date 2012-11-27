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
    cur = palReadPad(IOPORT2, 19);
    if (cur != last){
      if (cur == 0)
        mavlink_dbg_print(MAV_SEVERITY_ALERT, "Button 'Alert' pressed");
      else
        mavlink_dbg_print(MAV_SEVERITY_ALERT, "Button 'Alert' released");
    }
  }
  return 0;
}

/**
 *
 */
static WORKING_AREA(GsmLedThreadWA, 64);
static msg_t GsmLedThread(void *arg) {
  chRegSetThreadName("GsmLed");
  (void)arg;

  struct EventListener el_cc_heartbeat;
  chEvtRegisterMask(&event_mavlink_heartbeat_cc, &el_cc_heartbeat, EVMSK_MAVLINK_HEARTBEAT_CC);

  while (!chThdShouldTerminate()) {
    chEvtWaitOne(EVMSK_MAVLINK_HEARTBEAT_CC);
    gsm_led_on();
    chThdSleepMilliseconds(200);
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
          NORMALPRIO - 5,
          UiThread,
          NULL);
  chThdCreateStatic(GsmLedThreadWA,
          sizeof(GsmLedThreadWA),
          NORMALPRIO - 5,
          GsmLedThread,
          NULL);
}


