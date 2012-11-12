#include <time.h>

#include "ch.h"
#include "hal.h"

#include "main.h"

/*
 ******************************************************************************
 * DEFINES
 ******************************************************************************
 */
#define DS1338_SECONDS    0x00
#define DS1338_MINUTES    0x01
#define DS1338_HOURS      0x02
#define DS1338_DAY        0x03
#define DS1338_DATE       0x04
#define DS1338_MONTH      0x05
#define DS1338_YEAR       0x06

#define DS1338_TIME       DS1338_SECONDS
#define DS1338_TIME_SIZE  DS1338_YEAR - DS1338_SECONDS + 1

#define DS1338_CONTROL    0x07
#define DS1338_NVRAM      0x08
#define DS1338_NVRAM_SIZE (8 * 56)

/*
 ******************************************************************************
 * EXTERNS
 ******************************************************************************
 */

/* RTC-GPS sync */
extern BinarySemaphore rtc_sem;

/* store here time from GPS */
extern struct tm gps_timp;

/* some global flags */
extern GlobalFlags_t GlobalFlags;

/*
 ******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************
 */
static uint8_t rxbuf[DS1338_TIME_SIZE];
static uint8_t txbuf[DS1338_TIME_SIZE];
static i2caddr_t ds1338addr = 0b1101000;

/*
 *******************************************************************************
 *******************************************************************************
 * LOCAL FUNCTIONS
 *******************************************************************************
 *******************************************************************************
 */

/**
 *
 */
void ds1338_get_time(void){

}

void ds1338Init(void) {
  txbuf[0] = DS1338_CONTROL;
  i2cMasterTransmitTimeout(&I2CD1, ds1338addr, txbuf, 1, rxbuf, 0, TIME_INFINITE);
  i2cMasterReceiveTimeout(&I2CD1, ds1338addr, rxbuf, 1, TIME_INFINITE);
  /* oscillator stop flag */
  if (rxbuf[0] & 0b10000){
    txbuf[0] = 0;
    txbuf[1] = 0;
    /* set CH=0 */
    i2cMasterTransmitTimeout(&I2CD1, ds1338addr, txbuf, 2, rxbuf, 0, TIME_INFINITE);
    /* clear OSF flag */
    txbuf[0] = DS1338_CONTROL;
    txbuf[1] = 0;
    i2cMasterTransmitTimeout(&I2CD1, ds1338addr, txbuf, 2, rxbuf, 0, TIME_INFINITE); /* set pointer */
  }
}





