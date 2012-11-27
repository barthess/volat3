#include "ch.h"
#include "hal.h"
#include "mavlink.h"

#include "message.h"
#include "main.h"
#include "link_dm_packer.h"
#include "link_dm_unpacker.h"

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
extern GlobalFlags_t GlobalFlags;

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
 * Поток разбора входящих данных.
 */
static WORKING_AREA(DmUnpackerThreadWA, 1024);
static msg_t DmUnpackerThread(void *sdp){
  chRegSetThreadName("DmUnpacker");

  while(GlobalFlags.messaging_ready == 0)
    chThdSleepMilliseconds(50);

  DmUnpackCycle((SerialDriver *)sdp);
  chThdExit(0);
  return 0;
}

/**
 * Упаковка данных для модуля индюкации.
 */
static WORKING_AREA(DmPackerThreadWA, 1536);
static msg_t DmPackerThread(void *sdp){
  chRegSetThreadName("DmPacker");

  while(GlobalFlags.messaging_ready == 0)
    chThdSleepMilliseconds(50);

  DmPackCycle((SerialDriver *)sdp);
  chThdExit(0);
  return 0;
}

/*
 *******************************************************************************
 * EXPORTED FUNCTIONS
 *******************************************************************************
 */

/**
 * Fork link threads for mpiovd.
 */
void link_dm_up(SerialDriver *sdp){

  chThdCreateStatic(DmPackerThreadWA,
          sizeof(DmPackerThreadWA),
          DM_THREAD_PRIO,
          DmPackerThread,
          sdp);

  chThdCreateStatic(DmUnpackerThreadWA,
          sizeof(DmUnpackerThreadWA),
          DM_THREAD_PRIO,
          DmUnpackerThread,
          sdp);
}

/**
 *
 */
bool_t dm_port_ready(void){
  if (GlobalFlags.dm_port_ready)
    return TRUE;
  else
    return FALSE;
}
