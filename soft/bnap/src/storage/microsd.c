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

#include "link_cc.h"
#include "link_cc_packer.h"
#include "link_dm.h"
#include "link_dm_packer.h"

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
//#define WRITE_TMO                       MS2ST(1)

#define STORAGE_CC_SEND_DELAY           MS2ST(100)

/*
 ******************************************************************************
 * EXTERNS
 ******************************************************************************
 */
extern GlobalFlags_t GlobalFlags;
extern EventSource event_mavlink_gps_raw_int;

extern EventSource event_mavlink_oblique_storage_count;
extern EventSource event_mavlink_oblique_storage_request_count_cc;
extern EventSource event_mavlink_oblique_storage_request_cc;
extern EventSource event_mavlink_oblique_storage_request_count_dm;
extern EventSource event_mavlink_oblique_storage_request_dm;

extern mavlink_oblique_storage_count_t    mavlink_oblique_storage_count_struct;
extern mavlink_oblique_storage_request_t  mavlink_oblique_storage_request_cc_struct;
extern mavlink_oblique_storage_request_t  mavlink_oblique_storage_request_dm_struct;

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
static void _insert_handler(void) {
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
static void _remove_handler(void) {
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
static WORKING_AREA(MmcWriterThreadWA, 1536);
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
    _insert_handler();

  /* main work cycle */
  while (!chThdShouldTerminate()){
    evt = chEvtWaitOneTimeout(EVMSK_MAVLINK_GPS_RAW_INT, WRITE_TMO);
    if (!mmcIsCardInserted(&MMCD1)){
      _remove_handler();
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
static void _oblique_storage_request_handler_cc(SerialDriver *sdp){
  uint32_t curr = mavlink_oblique_storage_request_cc_struct.first;
  uint32_t last = mavlink_oblique_storage_request_cc_struct.last;
  uint16_t len;
  void *data;

  if (curr > last)
    return;
  if (last >= Storage.used)
    last = Storage.used - 1;

  while (curr < last){
    bnapStoragaAcquire(&Storage);
    acquire_cc_out(); /* нагло занимаем канал передачи */
    bnapStorageGetRecord(&Storage, curr); /* сохраняем блок в буфере Storage */

    data = Storage.buf + RECORD_PAYLOAD_OFFSET + sizeof(len);
    len = *(uint16_t *)(Storage.buf + RECORD_PAYLOAD_OFFSET);
    while (len != 0){
      cc_sdWrite(sdp, data, len);
      data += len;
      len = *(uint16_t *)data;
      data += sizeof(len);
    }
    release_cc_out();
    bnapStoragaRelease(&Storage);
    curr++;

    chThdSleepMilliseconds(STORAGE_CC_SEND_DELAY);
  }
}

/**
 *
 */
static void _oblique_storage_request_handler_dm(SerialDriver *sdp){
  uint32_t curr = mavlink_oblique_storage_request_dm_struct.first;
  uint32_t last = mavlink_oblique_storage_request_dm_struct.last;
  uint16_t len  = 0;
  void *data;

  if (curr > last)
    return;
  if (last >= Storage.used)
    last = Storage.used - 1;

  acquire_dm_out(); /* нагло занимаем канал передачи */
  while (curr < last){
    bnapStoragaAcquire(&Storage);
    bnapStorageGetRecord(&Storage, curr); /* сохраняем блок в буфере Storage */

    data = Storage.buf + RECORD_PAYLOAD_OFFSET + sizeof(len);
    len = *(uint16_t *)(Storage.buf + RECORD_PAYLOAD_OFFSET);
    while (len != 0){
      dm_sdWrite(sdp, data, len);
      data += len;
      len = *(uint16_t *)data;
      data += sizeof(len);
    }

    bnapStoragaRelease(&Storage);
    curr++;
  }
  release_dm_out();
}

/**
 *
 */
static WORKING_AREA(MmcReaderCcThreadWA, 256);
static msg_t MmcReaderCcThread(void *sdp){
  chRegSetThreadName("MmcReaderCc");

  struct EventListener el_storage_request_count_cc;
  struct EventListener el_storage_request_cc;

  chEvtRegisterMask(&event_mavlink_oblique_storage_request_count_cc,
                    &el_storage_request_count_cc,
                    EVMSK_MAVLINK_OBLIQUE_STORAGE_REQUEST_COUNT_CC);
  chEvtRegisterMask(&event_mavlink_oblique_storage_request_cc,
                    &el_storage_request_cc,
                    EVMSK_MAVLINK_OBLIQUE_STORAGE_REQUEST_CC);

  eventmask_t evt = 0;

  while (!chThdShouldTerminate()){
    evt = chEvtWaitOneTimeout(EVMSK_MAVLINK_OBLIQUE_STORAGE_REQUEST_COUNT_CC | EVMSK_MAVLINK_OBLIQUE_STORAGE_REQUEST_CC, MS2ST(50));
    if (!mmcIsCardInserted(&MMCD1))
      continue;
    else{
      switch (evt){
      case(EVMSK_MAVLINK_OBLIQUE_STORAGE_REQUEST_COUNT_CC):
        bnapStoragaAcquire(&Storage);
        mavlink_oblique_storage_count_struct.count = Storage.used;
        bnapStoragaRelease(&Storage);
        chEvtBroadcastFlags(&event_mavlink_oblique_storage_count, EVMSK_MAVLINK_OBLIQUE_STORAGE_COUNT);
        break;

      case(EVMSK_MAVLINK_OBLIQUE_STORAGE_REQUEST_CC):
        _oblique_storage_request_handler_cc(sdp);
        break;

      default:
        break;
      }
    }
  }

  chEvtUnregister(&event_mavlink_oblique_storage_request_count_cc,
                  &el_storage_request_count_cc);
  chEvtUnregister(&event_mavlink_oblique_storage_request_cc,
                  &el_storage_request_cc);
  return 0;
}

/**
 *
 */
static WORKING_AREA(MmcReaderDmThreadWA, 256);
static msg_t MmcReaderDmThread(void *sdp){
  chRegSetThreadName("MmcReaderDm");

  struct EventListener el_storage_request_count_dm;
  struct EventListener el_storage_request_dm;

  chEvtRegisterMask(&event_mavlink_oblique_storage_request_count_dm,
                    &el_storage_request_count_dm,
                    EVMSK_MAVLINK_OBLIQUE_STORAGE_REQUEST_COUNT_DM);
  chEvtRegisterMask(&event_mavlink_oblique_storage_request_dm,
                    &el_storage_request_dm,
                    EVMSK_MAVLINK_OBLIQUE_STORAGE_REQUEST_DM);

  eventmask_t evt = 0;

  while (!chThdShouldTerminate()){
    evt = chEvtWaitOneTimeout(EVMSK_MAVLINK_OBLIQUE_STORAGE_REQUEST_COUNT_DM | EVMSK_MAVLINK_OBLIQUE_STORAGE_REQUEST_DM, MS2ST(50));
    if (!mmcIsCardInserted(&MMCD1))
      continue;
    else{
      switch (evt){
      case(EVMSK_MAVLINK_OBLIQUE_STORAGE_REQUEST_COUNT_DM):
        bnapStoragaAcquire(&Storage);
        mavlink_oblique_storage_count_struct.count = Storage.used;
        bnapStoragaRelease(&Storage);
        chEvtBroadcastFlags(&event_mavlink_oblique_storage_count, EVMSK_MAVLINK_OBLIQUE_STORAGE_COUNT);
        break;

      case(EVMSK_MAVLINK_OBLIQUE_STORAGE_REQUEST_DM):
        _oblique_storage_request_handler_dm(sdp);
        break;

      default:
        break;
      }
    }
  }

  chEvtUnregister(&event_mavlink_oblique_storage_request_count_dm,
                  &el_storage_request_count_dm);
  chEvtUnregister(&event_mavlink_oblique_storage_request_dm,
                  &el_storage_request_dm);
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

  chThdCreateStatic(MmcReaderCcThreadWA,
          sizeof(MmcReaderCcThreadWA),
          MMC_THREAD_PRIO,
          MmcReaderCcThread,
          &SDGSM);

  chThdCreateStatic(MmcReaderDmThreadWA,
          sizeof(MmcReaderDmThreadWA),
          MMC_THREAD_PRIO,
          MmcReaderDmThread,
          &SDDM);
}
