/*
 Корректная работа хранилища зиждется на следующих постулатах:
 - системное время устанавливается только автоматически по GSP. Вручную это
   делать запрещено
 - лог пишется тогда и только тогда, когда время признано правильным и взведен
   соответствующий флаг в GlobalFlags, потому что лог с неверным временем
   бесполезен
 - по временной метке находится последняя запись в буфере по характерному
   перегибу лини времени.
 */

#include <string.h>

#include "ch.h"
#include "hal.h"

#include "main.h"
#include "message.h"
#include "logger.h"
#include "fsck.h"

/*
 ******************************************************************************
 * DEFINES
 ******************************************************************************
 */
#define SDC_POLLING_INTERVAL            MS2ST(100)
#define SDC_POLLING_DELAY               MS2ST(5)

#define MAX_SPI_BITRATE                 100
#define MIN_SPI_BITRATE                 250

/* logger writing interval. Set slightly longer than GPS period. */
#define WRITE_TMO                  MS2ST(1100)

/*
 ******************************************************************************
 * EXTERNS
 ******************************************************************************
 */
extern GlobalFlags_t GlobalFlags;
extern EventSource event_gps_raw_int;

/*
 ******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************
 */

/* pointer to current record in storage */
static uint32_t StorageTip = 0;

/**
 * MMC driver instance.
 */
MMCDriver MMCD1;

/* Maximum speed SPI configuration (__MHz, NCPHA=1, CPOL=0).*/
static SPIConfig hs_spicfg = {
  NULL,
  IOPORT1,
  PIOA_SPI0_NSS,
  (MAX_SPI_BITRATE << 8) | AT91C_SPI_NCPHA | AT91C_SPI_BITS_8
};

/* Low speed SPI configuration (192kHz, NCPHA=1, CPOL=0).*/
static SPIConfig ls_spicfg = {
  NULL,
  IOPORT1,
  PIOA_SPI0_NSS,
  (MIN_SPI_BITRATE << 8) | AT91C_SPI_NCPHA | AT91C_SPI_BITS_8
};

/* MMC/SD over SPI driver configuration.*/
static MMCConfig mmccfg = {&SPID1, &ls_spicfg, &hs_spicfg};

/*
 *******************************************************************************
 *******************************************************************************
 * LOCAL FUNCTIONS
 *******************************************************************************
 *******************************************************************************
 */

/*
 * SD card insertion event.
 */
static bool_t insert_handler(void) {
  mmcObjectInit(&MMCD1);
  mmcStart(&MMCD1, &mmccfg);
  bool_t status;

  /* On insertion SDC initialization and FS mount. */
  if (mmcConnect(&MMCD1)){
    setGlobalFlag(GlobalFlags.storage_connected);
    StorageTip = fsck(&MMCD1);
    status = mmcStartSequentialWrite(&MMCD1, StorageTip);
    chDbgCheck(status == CH_SUCCESS, "seek failed");
    setGlobalFlag(GlobalFlags.logger_ready);
  }

  return status;
}

/*
 * SD card removal event.
 */
static void remove_handler(void) {
  clearGlobalFlag(GlobalFlags.logger_ready);
  clearGlobalFlag(GlobalFlags.storage_connected);

  if ((&MMCD1)->state == BLK_ACTIVE){
    mmcDisconnect(&MMCD1);
    mmcStop(&MMCD1);
  }
}

/**
 *
 */
static bool_t write_block(MMCDriver *mmcp){
  uint8_t *buf;
  bool_t status;

  buf = fill_record();
  status = mmcSequentialWrite(mmcp, buf);
  chDbgCheck(status == CH_SUCCESS, "write failed");
  StorageTip++;
  if (StorageTip > mmcp->capacity)
    StorageTip = 0;

  return status;
}

/**
 *
 */
static WORKING_AREA(SdThreadWA, 1024);
static msg_t SdThread(void *arg){
  chRegSetThreadName("MicroSD");
  (void)arg;
  struct EventListener el_gps_raw_int;
  chEvtRegisterMask(&event_gps_raw_int, &el_gps_raw_int, EVMSK_GPS_RAW_INT);

  /* wait until card not ready */
NOT_READY:
  while (!mmcIsCardInserted(&MMCD1))
    chThdSleep(SDC_POLLING_INTERVAL);
  chThdSleep(SDC_POLLING_INTERVAL);
  if (!mmcIsCardInserted(&MMCD1))
    goto NOT_READY;
  else
    insert_handler();

  /* main work cycle */
  while (!chThdShouldTerminate()){
    chEvtWaitOneTimeout(EVMSK_GPS_RAW_INT, WRITE_TMO);
    if (!mmcIsCardInserted(&MMCD1)){
      remove_handler();
      goto NOT_READY;
    }
    else{
      write_block(&MMCD1);
    }
  }
  return 0;
}

/*
 *******************************************************************************
 * EXPORTED FUNCTIONS
 *******************************************************************************
 */

/**
 *
 */
void StorageInit(void){

  /*
   * Initializes the MMC driver to work with SPI.
   */
  palSetPadMode(IOPORT1, PIOA_SPI0_NSS, PAL_MODE_OUTPUT_PUSHPULL);
  palSetPad(IOPORT1, PIOA_SPI0_NSS);

  chThdCreateStatic(SdThreadWA,
          sizeof(SdThreadWA),
          NORMALPRIO - 5,
          SdThread,
          NULL);
}
