#ifndef DSP_H_
#define DSP_H_

#include "arm_math.h"

/**
 *
 */
typedef struct alphabeta_instance_q31 alphabeta_instance_q31;
struct alphabeta_instance_q31{
  /**
   * @brief   Accumulator.
   */
  q31_t acc;
};

q31_t alphabeta_q31(alphabeta_instance_q31 *S, q31_t val, uint32_t len);
void alphabeta_init_q31(alphabeta_instance_q31 *S, q31_t acc);

#endif /* DSP_H_ */
