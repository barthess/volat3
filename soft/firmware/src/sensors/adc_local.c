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

/*
 ******************************************************************************
 * PROTOTYPES
 ******************************************************************************
 */
static void _adc_filter(adcsample_t *in, uint16_t *out);

/*
 ******************************************************************************
 * DEFINES
 ******************************************************************************
 */
#define ADC_NUM_CHANNELS          16
#define ADC_BUF_DEPTH             2
#define ADC_REST                  1024  /* "подставка" для альфа-бета фильра */
#define ADC_FILTER_LEN            4

#define ADC_SUPPLY_VOLTAGE        (raw_data.analog[0])

/*
 ******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************
 */
static int32_t *c1[ADC_NUM_CHANNELS];
static int32_t *c2[ADC_NUM_CHANNELS];
static int32_t *c3[ADC_NUM_CHANNELS];
static uint32_t *flen[ADC_NUM_CHANNELS];

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

/**
 * Pass all ADC samples through array of filters
 */
static void _adc_filter(adcsample_t *in, adcsample_t *out){
  uint32_t i = 0;
  while (i < ADC_NUM_CHANNELS){
    out[i] = alphabeta_q31(&adc_filter[i], in[i] + ADC_REST, *flen[i]) - ADC_REST;
    i++;
  }
}

/**
 * Calculate real values from volt comepsated based on polinomial approximation.
 */
static adcsample_t _normalize(adcsample_t in, int32_t c1, int32_t c2, int32_t c3){
  return (adcsample_t)((c1*in*in + c2*in + c3) & 0xFFFF);
}

/**
 *
 */
static void load_params(void){
  c1[0]  = ValueSearch("AN_ch00_c1");
  c1[1]  = ValueSearch("AN_ch01_c1");
  c1[2]  = ValueSearch("AN_ch02_c1");
  c1[3]  = ValueSearch("AN_ch03_c1");
  c1[4]  = ValueSearch("AN_ch04_c1");
  c1[5]  = ValueSearch("AN_ch05_c1");
  c1[6]  = ValueSearch("AN_ch06_c1");
  c1[7]  = ValueSearch("AN_ch07_c1");
  c1[8]  = ValueSearch("AN_ch08_c1");
  c1[9]  = ValueSearch("AN_ch09_c1");
  c1[10] = ValueSearch("AN_ch10_c1");
  c1[11] = ValueSearch("AN_ch11_c1");
  c1[12] = ValueSearch("AN_ch12_c1");
  c1[13] = ValueSearch("AN_ch13_c1");
  c1[14] = ValueSearch("AN_ch14_c1");
  c1[15] = ValueSearch("AN_ch15_c1");

  c2[0]  = ValueSearch("AN_ch00_c2");
  c2[1]  = ValueSearch("AN_ch01_c2");
  c2[2]  = ValueSearch("AN_ch02_c2");
  c2[3]  = ValueSearch("AN_ch03_c2");
  c2[4]  = ValueSearch("AN_ch04_c2");
  c2[5]  = ValueSearch("AN_ch05_c2");
  c2[6]  = ValueSearch("AN_ch06_c2");
  c2[7]  = ValueSearch("AN_ch07_c2");
  c2[8]  = ValueSearch("AN_ch08_c2");
  c2[9]  = ValueSearch("AN_ch09_c2");
  c2[10] = ValueSearch("AN_ch10_c2");
  c2[11] = ValueSearch("AN_ch11_c2");
  c2[12] = ValueSearch("AN_ch12_c2");
  c2[13] = ValueSearch("AN_ch13_c2");
  c2[14] = ValueSearch("AN_ch14_c2");
  c2[15] = ValueSearch("AN_ch15_c2");

  c3[0]  = ValueSearch("AN_ch00_c3");
  c3[1]  = ValueSearch("AN_ch01_c3");
  c3[2]  = ValueSearch("AN_ch02_c3");
  c3[3]  = ValueSearch("AN_ch03_c3");
  c3[4]  = ValueSearch("AN_ch04_c3");
  c3[5]  = ValueSearch("AN_ch05_c3");
  c3[6]  = ValueSearch("AN_ch06_c3");
  c3[7]  = ValueSearch("AN_ch07_c3");
  c3[8]  = ValueSearch("AN_ch08_c3");
  c3[9]  = ValueSearch("AN_ch09_c3");
  c3[10] = ValueSearch("AN_ch10_c3");
  c3[11] = ValueSearch("AN_ch11_c3");
  c3[12] = ValueSearch("AN_ch12_c3");
  c3[13] = ValueSearch("AN_ch13_c3");
  c3[14] = ValueSearch("AN_ch14_c3");
  c3[15] = ValueSearch("AN_ch15_c3");

  flen[0]  = ValueSearch("AN_ch00_flen");
  flen[1]  = ValueSearch("AN_ch01_flen");
  flen[2]  = ValueSearch("AN_ch02_flen");
  flen[3]  = ValueSearch("AN_ch03_flen");
  flen[4]  = ValueSearch("AN_ch04_flen");
  flen[5]  = ValueSearch("AN_ch05_flen");
  flen[6]  = ValueSearch("AN_ch06_flen");
  flen[7]  = ValueSearch("AN_ch07_flen");
  flen[8]  = ValueSearch("AN_ch08_flen");
  flen[9]  = ValueSearch("AN_ch09_flen");
  flen[10] = ValueSearch("AN_ch10_flen");
  flen[11] = ValueSearch("AN_ch11_flen");
  flen[12] = ValueSearch("AN_ch12_flen");
  flen[13] = ValueSearch("AN_ch13_flen");
  flen[14] = ValueSearch("AN_ch14_flen");
  flen[15] = ValueSearch("AN_ch15_flen");
}

/**
 * Compensate input supply value changes.
 */
static adcsample_t _supply_compensate(adcsample_t in){
  uint32_t v = in;
  v = v << 14;
  v = v / ADC_SUPPLY_VOLTAGE;
  v = __USAT(v, 16);
  return v;
}

/**
 * Return millivolts of board supply voltage.
 * 30000mV = 3196 ADC counts
 */
static uint16_t get_board_voltage(adcsample_t in){
  return ((30000UL * (uint32_t)in) / 3196) & 0xFFFF;
}

/**
 * Return ДУМП-100 value
 */
static uint16_t get_dump100(adcsample_t in){
  return in;
}

/*
 *******************************************************************************
 * EXPORTED FUNCTIONS
 *******************************************************************************
 */

void ADCInitLocal(void){
  load_params();
  adcStart(&ADCD1, &adccfg);
  adcStartConversion(&ADCD1, &adccg, samples, ADC_BUF_DEPTH);
}

/**
 *
 */
void adc_process(adcsample_t *in, mavlink_mpiovd_sensors_raw_t *raw){
  raw->analog00 = get_board_voltage(ADC_SUPPLY_VOLTAGE);

  raw->analog01 = get_dump100(_supply_compensate(in[1]));
  //raw->analog01 = _normalize(_supply_compensate(in[1]),  *c1[1],  *c2[1],  *c3[1]);
//  raw->analog02 = _normalize(_supply_compensate(in[2]),  *c1[2],  *c2[2],  *c3[2]);
//  raw->analog03 = _normalize(_supply_compensate(in[3]),  *c1[3],  *c2[3],  *c3[3]);
//  raw->analog04 = _normalize(_supply_compensate(in[4]),  *c1[4],  *c2[4],  *c3[4]);
//  raw->analog05 = _normalize(_supply_compensate(in[5]),  *c1[5],  *c2[5],  *c3[5]);
//  raw->analog06 = _normalize(_supply_compensate(in[6]),  *c1[6],  *c2[6],  *c3[6]);
//  raw->analog07 = _normalize(_supply_compensate(in[7]),  *c1[7],  *c2[7],  *c3[7]);
//  raw->analog08 = _normalize(_supply_compensate(in[8]),  *c1[8],  *c2[8],  *c3[8]);
//  raw->analog09 = _normalize(_supply_compensate(in[9]),  *c1[9],  *c2[8],  *c3[9]);
//  raw->analog10 = _normalize(_supply_compensate(in[10]), *c1[10], *c2[10], *c3[10]);
//  raw->analog11 = _normalize(_supply_compensate(in[11]), *c1[11], *c2[11], *c3[11]);
//  raw->analog12 = _normalize(_supply_compensate(in[12]), *c1[12], *c2[12], *c3[12]);
//  raw->analog13 = _normalize(_supply_compensate(in[13]), *c1[13], *c2[13], *c3[13]);
//  raw->analog14 = _normalize(_supply_compensate(in[14]), *c1[14], *c2[14], *c3[14]);
//  raw->analog15 = _normalize(_supply_compensate(in[15]), *c1[15], *c2[15], *c3[15]);
}


