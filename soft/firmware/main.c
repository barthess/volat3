#include "ch.h"
#include "hal.h"

#include "link.h"
#include "message.h"
#include "main.h"
#include "sanity.h"
#include "board.h"

/* heap for (link threads) OR (shell thread)*/
MemoryHeap LinkThdHeap;
static uint8_t link_thd_buf[LINK_THD_HEAP_SIZE + sizeof(stkalign_t)];

/*
 * Application entry point.
 */
int main(void) {

  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  halInit();
  chSysInit();

  /* инициализация кучи под всякие нужные вещи */
  chHeapInit(&LinkThdHeap, (uint8_t *)MEM_ALIGN_NEXT(link_thd_buf), LINK_THD_HEAP_SIZE);

  MsgInit();
  MavInit();
  LinkInit();
  SanityControlInit();

  /*
   * Normal main() thread activity, in this demo it does nothing except
   * sleeping in a loop and check the button state.
   */
  chThdSleepMilliseconds(2000);

  while (TRUE) {
    chThdSleepMilliseconds(50);

//    mavlink_msg_mpiovd_sensors_raw_encode(mavlink_system.sysid, mavlink_system.compid, &msg, &raw);
//    len = mavlink_msg_to_send_buffer(buf, &msg);
//    sdWrite(&SDU1, buf, len);
  }

  while (TRUE) {
    chThdSleepMilliseconds(666);
  }
  return 0;
}


