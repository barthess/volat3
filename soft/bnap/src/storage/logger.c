#include <string.h>

#include "ch.h"
#include "hal.h"
#include "mavlink.h"

#include "main.h"
#include "crc32.h"
#include "timekeeper.h"
#include "logger.h"
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

/* */
uint8_t mmcbuf[BUFF_SIZE];

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
 *******************************************************************************
 *******************************************************************************
 * LOCAL FUNCTIONS
 *******************************************************************************
 *******************************************************************************
 */

/*
 *******************************************************************************
 * EXPORTED FUNCTIONS
 *******************************************************************************
 */

/**
 * Fill buffer with data.
 * return pointer to filled buffer
 */
uint8_t* fill_record(void){

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
    return NULL;
  memcpy(dest, &timestamp, RECORD_TIMESTAMP_SIZE);
  dest += RECORD_TIMESTAMP_SIZE;

  /* payload */
  chSysLock();
  mavlink_mpiovd_sensors_struct.time_usec = timestamp;
  mavlink_gps_raw_int_struct.time_usec = timestamp;
  chSysUnlock();

  memcpy_ts(dest, &mavlink_mpiovd_sensors_struct, sizeof(mavlink_mpiovd_sensors_struct), 4);
  dest += sizeof(mavlink_mpiovd_sensors_struct);
  memcpy_ts(dest, &mavlink_gps_raw_int_struct, sizeof(mavlink_gps_raw_int_struct), 4);
  dest += sizeof(mavlink_gps_raw_int_struct);

  /* checksum */
  crc = crc32(0, mmcbuf, RECORD_SIZE - RECORD_CRC_SIZE);
  dest = mmcbuf + RECORD_CRC_OFFSET;
  memcpy(dest, &crc, RECORD_CRC_SIZE);

  /**/
  return mmcbuf;
}

/**
 * Check CRC validity of data block
 */
bool_t is_crc_valid(uint8_t *buf){
  uint32_t crc;
  memcpy(&crc, buf + (RECORD_SIZE - RECORD_CRC_SIZE), RECORD_CRC_SIZE);

  if (crc == crc32(0, buf, RECORD_SIZE - RECORD_CRC_SIZE))
    return TRUE;
  else
    return FALSE;
}








