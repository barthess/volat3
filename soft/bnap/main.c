/* обязательно */
// TODO: запрос качества связи перед коннектом модема и посылка этого значения
// TODO: при каждой записи в хранилище обновлять время последней доступной записи (in RAM) (?? и общее количество??)
// TODO: пакет для запроса накопленных данных и прога для его демонстрации.
// TODO: дополнительный пакет "алярма" со всей хуйнёй одним махом (битовыми полями)

/* хорошо бы, при наличии времени */
// TODO: passing PIN to modem during initialization.
// TODO: дополнительная проверка времени после поиска в массиве хранилища последней метки времени (текущее время должно быть больше)
// TODO: узнать, кто еще читает-пишет в порты и мешает нормальному функционированию cross.
// TODO: прошивалку модема через cross.
// TODO: protect parameter struct using mutex or semaphore to avoid corruption during multithreaded access
// TODO: дополонительные команды для модемного апплета (reboot, dial, etc.)

/* не в этой жизни */
// TODO: проброс параметров от остальных компонентов системы на ДЦ
// TODO: rewrite GPS code using nmeap
// TODO: более высокая точность парсинга координат gps.
// TODO: EXTI (see todos in TimekeeperThread and gps_get_time())
// TODO: обработка битых блоков хранилища

#include <time.h>

#include "ch.h"
#include "hal.h"
#include "mavlink.h"
#include "chprintf.h"

#include "main.h"
#include "sensors.h"
#include "message.h"
#include "i2c_local.h"
#include "timekeeper.h"
#include "sanity.h"
#include "gps.h"
#include "link.h"
#include "ds1338.h"
#include "exti_local.h"
#include "storage.h"
#include "wavecom.h"
#include "bnap_ui.h"
#include "param.h"
#include "cross.h"
#include "mavcmd_local.h"
#include "eeprom_testsuit.h"
#include "settings_modem.h"

/*
 ******************************************************************************
 * EXTERNS
 ******************************************************************************
 */

RawData raw_data;

CompensatedData comp_data;

/* RTC-GPS sync */
BinarySemaphore pps_sem;

/* store here time from GPS */
struct tm gps_timp;

/* some global flags */
GlobalFlags_t GlobalFlags = {0,0,0,0,0,0,0,0,
                             0,0,0,0,0,0,0,0,
                             0,0,0,0,0,0,0,0,
                             0,0,0,0,0,0,0,0};

/* heap for some threads */
MemoryHeap ThdHeap;
static uint8_t link_thd_buf[THREAD_HEAP_SIZE + sizeof(stkalign_t)];

/*
 ******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************
 */
/**/
static const SerialConfig gsm_ser_cfg = {
    GSM_BAUDRATE,
//    AT91C_US_USMODE_HWHSH | AT91C_US_CLKS_CLOCK | AT91C_US_CHRL_8_BITS |
//                              AT91C_US_PAR_NONE | AT91C_US_NBSTOP_1_BIT
      AT91C_US_USMODE_NORMAL | AT91C_US_CLKS_CLOCK | AT91C_US_CHRL_8_BITS |
                                AT91C_US_PAR_NONE | AT91C_US_NBSTOP_1_BIT
};
static const SerialConfig dm_ser_cfg = {
    DM_BAUDRATE,
    AT91C_US_USMODE_NORMAL | AT91C_US_CLKS_CLOCK | AT91C_US_CHRL_8_BITS |
                              AT91C_US_PAR_NONE | AT91C_US_NBSTOP_1_BIT
};
static const SerialConfig mpiovd_ser_cfg = {
    MPIOVD_BAUDRATE,
    AT91C_US_USMODE_NORMAL | AT91C_US_CLKS_CLOCK | AT91C_US_CHRL_8_BITS |
                              AT91C_US_PAR_NONE | AT91C_US_NBSTOP_1_BIT
};

/*
 *******************************************************************************
 *******************************************************************************
 * LOCAL FUNCTIONS
 *******************************************************************************
 *******************************************************************************
 */

int main(void) {
  halInit();
  chSysInit();

  chHeapInit(&ThdHeap, (uint8_t *)MEM_ALIGN_NEXT(link_thd_buf), THREAD_HEAP_SIZE);

  gsm_release_reset();
  gps_led_off();
  gsm_led_off();

  chBSemInit(&pps_sem, TRUE);

  sdStart(&SDGSM, &gsm_ser_cfg);
  sdStart(&SDDM, &dm_ser_cfg);
  setGlobalFlag(GlobalFlags.dm_port_ready);
  sdStart(&SDMPIOVD, &mpiovd_ser_cfg);
  setGlobalFlag(GlobalFlags.mpiovd_port_ready);

  i2cLocalInit();//0xFFFB8000
  //EepromTestThread(&SDDM);
  MsgInit();        /* init event sources */
  ParametersInit(); /* need events for proper functionality */
  MavInit();        /* set device IDs previusly red from from EEPROM byt param init*/
  GPSInit();
  LinkInit();
  ds1338Init();
  TimekeeperInit();
  StorageInit();
  SanityControlInit();
  ModemSettingsInit();
  ModemInit();
//  ModemCrossInit();
  MavCmdInitLocal();
  UiInit();

//  ExtiLocalInit();
  while (TRUE) {
    chThdSleepMilliseconds(1000);
  }
  return 0;
}
