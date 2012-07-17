#include "ch.h"
#include "hal.h"

#include "spi_local.h"

/*
 *
 */
static const SPIConfig out_spicfg = {
  NULL,
  GPIOE,
  GPIOE_SR_OUT_NSS,
  SPI_CR1_BR_2 | SPI_CR1_BR_1 | SPI_CR1_BR_0
};

/*
 *
 */
static const SPIConfig in_spicfg = {
  NULL,
  GPIOB,
  GPIOB_SR_IN_NSS,
  SPI_CR1_BR_2 | SPI_CR1_BR_1 | SPI_CR1_BR_0
};

/*
 * SPI TX and RX buffers.
 */
static uint8_t txbuf[512];
static uint8_t rxbuf[512];

/*
 * SPI bus contender 1.
 */
static WORKING_AREA(sr_in_thread_wa, 256);
static msg_t sr_in_thread(void *p) {

  (void)p;
  chRegSetThreadName("SPI_IN");
  while (TRUE) {
    spiAcquireBus(&SPID2);              /* Acquire ownership of the bus.    */
    spiStart(&SPID2, &in_spicfg);       /* Setup transfer parameters.       */
    spiSelect(&SPID2);                  /* Slave Select assertion.          */
    spiExchange(&SPID2, 512, txbuf, rxbuf); /* Atomic transfer operations.  */
    spiUnselect(&SPID2);                /* Slave Select de-assertion.       */
    spiReleaseBus(&SPID2);              /* Ownership release.               */
  }
  return 0;
}

/*
 * SPI bus contender 2.
 */
static WORKING_AREA(sr_out_thread_wa, 256);
static msg_t sr_out_thread(void *p) {

  (void)p;
  chRegSetThreadName("SPI_OUT");
  while (TRUE) {
    spiAcquireBus(&SPID2);              /* Acquire ownership of the bus.    */
    spiStart(&SPID2, &out_spicfg);       /* Setup transfer parameters.       */
    spiSelect(&SPID2);                  /* Slave Select assertion.          */
    spiExchange(&SPID2, 512, txbuf, rxbuf);  /* Atomic transfer operations. */
    spiUnselect(&SPID2);                /* Slave Select de-assertion.       */
    spiReleaseBus(&SPID2);              /* Ownership release.               */
  }
  return 0;
}

/*
 * Application entry point.
 */
void SpiInitLocal(void) {
  /*
   * SPI1 I/O pins setup.
   */
//  palSetPadMode(IOPORT1, 5, PAL_MODE_STM32_ALTERNATE_PUSHPULL);     /* SCK. */
//  palSetPadMode(IOPORT1, 6, PAL_MODE_STM32_ALTERNATE_PUSHPULL);     /* MISO.*/
//  palSetPadMode(IOPORT1, 7, PAL_MODE_STM32_ALTERNATE_PUSHPULL);     /* MOSI.*/
//  palSetPadMode(IOPORT1, GPIOA_SPI1NSS, PAL_MODE_OUTPUT_PUSHPULL);
//  palSetPad(IOPORT1, GPIOA_SPI1NSS);

  /*
   * Starting the transmitter and receiver threads.
   */
  chThdCreateStatic(sr_in_thread_wa, sizeof(sr_in_thread_wa),
                    NORMALPRIO, sr_in_thread, NULL);
  chThdCreateStatic(sr_out_thread_wa, sizeof(sr_out_thread_wa),
                    NORMALPRIO, sr_out_thread, NULL);
}

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

/*
 ******************************************************************************
 * PROTOTYPES
 ******************************************************************************
 */

/*
 ******************************************************************************
 ******************************************************************************
 * LOCAL FUNCTIONS
 ******************************************************************************
 ******************************************************************************
 */

/*
 ******************************************************************************
 * EXPORTED FUNCTIONS
 ******************************************************************************
 */

