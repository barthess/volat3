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
#include "dsp.h"

/*
 ******************************************************************************
 * EXTERNS
 ******************************************************************************
 */
extern RawData raw_data;
//extern CompensatedData comp_data;
//extern mavlink_sys_status_t mavlink_sys_status_struct;
//extern GlobalParam_t global_data[];

/*
 ******************************************************************************
 * PROTOTYPES
 ******************************************************************************
 */
void _adc_filter(adcsample_t *in, uint16_t *out);

/*
 ******************************************************************************
 * DEFINES
 ******************************************************************************
 */
#define ADC_NUM_CHANNELS          16
#define ADC_BUF_DEPTH             2
#define ADC_REST                  1024  /* "подставка" для альфа-бета фильра */

/*
 ******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************
 */
static alphabeta_instance_q31 adc_filter[ADC_NUM_CHANNELS];

static ADCConfig adccfg; // для STM32 -- должна быть пустышка

static adcsample_t samples[ADC_NUM_CHANNELS * ADC_BUF_DEPTH];

/*
 * ADC streaming callback.
 */
static void adccallback(ADCDriver *adcp, adcsample_t *samples, size_t n) {
  (void)adcp;
  (void)n;

  _adc_filter(samples, raw_data.analog);
  //palTogglePad(GPIOE, GPIOE_LED);
}

/*
 *
 */
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
  /* CR1 */
  0,
  /* CR2 */
  0,
  /* SMPR1 */
  ADC_SMPR1_SMP_AN10(ADC_SAMPLE_239P5) |
  ADC_SMPR1_SMP_AN11(ADC_SAMPLE_239P5) |
  ADC_SMPR1_SMP_AN12(ADC_SAMPLE_239P5) |
  ADC_SMPR1_SMP_AN13(ADC_SAMPLE_239P5) |
  ADC_SMPR1_SMP_AN14(ADC_SAMPLE_239P5) |
  ADC_SMPR1_SMP_AN15(ADC_SAMPLE_239P5),
  /* SMPR2 */
  ADC_SMPR2_SMP_AN0(ADC_SAMPLE_239P5)  |
  ADC_SMPR2_SMP_AN1(ADC_SAMPLE_239P5)  |
  ADC_SMPR2_SMP_AN2(ADC_SAMPLE_239P5)  |
  ADC_SMPR2_SMP_AN3(ADC_SAMPLE_239P5)  |
  ADC_SMPR2_SMP_AN4(ADC_SAMPLE_239P5)  |
  ADC_SMPR2_SMP_AN5(ADC_SAMPLE_239P5)  |
  ADC_SMPR2_SMP_AN6(ADC_SAMPLE_239P5)  |
  ADC_SMPR2_SMP_AN7(ADC_SAMPLE_239P5)  |
  ADC_SMPR2_SMP_AN8(ADC_SAMPLE_239P5)  |
  ADC_SMPR2_SMP_AN9(ADC_SAMPLE_239P5),
  /* SQR1 */
  ADC_SQR1_NUM_CH(ADC_NUM_CHANNELS) |
  ADC_SQR1_SQ16_N(ADC_CHANNEL_IN15) |
  ADC_SQR1_SQ15_N(ADC_CHANNEL_IN14) |
  ADC_SQR1_SQ14_N(ADC_CHANNEL_IN13) |
  ADC_SQR1_SQ13_N(ADC_CHANNEL_IN12),
  /* SQR2 */
  ADC_SQR2_SQ12_N(ADC_CHANNEL_IN11) |
  ADC_SQR2_SQ11_N(ADC_CHANNEL_IN10) |
  ADC_SQR2_SQ10_N(ADC_CHANNEL_IN9)  |
  ADC_SQR2_SQ9_N(ADC_CHANNEL_IN8)   |
  ADC_SQR2_SQ8_N(ADC_CHANNEL_IN7)   |
  ADC_SQR2_SQ7_N(ADC_CHANNEL_IN6),
  /* SQR3 */
  ADC_SQR3_SQ6_N(ADC_CHANNEL_IN5) |
  ADC_SQR3_SQ5_N(ADC_CHANNEL_IN4) |
  ADC_SQR3_SQ4_N(ADC_CHANNEL_IN3) |
  ADC_SQR3_SQ3_N(ADC_CHANNEL_IN2) |
  ADC_SQR3_SQ2_N(ADC_CHANNEL_IN1) |
  ADC_SQR3_SQ1_N(ADC_CHANNEL_IN0)
};

/*
 *******************************************************************************
 *******************************************************************************
 * LOCAL FUNCTIONS
 *******************************************************************************
 *******************************************************************************
 */

void _adc_filter(adcsample_t *in, adcsample_t *out){
  uint32_t i = 0;
  while (i < ADC_NUM_CHANNELS){
    out[i] = alphabeta_q31(&adc_filter[i], in[i] + ADC_REST, ) - ADC_REST;
    i++;
  }
}

/*
 *******************************************************************************
 * EXPORTED FUNCTIONS
 *******************************************************************************
 */
void ADCInitLocal(void){
  adcStart(&ADCD1, &adccfg);
  adcStartConversion(&ADCD1, &adccg, samples, ADC_BUF_DEPTH);
}


