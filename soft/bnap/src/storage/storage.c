/*
 Уровень абстракции над микросдшкой.
 */
#include "ch.h"
#include "hal.h"

#include "main.h"
#include "message.h"
#include "storage.h"
#include "timekeeper.h"
#include "crc32.h"
#include "utils.h"

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
extern mavlink_gps_raw_int_t      mavlink_gps_raw_int_struct;
extern mavlink_mpiovd_sensors_t   mavlink_mpiovd_sensors_struct;

extern GlobalFlags_t              GlobalFlags;

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
 * Fill buffer with data.
 * return pointer to filled buffer
 */
static bool_t _fill_buf(void *mmcbuf){

  int64_t timestamp;
  uint32_t sig = RECORD_SIGNATURE;
  uint32_t crc;
  void *dest = mmcbuf;

  /* clear buffer */
  memset(dest, 0, RECORD_SIZE);

  /* signature */
  memcpy(dest, &sig, RECORD_SIGNATURE_SIZE);
  dest += RECORD_SIGNATURE_SIZE;

  /* timestamp */
  if (GlobalFlags.time_good == 1)
    timestamp = fastGetTimeUnixUsec();
  else
    return CH_FAILED;

  memcpy(dest, &timestamp, RECORD_TIMESTAMP_SIZE);
  dest += RECORD_TIMESTAMP_SIZE;

  /* payload */
  memcpy_ts(dest, &mavlink_mpiovd_sensors_struct, sizeof(mavlink_mpiovd_sensors_struct), 4);
  ((mavlink_mpiovd_sensors_t *)dest)->time_usec = timestamp;
  dest += sizeof(mavlink_mpiovd_sensors_struct);
  memcpy_ts(dest, &mavlink_gps_raw_int_struct, sizeof(mavlink_gps_raw_int_struct), 4);
  ((mavlink_gps_raw_int_t *)dest)->time_usec = timestamp;
  dest += sizeof(mavlink_gps_raw_int_struct);

  /* checksum */
  crc = crc32(0, mmcbuf, RECORD_SIZE - RECORD_CRC_SIZE);
  dest = mmcbuf + RECORD_CRC_OFFSET;
  memcpy(dest, &crc, RECORD_CRC_SIZE);

  /**/
  return CH_SUCCESS;
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

  chDbgCheck(GlobalFlags.time_good == 1, "This function can not work without correct system time");

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
uint32_t _rec2block(BnapStorage *bsp, uint32_t recnum){
  chDbgCheck(bsp->used != 0, "handle empty storage separately");
  chDbgCheck(recnum > bsp->used, "handle overflow externally");

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
void bnapStorageObjectInit(BnapStorage *bsp, MMCDriver *mmcp, void *mmcbuf){
  bsp->mmcp = mmcp;
  mmcObjectInit(bsp->mmcp);
  chSemInit(&bsp->semaphore, FALSE);
  bsp->buf = mmcbuf;
}

void bnapStorageStart(BnapStorage *bsp, const MMCConfig *config){
  mmcStart(bsp->mmcp, config);
}

bool_t bnapStorageConnect(BnapStorage *bsp){
  return mmcConnect(bsp->mmcp);
}

bool_t bnapStorageDisconnect(BnapStorage *bsp){
  return mmcDisconnect(bsp->mmcp);
}

void bnapStorageStop(BnapStorage *bsp){
  mmcStop(bsp->mmcp);
}

void bnapStoragaAcquire(BnapStorage *bsp){
  chSemWait(&bsp->semaphore);
}

void bnapStoragaRelease(BnapStorage *bsp){
  chSemSignal(&bsp->semaphore);
}

/**
 *
 */
bool_t bnapStorageDoRecord(BnapStorage *bsp){
  bool_t status;

  status = _fill_buf(bsp->buf);
  chDbgCheck(status == CH_SUCCESS, "filling buffer failed");

  status = bsp->mmcp->vmt->write(bsp->mmcp, bsp->tip, bsp->buf, 1);
  chDbgCheck(status == CH_SUCCESS, "write failed");

  bsp->tip++;
  if (bsp->tip >= bsp->mmcp->capacity)
    bsp->tip = 0; /* wrap ring buffer */

  bsp->used++;
  if (bsp->used >= bsp->mmcp->capacity)
    bsp->used = bsp->mmcp->capacity; /* clamp value */

  return status;
}

/**
 *
 */
bool_t bnapStorageGetRecord(BnapStorage *bsp, uint32_t rec){
  bool_t status;

  status = bsp->mmcp->vmt->read(bsp->mmcp, _rec2block(bsp, rec), bsp->buf, 1);
  chDbgCheck(status == CH_SUCCESS, "read failed");

  return status;
}

/**
 * Search number of first block suitable for writing.
 */
void bnapStorageMount(BnapStorage *bsp){
  uint32_t p0 = 0;
  uint32_t p1 = bsp->mmcp->capacity - 1;
  while (p1 > p0){
    _search_knee(bsp->mmcp, &p0, &p1, bsp->buf);
  }
  bsp->tip = p0;

  if (-1 == _check_block(bsp->mmcp, p0, bsp->buf))
    bsp->used = p0; /* only blocks before tip used */
  else
    bsp->used = bsp->mmcp->capacity; /* whole card used */
}

