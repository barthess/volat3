#include "ch.h"
#include "hal.h"

#include "main.h"
#include "bnap_ui.h"

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
extern BinarySemaphore pps_sem;

/*
 ******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************
 */

/*
 *******************************************************************************
 *******************************************************************************
 * LOCAL FUNCTIONS
 *******************************************************************************
 *******************************************************************************
 */

/**
 *
 */
static void pps_cb(EXTDriver *extp, expchannel_t channel){
  (void)extp;
  (void)channel;
  //chBSemSignalI(&pps_sem);
  if (palReadPad(IOPORT2, PIOB_PPS) == 1)
    gps_led_on();
  else
    gps_led_off();
}

//static void btn1_cb(EXTDriver *extp, expchannel_t channel){
//  (void)extp;
//  (void)channel;
//  if (palReadPad(IOPORT2, PIOB_BTN1) == 0)
//    gps_led_toggle();
//}

/**
 *
 */
static const EXTConfig extcfg_b = {
    {
        {0, NULL},//0
        {0, NULL},
        {0, NULL},
        {0, NULL},
        {0, NULL},
        {0, NULL},
        {0, NULL},
        {0, NULL},//7
        {0, NULL},
        {0, NULL},
        {0, NULL},
        {1, pps_cb},
        {0, NULL},
        {0, NULL},
        {0, NULL},
        {0, NULL},//15
        {0, NULL},
        {0, NULL},
        {0, NULL},
        {0, NULL},//{1, btn1_cb},
        {0, NULL},
        {0, NULL},
        {0, NULL},
        {0, NULL},//23
        {0, NULL},
        {0, NULL},
        {0, NULL},
        {0, NULL},
        {0, NULL},
        {0, NULL},
        {0, NULL},
        {0, NULL},//31
    },
    SAM7_EXT_MODE_RISING_EDGE,
    SAM7_EXT_PRIOR_HIGHEST - 2
};

/*
 *******************************************************************************
 * EXPORTED FUNCTIONS
 *******************************************************************************
 */

void ExtiLocalInit(void){
  extStart(&EXTDB, &extcfg_b);
  extChannelEnable(&EXTDB, PIOB_PPS);
}

/**
 * Enables interrupts from PPS from GPS receiver
 */
//void ExtiEnablePps(void){
//  extChannelEnable(&EXTDA, 0);
//}
