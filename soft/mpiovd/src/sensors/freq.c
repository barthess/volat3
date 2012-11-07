#include "ch.h"
#include "hal.h"

#include "freq.h"
#include "sensors.h"
#include "utils.h"

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

/*
 ******************************************************************************
 * PROTOTYPES
 ******************************************************************************
 */
static void icuperiodcb_tacho(ICUDriver *icup);
static void icuoverflowcb_tacho(ICUDriver *icup);

/*
 ******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************
 */
static ICUConfig icucfg_tacho = {
  ICU_INPUT_ACTIVE_HIGH,
  200000,                                    /* 200kHz ICU clock frequency.   */
  NULL,
  icuperiodcb_tacho,
  icuoverflowcb_tacho,
  ICU_CHANNEL_1
};

/*
 ******************************************************************************
 ******************************************************************************
 * LOCAL FUNCTIONS
 ******************************************************************************
 ******************************************************************************
 */
static void icuperiodcb_tacho(ICUDriver *icup) {
  raw_data.engine_rpm = icuGetPeriod(icup);
}

static void icuoverflowcb_tacho(ICUDriver *icup) {
  (void)icup;
  raw_data.engine_rpm = 0;
}

/*
 ******************************************************************************
 * EXPORTED FUNCTIONS
 ******************************************************************************
 */

void FreqInit(void){
  icuStart(&ICUD4, &icucfg_tacho);
  icuEnable(&ICUD4);
}

/**
 * Calculate revolutions per minute from raw counter value.
 */
uint16_t get_engine_rpm(void){
  uint32_t rpm = (60UL * icucfg_tacho.frequency) / raw_data.engine_rpm;
  putinrange(rpm, 0, 0xFFFF);
  return rpm;
}
