#include "ch.h"
#include "arm_math.h"

#ifndef DSP_H_
#define DSP_H_

#define BLOCK_SIZE    4

/* размер кучи под FIR */
#define FIR_HEAP_SIZE    1024

void DspInit(void);

#endif /* DSP_H_ */
