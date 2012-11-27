#include "ch.h"
#include "hal.h"
#include "chprintf.h"

#include "main.h"
#include "cross.h"

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
/**
 *
 */
static WORKING_AREA(CrossFromModemThreadWA, 128);
static msg_t CrossFromModemThread(void *arg) {
  chRegSetThreadName("CrossFromModem");
  (void)arg;
  uint8_t c;

  while (!chThdShouldTerminate()) {
    c = sdGet(&SDGSM);
//    if (c == '\r'){
//      sdPut(&SDDM, '\\');
//      sdPut(&SDDM, 'r');
//    }
//    else if (c == '\n'){
//      sdPut(&SDDM, '\\');
//      sdPut(&SDDM, 'n');
//    }
//    else
      sdPut(&SDDM, c);
  }
  return 0;
}

/**
 *
 */
static WORKING_AREA(CrossToModemThreadWA, 128);
static msg_t CrossToModemThread(void *arg) {
  chRegSetThreadName("CrossToModem");
  (void)arg;
  uint8_t c;

  palClearPad(IOPORT2, PIOB_GSM_RTS);

  while (!chThdShouldTerminate()) {
    c = sdGet(&SDDM);
    sdPut(&SDGSM, c);
  }
  return 0;
}

/*
 ******************************************************************************
 * EXPORTED FUNCTIONS
 ******************************************************************************
 */
/**
 *
 */
void ModemCrossInit(void){
//  chThdSleepMilliseconds(10000);
//  uint8_t st[] = "AT+IPR=9600\r\n";
//  sdWrite(&SDGSM, st, sizeof(st));

  chprintf((BaseSequentialStream *)&SDDM, "%s", "*** Manual modem settings");
  chThdSleepMilliseconds(200);

  chThdCreateStatic(CrossFromModemThreadWA,
          sizeof(CrossFromModemThreadWA),
          NORMALPRIO,
          CrossFromModemThread,
          NULL);
  chThdCreateStatic(CrossToModemThreadWA,
          sizeof(CrossToModemThreadWA),
          NORMALPRIO,
          CrossToModemThread,
          NULL);
}

