#ifndef UTILS_H_
#define UTILS_H_

#include <stdlib.h>
#include "ch.h"
#include "hal.h"


#define putinrange(v, vmin, vmax){                                            \
  chDbgCheck(vmin <= vmax, "");                                               \
  if (v <= vmin)                                                              \
    v = vmin;                                                                 \
  else if (v >= vmax)                                                         \
    v = vmax;                                                                 \
}

uint64_t pack8to64(uint8_t *buf);
uint32_t pack8to32(uint8_t *buf);
uint16_t pack8to16(uint8_t *buf);
uint64_t pack32to64(uint32_t *buf);

int32_t Simpson(int32_t a, int32_t b, int32_t c, int32_t t);
int32_t Simpson38(int32_t a, int32_t b, int32_t c, int32_t d, int32_t t);

int16_t complement2signed(uint8_t msb, uint8_t lsb);
void polled_delay_us(uint32_t uS);
uint32_t isqrt(uint32_t x);

#endif /* UTILS_H_ */
