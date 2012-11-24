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
static WORKING_AREA(CcUnpackerThreadWA, 1024);
static msg_t CcUnpackerThread(void *sdp){
  chRegSetThreadName("CcUnpacker");

  while(GlobalFlags.messaging_ready == 0)
    chThdSleepMilliseconds(50);

  while (GlobalFlags.modem_connected == 0)
    chThdSleepMilliseconds(50);

  CcUnpackCycle((SerialDriver *)sdp);
//  while (TRUE)
//    chThdSleepMilliseconds(200);

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

  while (GlobalFlags.modem_connected == 0)
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

  chThdCreateStatic(CcUnpackerThreadWA,
          sizeof(CcUnpackerThreadWA),
          CC_THREAD_PRIO,
          CcUnpackerThread,
          sdp);

  chThdCreateStatic(CcPackerThreadWA,
          sizeof(CcPackerThreadWA),
          CC_THREAD_PRIO,
          CcPackerThread,
          sdp);
}
