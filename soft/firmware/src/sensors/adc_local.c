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
#define ADC_REST                  4096  /* "подставка" для альфа-бета фильра */

#define adc_raw_voltage           (raw_data.analog[0]) // alias

/*
 ******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************
 */
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
 * Compensate input supply value changes.
 *
 * @return    resistance of sensor connected to input.
 */
#define ADC_TO_VOLTAGE  0.00079552716f  // коэффициент пересчета из условных единиц в напряжение непосредственно на входе АЦП
#define Vcoeff  (24.0f / 2693.0f)       // коэффициент пересчета условных единиц АЦП в вольты
#define R0  1000.0f                     // сопротивление R26
#define R1  100000.0f                   // сопротивление R34
#define R2  10000.0f                    // сопротивление R42

static uint16_t _supply_compensate(adcsample_t in){
  float U1; // напряжение в точке соединения входного резистора R26 и сопротивления датчика
  float U2; // напражение на входе АЦП
  float Ud; // падение напряжения на диоде VD12
  float R;  // измерянное сопротивление датчика
  float V;  // напряжение, приходящее на резистор R26 (уже после защитных диодов)

  U2 = (float)in * ADC_TO_VOLTAGE;
  Ud = 0.037 * logf(250*U2 + 1);  // формула аппроксимации характеристики диода VD12
  U1 = (U2 * (R1 + R2) + Ud * R2) / R2;
  V = (float)adc_raw_voltage * Vcoeff;
  R = (U1 * R0) / (V - U1);
  putinrange(R, 0, 1000.0f);
  return roundf(R);
}

/**
 *
 */
static void load_params(void){
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
 * Return millivolts of board supply voltage.
 * 24V = 2585
 * Необходимо учесть падение на защитном диоде и не менее защитном транзисторе.
 * Это падение напрямую зависит от суммы сопротивлений подключенных датчиков,
 * от которых, в свою очередь, зависит сумма измеренных значений.
 * Снимаем 2 точки и выводим уравнение прямой:
 * 2563  -  1660 mV
 * 41429 -  1010 mV
 *
 * y - y1    x - x1
 * ------- = -------
 * y2 - y1   x2 - x1
 *
 *     (x - x1) * (y2 - y1)
 * y = -------------------  + y1
 *            x2 - x1
 */
static uint16_t get_board_voltage(adcsample_t in){
  int32_t voltdrop; /* uV */
  const uint32_t ref_drop = 1012000; /* uV */
  const uint32_t ref_voltage = 24 * 1000000; /* uV */
  const uint32_t ref_adc = 2585;

  /* voltage compensation coefficients */
  const int32_t x1 = 2563;
  const int32_t x2 = 41429;
  const int32_t y1 = 1660;
  const int32_t y2 = 1010;

  int32_t s = 0;
  uint32_t i = ADC_NUM_CHANNELS;
  while (i){
    s += raw_data.analog[i-1];
    i--;
  }

  putinrange(s, x1, x2);
  voltdrop = y1 + ((s - x1) * (y2 - y1)) / (x2 - x1); // mV
  voltdrop *= 1000; // uV

  /* рассчитываем коэффициент перевода из условных единиц */
  uint32_t k = (ref_voltage - ref_drop) / ref_adc;

  return __USAT(((k * (uint32_t)in + voltdrop) / 1000), 15);
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

  //raw->analog00 = adc_raw_voltage;
  //raw->analog01 = raw_data.analog[1];

  raw->analog00 = get_board_voltage(adc_raw_voltage);

  raw->analog01 = _supply_compensate(in[1]);
  raw->analog02 = _supply_compensate(in[2]); 
  raw->analog03 = _supply_compensate(in[3]); 
  raw->analog04 = _supply_compensate(in[4]); 
  raw->analog05 = _supply_compensate(in[5]); 
  raw->analog06 = _supply_compensate(in[6]); 
  raw->analog07 = _supply_compensate(in[7]); 
  raw->analog08 = _supply_compensate(in[8]); 
  raw->analog09 = _supply_compensate(in[9]); 
  raw->analog10 = _supply_compensate(in[10]);
  raw->analog11 = _supply_compensate(in[11]);
  raw->analog12 = _supply_compensate(in[12]);
  raw->analog13 = _supply_compensate(in[13]);
  raw->analog14 = _supply_compensate(in[14]);
  raw->analog15 = _supply_compensate(in[15]);
}


