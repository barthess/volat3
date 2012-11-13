#include <time.h>

#include "ch.h"
#include "hal.h"
#include "main.h"

/**
 * @brief   Get current time in format suitable for usage in FatFS.
 *
 * param[in]  timp pointer to correctly filled time structure
 */
uint32_t rtcGetTimeFat(struct tm *timp) {
  uint32_t fattime;

  fattime  = (timp->tm_sec)       >> 1;
  fattime |= (timp->tm_min)       << 5;
  fattime |= (timp->tm_hour)      << 11;
  fattime |= (timp->tm_mday)      << 16;
  fattime |= (timp->tm_mon + 1)   << 21;
  fattime |= (timp->tm_year - 80) << 25;

  return fattime;
}

