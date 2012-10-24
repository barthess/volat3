//TODO:  удалить из SPI режим CPOL=1,CPHA=0 и объявить в эррате первый и последний бит неработающими


#include "ch.h"
#include "hal.h"

#include "link.h"
#include "message.h"
#include "main.h"
#include "sanity.h"
#include "board.h"
#include "eeprom.h"
#include "i2c_local.h"
#include "sensors.h"
#include "autopilot.h"
#include "dsp.h"


/* heap for (link threads) OR (shell thread)*/
MemoryHeap ThdHeap;
static uint8_t link_thd_buf[LINK_THD_HEAP_SIZE + sizeof(stkalign_t)];

uint32_t GlobalFlags;

/*
 * Application entry point.
 */
int main(void) {
  halInit();
  chSysInit();

  /* инициализация кучи под всякие нужные и не очень вещи */
  chHeapInit(&ThdHeap, (uint8_t *)MEM_ALIGN_NEXT(link_thd_buf), LINK_THD_HEAP_SIZE);

  GlobalFlags = 0;

  MsgInit();
  MavInit();
  I2CInitLocal();
  LinkInit();
  SanityControlInit();
  SensorsInit();
  AutopilotInit();

  while (TRUE) {
    chThdSleepMilliseconds(666);
  }
  return 0;
}


