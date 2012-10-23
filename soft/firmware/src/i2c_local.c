#include "ch.h"
#include "hal.h"

#include "i2c_local.h"
#include "main.h"
#include "param.h"
#include "storage.h"

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
 * GLOBAL VARIABLES
 ******************************************************************************
 */
/* interface #2 */
static const I2CConfig i2cfg2 = {
    OPMODE_I2C,
    100000, //
    STD_DUTY_CYCLE, //
};

/*
 *******************************************************************************
 * EXPORTED FUNCTIONS
 *******************************************************************************
 */
void I2CInitLocal(void){

  i2cStart(&I2CD1, &i2cfg2);
  ParametersInit(); /* читает настройки из EEPROM по I2C*/
  StorageInit();    /* init uptime and trip storage*/
}


/* обертка запускатор транзакции */
msg_t i2c_transmit(i2caddr_t addr, const uint8_t *txbuf, size_t txbytes,
                   uint8_t *rxbuf, size_t rxbytes){
  msg_t status = RDY_OK;

  i2cAcquireBus(&I2CD1);
  status = i2cMasterTransmitTimeout(&I2CD1, addr, txbuf, txbytes, rxbuf, rxbytes, MS2ST(6));
  i2cReleaseBus(&I2CD1);
  if (status == RDY_TIMEOUT){
    /* в случае таймаута необходимо перезапустить драйвер */
    i2cStop(&I2CD1);
    chThdSleepMilliseconds(1);
    i2cStart(&I2CD1, &i2cfg2);
    return status;
  }
  return status;
}

/* обертка запускатор транзакции */
msg_t i2c_receive(i2caddr_t addr, uint8_t *rxbuf, size_t rxbytes){
  msg_t status = RDY_OK;

  i2cAcquireBus(&I2CD1);
  status = i2cMasterReceiveTimeout(&I2CD1, addr, rxbuf, rxbytes, MS2ST(6));
  i2cReleaseBus(&I2CD1);
  chDbgAssert(status == RDY_OK, "i2c_transmit(), #1", "error in driver");
  if (status == RDY_TIMEOUT){
    /* в случае таймаута необходимо перезапустить драйвер */
    i2cStop(&I2CD1);
    chThdSleepMilliseconds(1);
    i2cStart(&I2CD1, &i2cfg2);
    return status;
  }
  return status;
}
