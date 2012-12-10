/* обязательно */
// TODO: протестировать коррекцию при убегании времени RTC вперед

/* хорошо бы, при наличии времени */
// TODO: субкомманда для хранилища stat
// TODO: дополнительный пакет "алярма" со всей хуйнёй одним махом в виде битовых полей. На текущий момент его заменяет текстовое сообщение при нажатии на кнопку Алярма.
// TODO: прошивка сегфолтится почему-то с включенной оптимизацией

/* не в этой жизни */
// TODO: дополонительные команды для модемного апплета (reset, dial, etc.)
// TODO: protect parameter struct using mutex or semaphore to avoid corruption during multithreaded access
// TODO: как-то нестабильно работает писалка параметров в EEPROM на скорости 263 kHz.
// TODO: возможность прервать выдачу накопленных данных
// TODO: проброс параметров от остальных компонентов системы на ДЦ
// TODO: rewrite GPS code using nmeap
// TODO: более высокая точность парсинга координат gps.
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
#include "wavecom.h"
#include "bnap_ui.h"
#include "param.h"
#include "cross.h"
#include "mavcmd_local.h"
#include "eeprom_testsuit.h"
#include "settings_modem.h"
#include "microsd.h"
#include "storage.h"

/*
 ******************************************************************************
 * EXTERNS
 ******************************************************************************
 */

/* threads' handlers */
Thread *link_cc_unpacker_tp     = NULL;
Thread *link_cc_packer_tp       = NULL;
Thread *link_dm_unpacker_tp     = NULL;
Thread *link_dm_packer_tp       = NULL;
Thread *link_mpiovd_unpacker_tp = NULL;
Thread *modem_tp                = NULL;
Thread *rssi_tp                 = NULL;
Thread *microsd_reader_cc_tp    = NULL;
Thread *microsd_reader_dm_tp    = NULL;
Thread *microsd_writer_tp       = NULL;
Thread *gps_tp                  = NULL;

RawData raw_data;

CompensatedData comp_data;

/* RTC-GPS sync */
BinarySemaphore pps_sem;

/* Semaphores for mutual access to GSM and DM output */
BinarySemaphore cc_out_sem;
BinarySemaphore cc_in_sem;
BinarySemaphore dm_out_sem;
BinarySemaphore dm_in_sem;

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

/* bnap non volatile storage objects */
MMCDriver MMCD1;
BnapStorage_t Storage;

/*
 ******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************
 */
/**/
static const SerialConfig gsm_ser_cfg = {
    GSM_BAUDRATE,
    //9600,
    AT91C_US_USMODE_HWHSH | AT91C_US_CLKS_CLOCK | AT91C_US_CHRL_8_BITS |
                              AT91C_US_PAR_NONE | AT91C_US_NBSTOP_1_BIT
//      AT91C_US_USMODE_NORMAL | AT91C_US_CLKS_CLOCK | AT91C_US_CHRL_8_BITS |
//                                AT91C_US_PAR_NONE | AT91C_US_NBSTOP_1_BIT
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
  chThdSleep(1);

  chHeapInit(&ThdHeap, (uint8_t *)MEM_ALIGN_NEXT(link_thd_buf), THREAD_HEAP_SIZE);

  gsm_release_reset();
  gps_led_off();
  gsm_led_off();

  chBSemInit(&pps_sem, TRUE);
  chBSemInit(&cc_out_sem, FALSE);
  chBSemInit(&cc_in_sem, FALSE);
  chBSemInit(&dm_out_sem, FALSE);
  chBSemInit(&dm_in_sem, FALSE);

  sdStart(&SDGSM, &gsm_ser_cfg);
  sdStart(&SDDM, &dm_ser_cfg);
  setGlobalFlag(GlobalFlags.dm_port_ready);
  sdStart(&SDMPIOVD, &mpiovd_ser_cfg);
  setGlobalFlag(GlobalFlags.mpiovd_port_ready);

  i2cLocalInit();//0xFFFB8000
  ds1338Init();
  MsgInit();        /* init event sources */
  ParametersInit(); /* need events for proper functionality */
  MavInit();        /* set device IDs previusly red from from EEPROM byt param init*/
  GPSInit();
  LinkInit();
  TimekeeperInit();
  MicrosdInit();
  SanityControlInit();
  ModemSettingsInit();
  ModemInit();
  MavCmdInitLocal();
  UiInit();

  ExtiLocalInit();
  ExtiEnablePps();

  while (TRUE) {
    chThdSleepMilliseconds(1000);
  }
  return 0;
}
