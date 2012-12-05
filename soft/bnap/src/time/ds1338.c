#include <time.h>
#include <string.h>

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

/* some global flags */
extern GlobalFlags_t GlobalFlags;

/*
 ******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************
 */
static uint8_t rxbuf[DS1338_TIME_SIZE];
static uint8_t txbuf[DS1338_TIME_SIZE + 1]; /* +1 for address byte */
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
static void ds1338_tm2bcd(struct tm *timp, uint8_t *bcd){

  bcd[0]  =  timp->tm_sec  % 10;         /// Единицы секунд
  bcd[0] |= (timp->tm_sec  / 10) << 4;   /// Десятки секунд

  bcd[1]  =  timp->tm_min  % 10;         /// Единицы минут
  bcd[1] |= (timp->tm_min  / 10) << 4;   /// Десятки минут

  bcd[2]  =  timp->tm_hour % 10;         /// Единицы часов
  bcd[2] |= (timp->tm_hour / 10) << 4;   /// Десятки часов

  bcd[3]  =  timp->tm_wday + 1;

  bcd[4]  =  timp->tm_mday % 10;         /// Единицы дней
  bcd[4] |= (timp->tm_mday / 10) << 4;   /// Десятки дней

  timp->tm_mon += 1;                     /// Согласование форматов  RTC1338 и структуры tm
  bcd[5]  =  timp->tm_mon  % 10;         /// Единицы месяцев
  bcd[5] |= (timp->tm_mon  / 10) << 4;    /// Десятки месяцев

  timp->tm_year -= 100;
  bcd[6]  =  timp->tm_year % 10;          /// Единицы лет
  bcd[6] |= (timp->tm_year / 10) << 4;    /// Десятки лет
}

/**
 *
 */
static void ds1338_bcd2tm(uint8_t *bcd, struct tm *timp){

  timp->tm_sec  = ((bcd[0] >> 4) & 0x07) * 10 + (bcd[0] & 0x0F);
  timp->tm_min  = ((bcd[1] >> 4) & 0x07) * 10 + (bcd[1] & 0x0F);
  timp->tm_hour = ((bcd[2] >> 4) & 0x03) * 10 + (bcd[2] & 0x0F);
  timp->tm_mday = ((bcd[4] >> 4) & 0x03) * 10 + (bcd[4] & 0x0F);
  timp->tm_mon  = ((bcd[5] >> 4) & 0x01) * 10 + (bcd[5] & 0x0F);
  timp->tm_year = ((bcd[6] >> 4) & 0x0F) * 10 + (bcd[6] & 0x0F);
  timp->tm_isdst= 0;

  // Согласование форматов  RTC1338 и структуры tm
  timp->tm_mon -= 1;
  timp->tm_year += 100;
}

/**
 * param[in]    bcd array to store time in DS1338
 */
static void ds1338_set_time_bcd(uint8_t *bcd){
  txbuf[0] = 0;
  memcpy(txbuf + 1, bcd, DS1338_TIME_SIZE);
  i2cAcquireBus(&I2CD1);
  i2cMasterTransmitTimeout(&I2CD1, ds1338addr, txbuf, DS1338_TIME_SIZE + 1, NULL, 0, TIME_INFINITE);
  i2cReleaseBus(&I2CD1);
}

/**
 * param[out]   bcd array to store time from DS1338
 */
static void ds1338_get_time_bcd(uint8_t *bcdbuf){
  txbuf[0] = 0;
  i2cAcquireBus(&I2CD1);
  i2cMasterTransmitTimeout(&I2CD1, ds1338addr, txbuf, 1, NULL, 0, TIME_INFINITE);
  i2cMasterReceiveTimeout(&I2CD1, ds1338addr, bcdbuf, DS1338_TIME_SIZE, TIME_INFINITE);
  i2cReleaseBus(&I2CD1);
}

/*
 *******************************************************************************
 * EXPORTED FUNCTIONS
 *******************************************************************************
 */
/**
 *
 */
void ds1338Init(void) {

  i2cAcquireBus(&I2CD1);
  txbuf[0] = DS1338_CONTROL;
  i2cMasterTransmitTimeout(&I2CD1, ds1338addr, txbuf, 1, NULL, 0, TIME_INFINITE);
  i2cMasterReceiveTimeout(&I2CD1, ds1338addr, rxbuf, 1, TIME_INFINITE);

  /* oscillator stop flag */
  if (rxbuf[0] & 0b10000){
    clearGlobalFlag(GlobalFlags.time_good);
    txbuf[0] = 0;
    txbuf[1] = 0;
    /* set CH=0 */
    i2cMasterTransmitTimeout(&I2CD1, ds1338addr, txbuf, 2, rxbuf, 0, TIME_INFINITE);
    /* clear OSF flag */
    txbuf[0] = DS1338_CONTROL;
    txbuf[1] = 0;
    i2cMasterTransmitTimeout(&I2CD1, ds1338addr, txbuf, 2, rxbuf, 0, TIME_INFINITE); /* set pointer */
  }
  i2cReleaseBus(&I2CD1);
}

/**
 *
 */
void ds1338GetTimeTm(struct tm *timp){
  uint8_t bcdbuf[DS1338_TIME_SIZE];

  ds1338_get_time_bcd(bcdbuf);
  ds1338_bcd2tm(bcdbuf, timp);
}

/**
 *
 */
time_t ds1338GetTimeUnixSec(void){
  struct tm timp;
  uint8_t bcdbuf[DS1338_TIME_SIZE];

  ds1338_get_time_bcd(bcdbuf);
  ds1338_bcd2tm(bcdbuf, &timp);
  return mktime(&timp);
}

/**
 *
 */
int64_t ds1338GetTimeUnixUsec(void){
  int64_t t;
  t = ds1338GetTimeUnixSec();
  t *= 1000000;
  return t;
}

/**
 * param[in] timp   pointer to correctly filled time  structure
 */
void ds1338SetTimeTm(struct tm *timp){
  uint8_t bcd[DS1338_TIME_SIZE];

  ds1338_tm2bcd(timp, bcd);
  ds1338_set_time_bcd(bcd);
}





