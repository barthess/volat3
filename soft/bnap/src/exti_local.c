#include "ch.h"
#include "hal.h"

#include "main.h"
#include "bnap_ui.h"
#include "timekeeper.h"

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
 *******************************************************************************
 *******************************************************************************
 * LOCAL FUNCTIONS
 *******************************************************************************
 *******************************************************************************
 */

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
        {1, exti_pps_cb},
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
    SAM7_EXT_PRIOR_HIGHEST
};

/*
 *******************************************************************************
 * EXPORTED FUNCTIONS
 *******************************************************************************
 */

void ExtiLocalInit(void){
  extStart(&EXTDB, &extcfg_b);
}

/**
 * Enables interrupts from PPS from GPS receiver
 */
void ExtiEnablePps(void){
  extChannelEnable(&EXTDB, PIOB_PPS);
}
