#include "ch.h"
#include "hal.h"

#include "spi_local.h"
#include "discrete.h"

static void in_spi_callback(SPIDriver *spip) {
  spiUnselectI(spip);                /* Slave Select de-assertion.       */
}

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

/**
 * Helper function
 */
void read_discrete(SPIDriver *spip, size_t n, uint8_t *rxbuf){
  uint32_t tmo;

  // TODO: rewrite this code. With optimizations it produces inadequate results
  tmo = halGetCounterValue();
  tmo = halGetCounterFrequency();
  sr_sample_on();
  tmo = 256;
  while (tmo)
    tmo--;

  sr_sample_off();
  tmo = 256;
  while (tmo)
    tmo--;

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

  uint64_t result64;

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

//    result64 = rel_normalize64(rxbuf_z_on, rxbuf_z_off, 0, -1, -1);

    result64 = rel_normalize32(rxbuf_z_on, rxbuf_z_off, 0, -1, -1);
    result64 = result64 << 32;
    result64 |= rel_normalize32(&rxbuf_z_on[4], &rxbuf_z_off[4], 0, -1, -1);

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

void SpiInitLocal(void) {
  chThdCreateStatic(sr_in_thread_wa, sizeof(sr_in_thread_wa),
                    NORMALPRIO, sr_in_thread, NULL);
  chThdCreateStatic(sr_out_thread_wa, sizeof(sr_out_thread_wa),
                    NORMALPRIO, sr_out_thread, NULL);
}


