/*
 - Следит за наличием карточки в слоте
 - монтирует/размонтирует
 - дергает функции записи по наступлению определенных событий

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
#include "storage.h"

/*
 ******************************************************************************
 * DEFINES
 ******************************************************************************
 */
#define SDC_POLLING_INTERVAL            MS2ST(500)
#define SDC_POLLING_DELAY               MS2ST(5)

#define MAX_SPI_BITRATE                 100
#define MIN_SPI_BITRATE                 250

/* logger writing interval. Set slightly longer than GPS period. */
#define WRITE_TMO                       MS2ST(1100)

/*
 ******************************************************************************
 * EXTERNS
 ******************************************************************************
 */
extern GlobalFlags_t GlobalFlags;
extern EventSource event_mavlink_gps_raw_int;
extern EventSource event_mavlink_oblique_storage_request_count;
extern EventSource event_mavlink_oblique_storage_request;

/*
 ******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************
 */

/* working buffer */
static uint8_t mmcbuf[STORAGE_BUFF_SIZE];

/**
 * MMC driver instance.
 */
static MMCDriver MMCD1;

/* Maximum speed SPI configuration (__MHz, NCPHA=1, CPOL=0).*/
static const SPIConfig hs_spicfg = {
  NULL,
  IOPORT1,
  PIOA_SPI0_NSS,
  (MAX_SPI_BITRATE << 8) | AT91C_SPI_NCPHA | AT91C_SPI_BITS_8
};

/* Low speed SPI configuration (192kHz, NCPHA=1, CPOL=0).*/
static const SPIConfig ls_spicfg = {
  NULL,
  IOPORT1,
  PIOA_SPI0_NSS,
  (MIN_SPI_BITRATE << 8) | AT91C_SPI_NCPHA | AT91C_SPI_BITS_8
};

/* MMC/SD over SPI driver configuration.*/
static const MMCConfig mmccfg = {&SPID1, &ls_spicfg, &hs_spicfg};

/**/
static BnapStorage Storage;

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
static void insert_handler(void) {
  bnapStorageObjectInit(&Storage, &MMCD1, mmcbuf);
  bnapStorageStart(&Storage, &mmccfg);

  /* On insertion SDC initialization and FS mount. */
  if (CH_SUCCESS == bnapStorageConnect(&Storage)){
    setGlobalFlag(GlobalFlags.storage_connected);
    bnapStoragaAcquire(&Storage);
    bnapStorageMount(&Storage);
    bnapStoragaRelease(&Storage);
    setGlobalFlag(GlobalFlags.logger_ready);
  }
}

/*
 * SD card removal event.
 */
static void remove_handler(void) {
  clearGlobalFlag(GlobalFlags.logger_ready);
  clearGlobalFlag(GlobalFlags.storage_connected);

  if (Storage.mmcp->state == BLK_ACTIVE){
    bnapStorageDisconnect(&Storage);
    bnapStorageStop(&Storage);
  }
}

/**
 *
 */
static WORKING_AREA(MmcWriterThreadWA, 512);
static msg_t MmcWriterThread(void *arg){
  chRegSetThreadName("MmcWriter");
  (void)arg;
  struct EventListener el_gps_raw_int;
  chEvtRegisterMask(&event_mavlink_gps_raw_int, &el_gps_raw_int, EVMSK_MAVLINK_GPS_RAW_INT);
  eventmask_t evt = 0;

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
    evt = chEvtWaitOneTimeout(EVMSK_MAVLINK_GPS_RAW_INT, WRITE_TMO);
    if (!mmcIsCardInserted(&MMCD1)){
      remove_handler();
      goto NOT_READY;
    }
    else{
      (void)evt;
      bnapStoragaAcquire(&Storage);
      bnapStorageDoRecord(&Storage);
      bnapStoragaRelease(&Storage);
    }
  }
  return 0;
}

/**
 *
 */
static WORKING_AREA(MmcReaderThreadWA, 512);
static msg_t MmcReaderThread(void *arg){
  chRegSetThreadName("MmcReader");
  (void)arg;

  struct EventListener el_storage_request_count;
  struct EventListener el_storage_request;
  chEvtRegisterMask(&event_mavlink_oblique_storage_request_count, &el_storage_request_count, EVMSK_MAVLINK_OBLIQUE_STORAGE_REQUEST_COUNT);
  chEvtRegisterMask(&event_mavlink_oblique_storage_request, &el_storage_request, EVMSK_MAVLINK_OBLIQUE_STORAGE_REQUEST);

  eventmask_t evt = 0;

  while (!chThdShouldTerminate()){
    evt = chEvtWaitOneTimeout(EVMSK_MAVLINK_OBLIQUE_STORAGE_REQUEST_COUNT |
                              EVMSK_MAVLINK_OBLIQUE_STORAGE_REQUEST, MS2ST(50));
    if (!mmcIsCardInserted(&MMCD1))
      continue;

    else{
      switch (evt){
      case(EVMSK_MAVLINK_OBLIQUE_STORAGE_REQUEST_COUNT):
        bnapStoragaAcquire(&Storage);
        bnapStoragaRelease(&Storage);
        break;

      case(EVMSK_MAVLINK_OBLIQUE_STORAGE_REQUEST):
        bnapStoragaAcquire(&Storage);
        bnapStorageGetRecord(&Storage, 0);
        bnapStoragaRelease(&Storage);
        break;

      default:
        break;
      }
    }
  }

  chEvtUnregister(&event_mavlink_oblique_storage_request_count, &el_storage_request_count);
  chEvtUnregister(&event_mavlink_oblique_storage_request, &el_storage_request);
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
void MicrosdInit(void){

  /*
   * Initializes the MMC driver to work with SPI.
   */
  palSetPadMode(IOPORT1, PIOA_SPI0_NSS, PAL_MODE_OUTPUT_PUSHPULL);
  palSetPad(IOPORT1, PIOA_SPI0_NSS);

  chThdCreateStatic(MmcWriterThreadWA,
          sizeof(MmcWriterThreadWA),
          MMC_THREAD_PRIO,
          MmcWriterThread,
          NULL);

  chThdCreateStatic(MmcReaderThreadWA,
          sizeof(MmcReaderThreadWA),
          MMC_THREAD_PRIO,
          MmcReaderThread,
          NULL);
}
