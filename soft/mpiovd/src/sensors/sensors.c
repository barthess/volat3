#include "ch.h"
#include "hal.h"

#include "sensors.h"
#include "message.h"

#include "adc_local.h"
#include "exti_local.h"
#include "spi_local.h"
#include "discrete.h"
#include "freq.h"

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
RawData raw_data;                     /* структура с сырыми данными с датчиков */
CompensatedData comp_data;            /* обработанные данные */

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
void SensorsInit(void){

  ExtiInitLocal();
  ADCInitLocal();
  SpiInitLocal();
  DiscreteInitLocal();
  FreqInit();
  /* start I2C sensors */
}

