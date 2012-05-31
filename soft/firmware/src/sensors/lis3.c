#include "ch.h"
#include "hal.h"

#include "lis3.h"
#include "main.h"
#include "sensors.h"

/*
 ******************************************************************************
 * DEFINES
 ******************************************************************************
 */
#define lis3_addr 0b0011101

/*
 ******************************************************************************
 * EXTERNS
 ******************************************************************************
 */
extern RawData raw_data;

/*
 ******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************
 */

/* buffers */
static uint8_t accel_rx_data[ACCEL_RX_DEPTH];
static uint8_t accel_tx_data[ACCEL_TX_DEPTH];

/*
 ******************************************************************************
 * PROTOTYPES
 ******************************************************************************
 */

/*
 *******************************************************************************
 *******************************************************************************
 * LOCAL FUNCTIONS
 *******************************************************************************
 *******************************************************************************
 */

static WORKING_AREA(PollLis3ThreadWA, 256);
static msg_t PollLis3Thread(void *arg){
  chRegSetThreadName("PollLis3");
  (void)arg;

  while (TRUE) {
    accel_tx_data[0] = ACCEL_OUT_DATA | AUTO_INCREMENT_BIT; /* register address */

    i2cAcquireBus(&I2CD1);
    i2cMasterTransmitTimeout(&I2CD1, lis3_addr, accel_tx_data, 1, accel_rx_data, 6, MS2ST(4));
    i2cReleaseBus(&I2CD1);

    raw_data.xacc = accel_rx_data[0] + (accel_rx_data[1] << 8);
    raw_data.yacc = accel_rx_data[2] + (accel_rx_data[3] << 8);
    raw_data.zacc = accel_rx_data[4] + (accel_rx_data[5] << 8);

    chThdSleepMilliseconds(200);
  }
  return 0;
}


/*
 *******************************************************************************
 * EXPORTED FUNCTIONS
 *******************************************************************************
 */
/**
 * Init function. Here we will also start personal serving thread.
 */
void init_lis3(void){
  /* configure accelerometer */
  accel_tx_data[0] = ACCEL_CTRL_REG1 | AUTO_INCREMENT_BIT; /* register address */
  accel_tx_data[1] = 0b11100111;
  accel_tx_data[2] = 0b01000001;
  accel_tx_data[3] = 0b00000000;

  /* sending */
  i2cAcquireBus(&I2CD1);
  i2cMasterTransmitTimeout(&I2CD1, lis3_addr, accel_tx_data, 4, accel_rx_data, 0, MS2ST(4));
  i2cReleaseBus(&I2CD1);

  chThdCreateStatic(PollLis3ThreadWA,
          sizeof(PollLis3ThreadWA),
          I2C_THREADS_PRIO,
          PollLis3Thread,
          NULL);
}
