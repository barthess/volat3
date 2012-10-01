#include "ch.h"
#include "hal.h"

#include "spi_local.h"
#include "discrete.h"
#include "utils.h"
#include "sensors.h"
#include "param.h"
#include "main.h"

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
extern RawData raw_data;
extern uint32_t GlobalFlags;

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
static uint32_t *rel_stm32_fix;

/*
 *
 */
//static const SPIConfig out_spicfg = {
//  NULL,
//  GPIOE,
//  GPIOE_SR_OUT_NSS,
//  SPI_CR1_BR_2 | SPI_CR1_BR_1 | SPI_CR1_BR_0
//};

/*
 *
 */
static const SPIConfig in_spicfg = {
  //in_spi_callback,
  NULL,
  GPIOB,
  GPIOB_SR_IN_NSS,
  SPI_CR1_CPOL | SPI_CR1_BR_2 | SPI_CR1_BR_1 | SPI_CR1_BR_0
  //SPI_CR1_BR_2 | SPI_CR1_BR_1 | SPI_CR1_BR_0
};

/*
 * SPI TX and RX buffers.
 */
//static uint8_t txbuf_fake[8];

static uint8_t rxbuf_z_on[9];
static uint8_t rxbuf_z_off[9];


/*
 ******************************************************************************
 ******************************************************************************
 * LOCAL FUNCTIONS
 ******************************************************************************
 ******************************************************************************
 */

/**
 * Кратковременно дергает вниз PL на всех сдвиговых регистрах
 */
void sample(void){
  sr_sample_on();
  polled_delay_us(5);
  sr_sample_off();
  polled_delay_us(5);
}

/**
 * Похоже, что SPI в STM32 криво работает с CPOL=1,CPHA=0.
 * Для того, чтобы получить правильные результаты нужно вычитать на один
 * байт больше, потом крайний правый столбец битов поднять на 1 ряд:
 *
 * 11111110    11111111
 * 11111111    11111111
 * 11111111 => 11111111
 * 11111111    11111111
 * 00000001    00000000
 */
void stm32_spi_workaround(uint8_t *rxbuf, size_t n){

  while (n){
    *rxbuf &= ~1;
    *rxbuf |= *(rxbuf + 1) & 1;
    rxbuf++;
    n--;
  }
}

/**
 * Helper function
 */
void read_spi(SPIDriver *spip, size_t n, void *rxbuf){

  sample();

  spiAcquireBus(spip);              /* Acquire ownership of the bus.    */
  spiSelect(spip);                  /* Slave Select assertion.          */
  spiReceive(spip, n, rxbuf);       /* Atomic transfer operations.      */
  spiUnselect(spip);
  spiReleaseBus(spip);              /* Ownership release.               */
  //spiStop(&SPID2);

  if (*rel_stm32_fix == 1)
    stm32_spi_workaround(rxbuf, n);
}

/*
 * SPI bus contender 1.
 */
static WORKING_AREA(sr_in_thread_wa, 256);
static msg_t sr_in_thread(void *p) {

  uint32_t result[2];
  uint32_t z_on[2];
  uint32_t z_off[2];

  systime_t time = chTimeNow();

  (void)p;
  chRegSetThreadName("SPI_IN");
  while (TRUE) {
    time += MS2ST(50);
    chThdSleepUntil(time);

    // включение происходит достаточно быстро в отличие от выключения
    z_check_on();
    chThdSleepMilliseconds(1);
    read_spi(&SPID2, 9, rxbuf_z_on);
    z_on[0] = pack8to32(rxbuf_z_on);
    z_on[1] = pack8to32(rxbuf_z_on + 4);

//    z_on[0] = 0;
//    z_on[1] = 0;

    z_check_off();
    chThdSleepMilliseconds(10);
    read_spi(&SPID2, 9, rxbuf_z_off);
    z_off[0] = pack8to32(rxbuf_z_off);
    z_off[1] = pack8to32(rxbuf_z_off + 4);
//      z_off[0] = -1;
//      z_off[1] = -1;

    rel_normalize(z_on, z_off, result);
    raw_data.discrete = pack32to64(result);
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
//    spiStart(&SPID2, &out_spicfg);      /* Setup transfer parameters.       */
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
 * EXPORTED FUNCTIONS
 ******************************************************************************
 */

void SpiInitLocal(void) {

  rel_stm32_fix = ValueSearch("REL_stm32_fix");

  spiStart(&SPID2, &in_spicfg);

  chThdCreateStatic(sr_in_thread_wa, sizeof(sr_in_thread_wa),
                    NORMALPRIO, sr_in_thread, NULL);
  chThdCreateStatic(sr_out_thread_wa, sizeof(sr_out_thread_wa),
                    NORMALPRIO, sr_out_thread, NULL);
}


