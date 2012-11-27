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
 * Упаковка данных для модуля индюкации.
 */
static WORKING_AREA(DmPackerThreadWA, 1536);
static msg_t DmPackerThread(void *sdp){
  chRegSetThreadName("DmPacker");
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
