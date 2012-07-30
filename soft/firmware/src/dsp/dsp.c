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
  *S = *S - (*S >> len) + val;
  return *S >> len;
}





