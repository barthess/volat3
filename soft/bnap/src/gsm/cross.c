#include "ch.h"
#include "hal.h"

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
    sdPut(&SDDM, c);
//    sdPut(&SDDM, sdGet(&SDGSM));
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
//    sdPut(&SDGSM, sdGet(&SDDM));
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

