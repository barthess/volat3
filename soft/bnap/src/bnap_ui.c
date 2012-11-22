/*
 * Buttons and leds
 */
#include <string.h>

#include "ch.h"
#include "hal.h"

#include "mavlink.h"

#include "main.h"
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

static WORKING_AREA(UiThreadWA, 128);
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
      if (cur == 0){
        mavlink_dbg_print(MAV_SEVERITY_ALERT, "Button 'Alert' pressed");
        palSetPad(IOPORT2, 13);
      }
      else{
        mavlink_dbg_print(MAV_SEVERITY_ALERT, "Button 'Alert' released");
        palClearPad(IOPORT2, 13);
      }
    }
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
}


