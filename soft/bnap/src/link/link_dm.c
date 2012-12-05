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
extern MemoryHeap ThdHeap;
extern Thread *link_dm_unpacker_tp;
extern Thread *link_dm_packer_tp;

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
static WORKING_AREA(DmUnpackerThreadWA, 512);
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
void spawn_dm_threads(SerialDriver *sdp){

  link_dm_packer_tp = chThdCreateFromHeap(
      &ThdHeap,
      sizeof(DmPackerThreadWA),
      DM_THREAD_PRIO,
      DmPackerThread,
      sdp);
  if (link_dm_packer_tp == NULL)
    chDbgPanic("Can not allocate memory");

  link_dm_unpacker_tp = chThdCreateFromHeap(
      &ThdHeap,
      sizeof(DmUnpackerThreadWA),
      DM_THREAD_PRIO,
      DmUnpackerThread,
      sdp);
  if (link_dm_unpacker_tp == NULL)
    chDbgPanic("Can not allocate memory");
}

/**
 *
 */
void kill_dm_threads(void){
  chThdTerminate(link_dm_packer_tp);
  chThdTerminate(link_dm_unpacker_tp);

  chThdWait(link_dm_packer_tp);
  chThdWait(link_dm_unpacker_tp);
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


