#include "ch.h"
#include "hal.h"

#include "discrete.h"
#include "sensors.h"
#include "param.h"

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
extern RawData raw_data;
extern CompensatedData comp_data;

/*
 ******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************
 */
/* указатели на коэффициенты */
static float *rel_0_31, *rel_32_63;

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

/* perform inversion of neede bits */
uint64_t normalize(uint64_t raw, uint32_t c1, uint32_t c2){
  uint64_t coeffs = c2;
  coeffs = (coeffs << 32) | c1;
  return raw ^ coeffs;
}

/**
 * Поток, принимающий и обрабатывающий дискретные данные.
 */
static WORKING_AREA(DiscreteThreadWA, 128);
static msg_t DiscreteThread(void* arg){
  chRegSetThreadName("Discrete");
  (void)arg;

  while (TRUE) {
    chThdSleepMilliseconds(100);
    raw_data.discrete = 0;
    comp_data.discrete = normalize(raw_data.discrete,
                                   *(uint32_t *)rel_0_31,
                                   *(uint32_t *)rel_32_63);
  }

  return 0;
}


/*
 ******************************************************************************
 * EXPORTED FUNCTIONS
 ******************************************************************************
 */

void DiscreteInitLocal(void){

  rel_0_31  = ValueSearch("REL_0_31");
  rel_32_63 = ValueSearch("REL_32_63");

  chThdCreateStatic(DiscreteThreadWA,
        sizeof(DiscreteThreadWA),
        NORMALPRIO,
        DiscreteThread,
        NULL);
}



