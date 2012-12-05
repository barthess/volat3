#include "ch.h"
#include "hal.h"
#include "mavlink.h"

#include "message.h"
#include "main.h"
#include "link_cc_packer.h"
#include "link_cc_unpacker.h"

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
extern Thread *link_cc_unpacker_tp;
extern Thread *link_cc_packer_tp;

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
static WORKING_AREA(CcUnpackerThreadWA, 512);
static msg_t CcUnpackerThread(void *sdp){
  chRegSetThreadName("CcUnpacker");

  while(GlobalFlags.messaging_ready == 0)
    chThdSleepMilliseconds(50);

  CcUnpackCycle((SerialDriver *)sdp);
  chThdExit(0);
  return 0;
}

/**
 * Упаковка данных для модуля индюкации.
 */
static WORKING_AREA(CcPackerThreadWA, 1536);
static msg_t CcPackerThread(void *sdp){
  chRegSetThreadName("CcPacker");

  while(GlobalFlags.messaging_ready == 0)
    chThdSleepMilliseconds(50);

  CcPackCycle((SerialDriver *)sdp);
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
void link_cc_up(SerialDriver *sdp){

  link_cc_packer_tp =  chThdCreateStatic(
      CcPackerThreadWA,
      sizeof(CcPackerThreadWA),
      CC_THREAD_PRIO,
      CcPackerThread,
      sdp);
  if (link_cc_packer_tp == NULL)
    chDbgPanic("Can not allocate memory");

  link_cc_unpacker_tp = chThdCreateStatic(
      CcUnpackerThreadWA,
      sizeof(CcUnpackerThreadWA),
      CC_THREAD_PRIO,
      CcUnpackerThread,
      sdp);
  if (link_cc_unpacker_tp == NULL)
      chDbgPanic("Can not allocate memory");
}

/**
 *
 */
bool_t cc_port_ready(void){
  if (GlobalFlags.modem_connected)
    return TRUE;
  else
    return FALSE;
}

