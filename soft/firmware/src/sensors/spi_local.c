#include "ch.h"
#include "hal.h"

#include "spi_local.h"
#include "discrete.h"

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
static void in_spi_callback(SPIDriver *spip);

/*
 ******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************
 */

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
  in_spi_callback,
  GPIOB,
  GPIOB_SR_IN_NSS,
  SPI_CR1_BR_2 | SPI_CR1_BR_1 | SPI_CR1_BR_0
};

/*
 * SPI TX and RX buffers.
 */
//static uint8_t txbuf[8];
static uint8_t rxbuf_z_on[8];
static uint8_t rxbuf_z_off[8];


/*
 ******************************************************************************
 ******************************************************************************
 * LOCAL FUNCTIONS
 ******************************************************************************
 ******************************************************************************
 */

/**
 * Helper function
 */
void read_discrete(SPIDriver *spip, size_t n, uint8_t *rxbuf){

  uint32_t t1, tmo;
  const uint32_t tmo_uS = 5;
  tmo = 1 + (halGetCounterFrequency() * tmo_uS) / 1000000;

  sr_sample_on();
  t1 = halGetCounterValue();
  while ((halGetCounterValue() - t1) < tmo)
    ;

  sr_sample_off();
  t1 = halGetCounterValue();
  while ((halGetCounterValue() - t1) < tmo)
    ;

  spiAcquireBus(spip);              /* Acquire ownership of the bus.    */
  spiStart(spip, &in_spicfg);       /* Setup transfer parameters.       */
  spiSelect(spip);                  /* Slave Select assertion.          */
  spiStartReceive(spip, n, rxbuf);  /* Atomic transfer operations.      */
  spiReleaseBus(spip);              /* Ownership release.               */
}

/*
 * SPI bus contender 1.
 */
static WORKING_AREA(sr_in_thread_wa, 256);
static msg_t sr_in_thread(void *p) {

  uint32_t result[2];

  (void)p;
  chRegSetThreadName("SPI_IN");
  while (TRUE) {

    z_check_on();
    chThdSleepMilliseconds(1);
    read_discrete(&SPID2, 8, rxbuf_z_on);
    chThdSleepMilliseconds(1);

    z_check_off();
    chThdSleepMilliseconds(1);
    read_discrete(&SPID2, 8, rxbuf_z_off);

    rel_normalize(rxbuf_z_on, rxbuf_z_off, result);

    chThdSleepMilliseconds(200);
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
//    spiAcquireBus(&SPID2);              /* Acquire ownership of the bus.    */
//    spiStart(&SPID2, &out_spicfg);       /* Setup transfer parameters.       */
//    spiSelect(&SPID2);                  /* Slave Select assertion.          */
//    spiExchange(&SPID2, 512, txbuf, rxbuf);  /* Atomic transfer operations. */
//    spiUnselect(&SPID2);                /* Slave Select de-assertion.       */
//    spiReleaseBus(&SPID2);              /* Ownership release.               */
    chThdSleepMilliseconds(1000);
  }
  return 0;
}

/**
 * Callback implementation
 */
static void in_spi_callback(SPIDriver *spip) {
  spiUnselectI(spip);                /* Slave Select de-assertion.       */
}

/*
 ******************************************************************************
 * EXPORTED FUNCTIONS
 ******************************************************************************
 */

void SpiInitLocal(void) {
  chThdCreateStatic(sr_in_thread_wa, sizeof(sr_in_thread_wa),
                    NORMALPRIO, sr_in_thread, NULL);
  chThdCreateStatic(sr_out_thread_wa, sizeof(sr_out_thread_wa),
                    NORMALPRIO, sr_out_thread, NULL);
}


