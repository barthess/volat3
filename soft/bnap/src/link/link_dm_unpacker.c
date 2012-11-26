#include "link_dm_unpacker.h"

/*
 ******************************************************************************
 * EXTERNS
 ******************************************************************************
 */

/*
 *******************************************************************************
 * EXPORTED FUNCTIONS
 *******************************************************************************
 */

/**
 *
 */
void DmUnpackCycle(SerialDriver *sdp){
  mavlink_message_t msg;
  mavlink_status_t status;
  msg_t c = 0;

  while (!chThdShouldTerminate()) {
    (void)msg;
    (void)status;
    (void)c;
    (void)sdp;
    chThdSleepMilliseconds(100);
  }
}
