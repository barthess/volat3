#include <string.h>

#include "ch.h"
#include "hal.h"

#include "main.h"
#include "logger.h"
#include "timekeeper.h"

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
extern GlobalFlags_t GlobalFlags;
extern uint8_t mmcbuf[BUFF_SIZE];

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
 * Parse block and return its timestamp.
 *
 * @param[in]   block number
 *
 * @return      timestamp of block
 * @retval -1   data in block is incorrect
 */
static int64_t parse_block(MMCDriver *mmcp, uint32_t n){
  uint32_t sig = RECORD_SIGNATURE;
  int64_t timestamp = -1;

  mmcStartSequentialRead(mmcp, n);
  mmcSequentialRead(mmcp, mmcbuf);

  /* */
  if (0 != memcmp(mmcbuf + RECORD_SIGNATURE_OFFSET, &sig, RECORD_SIGNATURE_SIZE))
    return -1;

  /* */
  if (!is_crc_valid(mmcbuf))
    return -1;

  /* */
  memcpy(&timestamp, mmcbuf + RECORD_SIGNATURE_SIZE, RECORD_TIMESTAMP_SIZE);
  if (timestamp > fastGetTimeUnixUsec())
    return -1;
  else
    return timestamp;
}



/*
 *******************************************************************************
 * EXPORTED FUNCTIONS
 *******************************************************************************
 */

/**
 * Search last record in ring buffer using binary search algorithm.
 * Function modifies block numbers using poiters to them.
 *
 * param[in] mmcdp          pointer to MMC driver
 * param[in] p0             pointer to first block number to search
 * param[in] p1             pointer to last block number to search
 */
static void _bin_search(MMCDriver *mmcp, uint32_t *p0, uint32_t *p1){
  int64_t  t0;  /* timestamp of block */
  int64_t  t;   /* mid point */
  uint32_t p;   /* mid point */

  chDbgCheck(GlobalFlags.time_good == 1, "This function can not work without correct system time");

  t0 = parse_block(mmcp, *p0);
  p  = *p0 + ((*p1 - *p0) / 2);
  t  = parse_block(mmcp, p);

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
 * Return number of first block suitable for writing.
 */
uint32_t fsck(MMCDriver *mmcp){
  uint32_t p0 = 0;
  uint32_t p1 = mmcp->capacity;
  while (p1 > p0){
    _bin_search(mmcp, &p0, &p1);
  }
  return p0;
}



