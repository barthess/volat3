#include "ch.h"
#include "hal.h"

#include "i2c_local.h"
#include "main.h"

//#include "param.h"

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
extern uint32_t GlobalFlags;

/*
 ******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************
 */
/* interface #2 */
static const I2CConfig i2cfg2 = {
    OPMODE_I2C,
    400000, //100000, //
    FAST_DUTY_CYCLE_16_9, //STD_DUTY_CYCLE, //
};

/*
 *******************************************************************************
 * EXPORTED FUNCTIONS
 *******************************************************************************
 */
void I2CInitLocal(void){

  i2cStart(&I2CD1, &i2cfg2);

  chThdSleepMilliseconds(1);
//  ParametersInit(); /* читает настройки из EEPROM по I2C*/
  chThdSleepMilliseconds(10);
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
    setGlobalFlag(I2C_RESTARTED_FLAG);
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
    setGlobalFlag(I2C_RESTARTED_FLAG);
    return status;
  }
  return status;
}
