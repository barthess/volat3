#include "ch.h"
#include "hal.h"

#include "main.h"
#include "i2c_local.h"

/*
 ******************************************************************************
 * DEFINES
 ******************************************************************************
 */
//#define CLDIV 16
//#define CHDIV 9
//#define CKDIV 3
#define CLDIV 16
#define CHDIV 9
#define CKDIV 4
#define CWGR  ((CKDIV << 16) | (CHDIV << 8) | (CLDIV << 0))

/*
 ******************************************************************************
 * EXTERNS
 ******************************************************************************
 */

extern GlobalFlags_t GlobalFlags;

/*
 ******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************
 */
static const I2CConfig i2ccfg = {CWGR};

/*
 *******************************************************************************
 *******************************************************************************
 * LOCAL FUNCTIONS
 *******************************************************************************
 *******************************************************************************
 */

void i2cLocalInit(void) {
  i2cStart(&I2CD1, &i2ccfg);
  setGlobalFlag(GlobalFlags.i2c_ready);
//  uint8_t txbuf[6] = {0, 0, 0x55, 7, 8, 9};
//  uint8_t rxbuf[4] = {0, 0, 0, 0};
//
//  while (TRUE){
//    i2cMasterTransmitTimeout(&I2CD1, 0b1010000, txbuf, 2, rxbuf, 4, TIME_INFINITE);
//    //i2cMasterTransmitTimeout(&I2CD1, 0b1010000, txbuf, 6, NULL, 0, TIME_INFINITE);
//    chThdSleepMilliseconds(100);
//  }
}





