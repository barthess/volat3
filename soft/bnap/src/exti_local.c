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
static void gps_pps_cb(EXTDriver *extp, expchannel_t channel){
  (void)extp;
  (void)channel;
  chBSemSignalI(&pps_sem);
}

static void btn1_cb(EXTDriver *extp, expchannel_t channel){
  (void)extp;
  (void)channel;
  if (palReadPad(IOPORT2, 19) == 0)
    gps_led_toggle();
}

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
        {0, NULL},
        {0, NULL},
        {0, NULL},
        {0, NULL},
        {0, NULL},//15
        {0, NULL},
        {0, NULL},
        {0, NULL},
        {1, btn1_cb},
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
  extChannelEnable(&EXTDB, 19);
}

/**
 * Enables interrupts from PPS from GPS receiver
 */
//void ExtiEnablePps(void){
//  extChannelEnable(&EXTDA, 0);
//}
