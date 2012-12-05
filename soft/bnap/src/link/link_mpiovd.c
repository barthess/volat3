#include "ch.h"
#include "hal.h"
#include "mavlink.h"

#include "message.h"
#include "main.h"
#include "link_mpiovd_unpacker.h"

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
extern Thread *link_mpiovd_unpacker_tp;

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
static WORKING_AREA(MpiovdUnpackerThreadWA, 512);
static msg_t MpiovdUnpackerThread(void *sdp){
  chRegSetThreadName("MpiovdUnpacker");

  while(GlobalFlags.messaging_ready == 0)
    chThdSleepMilliseconds(50);

  MpiovdUnpackCycle((SerialDriver *)sdp);
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
void link_mpiovd_up(SerialDriver *sdp){

  link_mpiovd_unpacker_tp = chThdCreateStatic(
      MpiovdUnpackerThreadWA,
      sizeof(MpiovdUnpackerThreadWA),
      MPIOVD_THREAD_PRIO,
      MpiovdUnpackerThread,
      sdp);
  if (link_mpiovd_unpacker_tp == NULL)
      chDbgPanic("Can not allocate memory");
}

/**
 *
 */
bool_t mpiovd_port_ready(void){
  if (GlobalFlags.mpiovd_port_ready)
    return TRUE;
  else
    return FALSE;
}
