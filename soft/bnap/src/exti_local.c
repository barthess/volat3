#include "ch.h"
#include "hal.h"

#include "main.h"

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

/**
 *
 */
static const EXTConfig extcfg_b = {
    {
        {1, NULL},
        {1, NULL},
        {1, NULL},
        {1, NULL},
        {1, NULL},
        {1, NULL},
        {1, NULL},
        {1, NULL},
        {1, NULL},
        {1, NULL},
        {1, NULL},
        {1, gps_pps_cb},
        {1, NULL},
        {1, NULL},
        {1, NULL},
        {1, NULL},
        {1, NULL},
        {1, NULL},
        {1, NULL},
        {1, NULL},
        {1, NULL},
        {1, NULL},
        {1, NULL},
        {1, NULL},
        {1, NULL},
        {1, NULL},
        {1, NULL},
        {1, NULL},
        {1, NULL},
        {1, NULL},
        {1, NULL},
        {1, NULL},
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
//  extStart(&EXTDB, &extcfg_b);
//  extChannelEnable(&EXTDB, 11);
}

/**
 * Enables interrupts from PPS from GPS receiver
 */
//void ExtiEnablePps(void){
//  extChannelEnable(&EXTDA, 0);
//}
