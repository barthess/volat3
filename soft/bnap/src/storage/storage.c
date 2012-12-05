#include "ch.h"
#include "hal.h"
#include "chprintf.h"

#include "main.h"
#include "message.h"
#include "storage.h"
#include "timekeeper.h"
#include "crc32.h"
#include "utils.h"
#include "link_cc_packer.h"
#include "link_dm_packer.h"

/*
 ******************************************************************************
 * DEFINES
 ******************************************************************************
 */

/* if RTC time is in future more than this value (uS) than kill all
 * data in storage and start from clear state */
#define STORAGE_VOID_LIMIT_US    (30 * 60 * 1000000)

/*
 ******************************************************************************
 * EXTERNS
 ******************************************************************************
 */
extern mavlink_system_t           mavlink_system_struct;

extern mavlink_gps_raw_int_t      mavlink_gps_raw_int_struct;
extern mavlink_mpiovd_sensors_t   mavlink_mpiovd_sensors_struct;

extern const GlobalFlags_t        GlobalFlags;

/*
 ******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************
 */

/*
 *******************************************************************************
 *******************************************************************************
 * LOCAL FUNCTIONS
 *******************************************************************************
 *******************************************************************************
 */

/**
 *
 */
static uint16_t _to_mmc_buf(void *out, uint8_t *sendbuf, mavlink_message_t *mavlink_message_struct){
  uint16_t len = 0;

  len = mavlink_msg_to_send_buffer(sendbuf, mavlink_message_struct);
  memcpy(out, &len, sizeof(len));
  out += sizeof(len);
  memcpy(out, sendbuf, len);
  return len + sizeof(len);
}

/**
 *
 */
static uint16_t _store_gps_raw_int(void *out){
  mavlink_message_t mavlink_message_struct;
  uint8_t sendbuf[MAVLINK_MAX_PACKET_LEN];

  memcpy_ts(sendbuf, &mavlink_gps_raw_int_struct, sizeof(mavlink_gps_raw_int_struct), 4);
//  ((mavlink_gps_raw_int_t *)sendbuf)->time_usec = timestamp;
  mavlink_msg_gps_raw_int_encode(mavlink_system_struct.sysid, MAV_COMP_ID_BNAP, &mavlink_message_struct, (mavlink_gps_raw_int_t *)sendbuf);

  return _to_mmc_buf(out, sendbuf, &mavlink_message_struct);
}

/**
 *
 */
static uint16_t _store_mpiovd_sensors(void *out){
  mavlink_message_t mavlink_message_struct;
  uint8_t sendbuf[MAVLINK_MAX_PACKET_LEN];

  memcpy_ts(sendbuf, &mavlink_mpiovd_sensors_struct, sizeof(mavlink_mpiovd_sensors_struct), 4);
//  ((mavlink_mpiovd_sensors_t *)sendbuf)->time_usec = timestamp;
  mavlink_msg_mpiovd_sensors_encode(mavlink_system_struct.sysid, MAV_COMP_ID_MPIOVD, &mavlink_message_struct, (mavlink_mpiovd_sensors_t *)sendbuf);

  return _to_mmc_buf(out, sendbuf, &mavlink_message_struct);
}

/**
 * Fill buffer with data.
 * return operation status
 */
static void _fill_buf(void *mmcbuf, int64_t *timestamp){

  uint32_t sig = RECORD_SIGNATURE;
  uint32_t crc;

  void *dest = mmcbuf;

  /* clear buffer */
  memset(dest, 0, RECORD_SIZE);

  /* signature */
  dest = mmcbuf + RECORD_SIGNATURE_OFFSET;
  memcpy(dest, &sig, RECORD_SIGNATURE_SIZE);

  /* timestamp */
  dest = mmcbuf + RECORD_TIMESTAMP_OFFSET;
  memcpy(dest, timestamp, RECORD_TIMESTAMP_SIZE);
  /* не забываем про свободное место для сохранения размера данных */
  dest = mmcbuf + RECORD_PAYLOAD_OFFSET;

  /* payload */
  dest += _store_mpiovd_sensors(dest);
  dest += _store_gps_raw_int(dest);

  /* checksum */
  crc = crc32(0, mmcbuf, RECORD_SIZE - RECORD_CRC_SIZE);
  memcpy(mmcbuf + RECORD_CRC_OFFSET, &crc, RECORD_CRC_SIZE);
}

/**
 * Check CRC validity of data block
 */
static bool_t _crc_valid(void *mmcbuf){
  uint32_t crc;
  memcpy(&crc, mmcbuf + (RECORD_SIZE - RECORD_CRC_SIZE), RECORD_CRC_SIZE);

  if (crc == crc32(0, mmcbuf, RECORD_SIZE - RECORD_CRC_SIZE))
    return TRUE;
  else
    return FALSE;
}

/**
 * Parse block and return its timestamp.
 *
 * @param[in] mmcp      pointer to MMC driver
 * @param[in] n         block number
 * @param[in] mmcbuf    temporal working buffer
 *
 * @return      timestamp of block
 * @retval -1   data in block is incorrect
 */
static int64_t _check_block(MMCDriver *mmcp, uint32_t n, void *mmcbuf){
  uint32_t sig = RECORD_SIGNATURE;
  int64_t timestamp = -1;

  mmcp->vmt->read(mmcp, n, mmcbuf, 1);

  /* */
  if (0 != memcmp(mmcbuf + RECORD_SIGNATURE_OFFSET, &sig, RECORD_SIGNATURE_SIZE))
    return -1;

  /* */
  if (!_crc_valid(mmcbuf))
    return -1;

  /* */
  memcpy(&timestamp, mmcbuf + RECORD_SIGNATURE_SIZE, RECORD_TIMESTAMP_SIZE);
  if (timestamp > fastGetTimeUnixUsec())
    return -1; /* timestamp in future */
  else
    return timestamp;
}

/**
 * Search last record in ring buffer using binary search algorithm.
 * Function modifies block numbers using poiters to them.
 *
 * @param[in] mmcdp   pointer to MMC driver
 * @param[in] p0      pointer to first block number to search
 * @param[in] p1      pointer to last block number to search
 * @param[in] mmcbuf  temporal working buffer
 */
static void _search_knee(MMCDriver *mmcp, uint32_t *p0, uint32_t *p1, void *mmcbuf){
  int64_t  t0;  /* timestamp of block */
  int64_t  t;   /* time of mid point */
  uint32_t p;   /* mid block number */

  t0 = _check_block(mmcp, *p0, mmcbuf);
  p  = *p0 + ((*p1 - *p0) / 2);
  t  = _check_block(mmcp, p, mmcbuf);

  if ((*p0 == 0) && (t0 == -1)){ /* probably storage never used before */
    *p1 = *p0;
  }
  else{
    if (t0 < t)
      *p0 = p;
    else
      *p1 = p;
  }
}

/**
 * Return block number corresponding to record number.
 *
 * @param[in] recnum  number of record (the oldest one has #0)
  */
uint32_t _rec2block(BnapStorage_t *bsp, uint32_t recnum){
  chDbgCheck(bsp->used != 0, "handle empty storage separately");
  chDbgCheck(recnum < bsp->used, "handle overflow externally");

  if (bsp->used < bsp->mmcp->capacity)
    return recnum;
  else
    return (bsp->tip + recnum) % bsp->mmcp->capacity;
}

/*
 *******************************************************************************
 * EXPORTED FUNCTIONS
 *******************************************************************************
 */
void bnapStorageObjectInit(BnapStorage_t *bsp, MMCDriver *mmcp, void *mmcbuf){
  bsp->mmcp = mmcp;
  mmcObjectInit(bsp->mmcp);
  chSemInit(&bsp->semaphore, 1);
  bsp->buf = mmcbuf;
}

void bnapStorageStart(BnapStorage_t *bsp, const MMCConfig *config){
  mmcStart(bsp->mmcp, config);
}

bool_t bnapStorageConnect(BnapStorage_t *bsp){
  return mmcConnect(bsp->mmcp);
}

bool_t bnapStorageDisconnect(BnapStorage_t *bsp){
  return mmcDisconnect(bsp->mmcp);
}

void bnapStorageStop(BnapStorage_t *bsp){
  mmcStop(bsp->mmcp);
}

void bnapStorageAcquire(BnapStorage_t *bsp){
  chSemWait(&bsp->semaphore);
}

void bnapStorageRelease(BnapStorage_t *bsp){
  chSemSignal(&bsp->semaphore);
}

/*
Поскольку точное время жизненно важно для хранилища, то обращаться с ним
следует следующим образом:

- проведем базовую проверку сравнив в временем компиляции и установим
  time_good если проверка пройдена.
- ждем, пока появится время GPS,
- проведем коррекцию. Если время RTC в прошлом - то просто добавим разницу
  к текущему значению, в противном случае сбросим флаг time_good, затормозим
  счетчик до минимума и будем ждать, пока время GPS не догонит системное.
  Если разница больше чем STORAGE_VOID_LIMIT_US - обнулим хранилище.
*/
bool_t bnapStorageDoRecord(BnapStorage_t *bsp){
  bool_t status;
  int64_t timestamp = -1;

  /* there is no sense to do something without normal time */
  if (GlobalFlags.time_good != 1)
    return CH_FAILED;

  if (GlobalFlags.time_proved == 1){
    timestamp = fastGetTimeUnixUsec();
    if (bsp->mtime > timestamp){
      if (bsp->mtime > (timestamp + STORAGE_VOID_LIMIT_US)){
        bnapStorageVoid(bsp); /* time too far in future */
      }
      return CH_FAILED; /* wait until mtime be older than current system time */
    }
  }

  _fill_buf(bsp->buf, &timestamp);
  status = bsp->mmcp->vmt->write(bsp->mmcp, bsp->tip, bsp->buf, 1);
  chDbgCheck(status == CH_SUCCESS, "write failed");

  bsp->tip++;
  if (bsp->tip >= bsp->mmcp->capacity)
    bsp->tip = 0; /* wrap ring buffer */

  bsp->used++;
  if (bsp->used >= bsp->mmcp->capacity)
    bsp->used = bsp->mmcp->capacity; /* clamp value */

  bsp->mtime = timestamp;
  return CH_SUCCESS;
}

/**
 *
 */
bool_t bnapStorageGetRecord(BnapStorage_t *bsp, uint32_t rec){
  bool_t status;

  status = bsp->mmcp->vmt->read(bsp->mmcp, _rec2block(bsp, rec), bsp->buf, 1);
  chDbgCheck(status == CH_SUCCESS, "read failed");

  return status;
}

/**
 * Search number of first block suitable for writing.
 */
void bnapStorageMount(BnapStorage_t *bsp){
  int64_t t0;
  int64_t t1;
  uint32_t p0 = 0;
  uint32_t p1 = bsp->mmcp->capacity - 1;
  while (p1 > p0 + 1)
    _search_knee(bsp->mmcp, &p0, &p1, bsp->buf);

  /* check founded 2 points separately */
  t0 = _check_block(bsp->mmcp, p0, bsp->buf);
  t1 = _check_block(bsp->mmcp, p1, bsp->buf);

  if (t0 > t1){
    bsp->tip = p1;
    bsp->mtime = t0;
  }
  else{
    bsp->tip = p0;
    bsp->mtime = t1;
  }

  if (t1 == -1)
    bsp->used = bsp->tip; /* only blocks before tip used */
  else
    bsp->used = bsp->mmcp->capacity; /* whole card used */
}

/**
 * Null some first block of device. It is fast but not very safe.
 * As a safety solution use bnapStorageWipe().
 */
void bnapStorageVoid(BnapStorage_t *bsp){
  uint32_t n = 0;

  chDbgCheck(bsp != NULL, "");

  memset(bsp->buf, 0, STORAGE_BUFF_SIZE);
  while (n < 32){
    bsp->mmcp->vmt->write(bsp->mmcp, n, bsp->buf, 1);
    n++;
  }
  bsp->tip = 0;
}

/**
 * Wipe all data in storage using data in MCU flash as pattern to speedup
 * process
 * @param[in] bsp   pointer to storage device
 * @param[in] sdp   pointer to serial driver used for CLI output. Set to
 *                  NULL if unused
 */
void bnapStorageWipe(BnapStorage_t *bsp, SerialDriver *sdp){
  const uint32_t flash_size = 262144;
  const uint32_t blockatonce = flash_size / 512;
  void *data = (void *)0x100000;
  uint32_t n = 0;

  chDbgCheck(bsp != NULL, "");

  while ((n < bsp->mmcp->capacity) && (!chThdShouldTerminate())){
    bsp->mmcp->vmt->write(bsp->mmcp, n, data, blockatonce);
    n += blockatonce;
    if (sdp != NULL)
      chprintf((BaseSequentialStream *)sdp, "%U/%U\r\n", n, bsp->mmcp->capacity);
  }
  bsp->tip = 0;
}


