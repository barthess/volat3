#include <math.h>

#include "ch.h"
#include "hal.h"

#include "adc_local.h"
#include "sensors.h"
#include "message.h"
#include "main.h"
#include "utils.h"
#include "link.h"
#include "param.h"

/*
 ******************************************************************************
 * EXTERNS
 ******************************************************************************
 */
extern RawData raw_data;
extern CompensatedData comp_data;
extern mavlink_sys_status_t mavlink_sys_status_struct;
extern GlobalParam_t global_data[];

/*
 ******************************************************************************
 * DEFINES
 ******************************************************************************
 */
#define DEFAULT_CURRENT_COEFF     1912   // коэффициент пересчета из условных единиц в амперы для саломёта -- 37, для машинки -- 1912
#define DEFAULT_CURRENT_OFFSET    16   // смещение нуля датчика тока в единицах АЦП
#define DEFAULT_VOLTAGE_COEFF     1022 // коэффициент пересчета из условных единиц в децывольты

#define ADC_NUM_CHANNELS          6
#define ADC_BUF_DEPTH             1

/* человекочитабельные названия каналов */
#define ADC_CURRENT_SENS          ADC_CHANNEL_IN10
#define ADC_MAIN_SUPPLY           ADC_CHANNEL_IN11
#define ADC_6V_SUPPLY             ADC_CHANNEL_IN12
#define ADC_AN33_0                ADC_CHANNEL_IN13
#define ADC_AN33_1                ADC_CHANNEL_IN14
#define ADC_AN33_2                ADC_CHANNEL_IN15

// где лежат текущие значения АЦП
#define ADC_CURRENT_SENS_OFFSET   (ADC_CHANNEL_IN10 - 10)
#define ADC_MAIN_SUPPLY_OFFSET    (ADC_CHANNEL_IN11 - 10)
#define ADC_6V_SUPPLY_OFFSET      (ADC_CHANNEL_IN12 - 10)
#define ADC_AN33_0_OFFSET         (ADC_CHANNEL_IN13 - 10)
#define ADC_AN33_1_OFFSET         (ADC_CHANNEL_IN14 - 10)
#define ADC_AN33_2_OFFSET         (ADC_CHANNEL_IN15 - 10)

#define PWR_CHECK_PERIOD          10 /* mS */

/*
 ******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************
 */

static ADCConfig adccfg; // для STM32 -- должна быть пустышка

static adcsample_t samples[ADC_NUM_CHANNELS * ADC_BUF_DEPTH];

/*
 * ADC streaming callback.
 */
static void adccallback(ADCDriver *adcp, adcsample_t *samples, size_t n) {
  (void)adcp;
  (void)samples;
  (void)n;
}

static void adcerrorcallback(ADCDriver *adcp, adcerror_t err) {
  (void)adcp;
  (void)err;
}

/*
 * ADC conversion group.
 */
static const ADCConversionGroup adccg = {
  TRUE,
  ADC_NUM_CHANNELS,
  adccallback,
  adcerrorcallback,
  0,                        /* CR1 */
  ADC_CR2_SWSTART,          /* CR2 */
  ADC_SMPR1_SMP_AN10(ADC_SAMPLE_239P5) |
  ADC_SMPR1_SMP_AN11(ADC_SAMPLE_239P5) |
  ADC_SMPR1_SMP_AN12(ADC_SAMPLE_239P5) |
  ADC_SMPR1_SMP_AN13(ADC_SAMPLE_239P5) |
  ADC_SMPR1_SMP_AN14(ADC_SAMPLE_239P5) |
  ADC_SMPR1_SMP_AN15(ADC_SAMPLE_239P5),
  0,                        /* SMPR2 */
  ADC_SQR1_NUM_CH(ADC_NUM_CHANNELS),
  0,
  (ADC_SQR3_SQ6_N(ADC_AN33_2)         | ADC_SQR3_SQ5_N(ADC_AN33_1) |
      ADC_SQR3_SQ4_N(ADC_AN33_0)      | ADC_SQR3_SQ3_N(ADC_6V_SUPPLY) |
      ADC_SQR3_SQ2_N(ADC_MAIN_SUPPLY) | ADC_SQR3_SQ1_N(ADC_CURRENT_SENS))
};

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
void ADCInitLocal(void){
  adcStart(&ADCD1, &adccfg);
  adcStartConversion(&ADCD1, &adccg, samples, ADC_BUF_DEPTH);
}


