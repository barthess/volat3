#include "ch.h"
#include "hal.h"

#include "dsp.h"

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
 ******************************************************************************
 ******************************************************************************
 * LOCAL FUNCTIONS
 ******************************************************************************
 ******************************************************************************
 */

/*
 ******************************************************************************
 * EXPORTED FUNCTIONS
 ******************************************************************************
 */

/**
 * @brief   Do alpha beta filtering.
 * @note    There is no saturation mechanism! Accumulator will wrap over zero
 *          if overflows.
 */
q31_t alphabeta_q31(alphabeta_instance_q31 *S, q31_t val, uint32_t len){
  S->acc -= S->acc >> len;
  S->acc += val;
  return S->acc >> len;
}

/**
 * @brief   Perform init.
 * @param[in] *S points to an instance of the filter structure.
 * @param[in] len is length of filter.
 * @param[in] acc initial value of accumulator.
 */
void alphabeta_init_q31(alphabeta_instance_q31 *S, q31_t acc){
  S->acc = acc;
}





