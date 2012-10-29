#include <stdio.h>

#include "ch.h"
#include "hal.h"

#include "main.h"
#include "eeprom.h"
#include "eeprom_conf.h"
#include "storage.h"
#include "chprintf.h"

/*
 * Суть такова:
 *
 * Для значений выделяется кольцевой буфер, в который записываются значения.
 * Поскольку значения всегда монотонно возрастают, мы всегда можем найти
 * последнее записанноt значение пробежавшись по буферу и найдя переход
 * с большего в меньшее.
 *
 * Частота сохранения задается в параметрах устройства, по умолчанию - 10 секунд.
 *
 * Защита от внезапного пропадания питания на середине записи зиждется
 * на следующих постулатах:
 * 1) Когда двигателей заведен - масса всегда включена, отрубать
 *    ее на ходу нормальный водятел не станет
 * 2) Когда двигатель заглушен - значение не меняяется, соответственно
 *    сохранять его не нужно.
 * 3) Значение записывается в ПЗУ дважды. Если произошла пропажа питания
 *    между записями - то значения будут отличаться. В таком случае
 *    потребуется откатиться на последнее корректное значение.
 *
 * Консольная утилита для подкручивания километража:
 * 0) блокирует семафором доступ
 * 1) требует пароль
 * 2) полность зануляет буффер и записывает в первую ячейку нужное значение
 */

/*
 ******************************************************************************
 * DEFINES
 ******************************************************************************
 */
#define RECORD_SIZE       (2 * (sizeof(uint32_t)))

#define acquire_trip() chBSemWait(&Trip_sem)
#define release_trip() chBSemSignal(&Trip_sem)

#define acquire_uptime() chBSemWait(&Uptime_sem)
#define release_uptime() chBSemSignal(&Uptime_sem)

/* (re)start virtual timer for uptime counting */
#define uptime_start_vt()                                                     \
  do{                                                                         \
    chVTSetI(&uptime_vt, S2ST(1), &uptime_do_tick_cb, NULL);                  \
  }while(0);

/*
 ******************************************************************************
 * EXTERNS
 ******************************************************************************
 */
extern uint32_t GlobalFlags;

/*
 ******************************************************************************
 * PROTOTYPES
 ******************************************************************************
 */

static VirtualTimer uptime_vt;    /* timer for uptime counting */

static EepromFileStream EepromTripFile;
static EepromFileStream EepromUptimeFile;

static uint32_t Trip;             /* пробег в импульсах спидометра */
static uint32_t Uptime;           /* секунды */

/* указатель на поток сохранялки, чтобы была возможность остановить
 * и перезапустить его */
static Thread *Storage_tp;

/*
 ******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************
 */
static uint32_t Trip_prev;
static uint32_t Uptime_prev;

static BinarySemaphore Trip_sem;
static BinarySemaphore Uptime_sem;

static uint8_t eeprom_trip_buf[EEPROM_TX_DEPTH];
static uint8_t eeprom_uptime_buf[EEPROM_TX_DEPTH];

static const I2CEepromFileConfig eeprom_trip_cfg = {
  &EEPROM_I2CD,
  EEPROM_TRIP_START,
  EEPROM_TRIP_END,
  EEPROM_SIZE,
  EEPROM_PAGE_SIZE,
  EEPROM_I2C_ADDR,
  MS2ST(EEPROM_WRITE_TIME_MS),
  eeprom_trip_buf,
};
static const I2CEepromFileConfig eeprom_uptime_cfg = {
  &EEPROM_I2CD,
  EEPROM_UPTIME_START,
  EEPROM_UPTIME_END,
  EEPROM_SIZE,
  EEPROM_PAGE_SIZE,
  EEPROM_I2C_ADDR,
  MS2ST(EEPROM_WRITE_TIME_MS),
  eeprom_uptime_buf,
};

/*
 ******************************************************************************
 ******************************************************************************
 * LOCAL FUNCTIONS
 ******************************************************************************
 ******************************************************************************
 */

/*
 * Increase uptime every tick if engine is up
 */
static void uptime_do_tick_cb(void *par){
  (void)par;
  chSysLockFromIsr();
  if (GlobalFlags & ENGINE_UP_FLAG)
    Uptime++;
  uptime_start_vt();
  chSysUnlockFromIsr();
}

/**
 * Пробежаться по буферу с целью обнаружения и исправления ошибок
 * после ВНЕЗАПНОЙ пропажи питания.
 * Исправить означает записать вместо испорченного значения предыдущее правильное,
 * чтобы последовательность монотонно возрастала.
 *
 * устанавливает положение в кольцевом буфере, на последнее корректное значение
 */
static void fsck(EepromFileStream *fp){
  uint32_t v1, v2;
  uint32_t prev = 0;
  uint32_t size = chFileStreamGetSize(fp);
  uint32_t next = 0;
  uint16_t tip  = 0;

  chFileStreamSeek(fp, tip);

  while (tip < size){
    v1 = EepromReadWord(fp);
    v2 = EepromReadWord(fp);

    if (v1 == v2){
      prev = next;
      next = v1;
    }
    else
      break; // мы наткнулись битую ячейку, что автоматически указывает на конец буфера

    if (next < prev)
      break; // нашли точку перегиба

    tip += RECORD_SIZE;
  }

  /* just to be safe */
  chDbgCheck((size % 4) == 0, "file size must divide by 4 without remainder");
  chDbgCheck((tip % 4) == 0,  "file pointer must divide by 4 without remainder");

  /* correct tip */
  if (tip == 0)
    chFileStreamSeek(fp, size - RECORD_SIZE);
  else
    chFileStreamSeek(fp, tip  - RECORD_SIZE);
}

/**
 *
 */
static void save(EepromFileStream *fp, uint32_t *curr, uint32_t *prev){
  volatile uint32_t t;
  uint32_t result;

  if (*curr != *prev){ /* only if changed */
    do
      t = *curr;
    while(t != *curr); /* in case of value changed when reading */

    /* wrap if needed */
    if (chFileStreamGetPosition(fp) > (chFileStreamGetSize(fp) - RECORD_SIZE))
      chFileStreamSeek(fp, 0);

    /* write twice */
    result = EepromWriteWord(fp, t);
    chDbgCheck(result == sizeof(t), "write to eeprom failed")
    result = EepromWriteWord(fp, t);
    chDbgCheck(result == sizeof(t), "write to eeprom failed")

    *prev = t;
  }
}

/**
 * Поток для регулярного сохранения пробега и моточасов в EEPROM
 */
static WORKING_AREA(StorageThreadWA, 384);
static msg_t StorageThread(void *arg) {
  chRegSetThreadName("Storage");
  (void)arg;

  setGlobalFlag(STORAGE_READY_FLAG);

  while (!chThdShouldTerminate()) {
    chThdSleepMilliseconds(1000);
    acquire_trip();
    save(&EepromTripFile,   &Trip,   &Trip_prev);
    release_trip();

    acquire_uptime();
    save(&EepromUptimeFile, &Uptime, &Uptime_prev);
    release_uptime();

    /* for selftesting */
    Trip++;
    Uptime++;
  }

  chThdExit(0);
  return 0;
}

/**
 *
 */
static void storage_cli_help(void){
  cli_println("Usage:");
  cli_println("'storage trip N PASSWD' to change vehicle trip,");
  cli_println("'storage uptime S PASSWD' to change engine uptime");
  cli_println("where:");
  cli_println("    N is amount of pulses from speedometer");
  cli_println("    S is amount of uptime seconds");
  cli_println("    PASSWD is super secret password");
  chThdSleepMilliseconds(100);
}

/**
 *
 */
static void storage_cli_print(SerialDriver *sdp){
  chprintf((BaseSequentialStream *)sdp, "trip: %u", Trip);
  cli_println("");
  chprintf((BaseSequentialStream *)sdp, "uptime: %u", Uptime);
  cli_println("");
}

/**
 *  val[in]   value to sotore after erasing.
 */
static bool_t storage_cli_set(EepromFileStream *fp, uint32_t val){
  uint32_t size = chFileStreamGetSize(fp);
  uint32_t result;

  while (size){
    EepromWriteByte(&EepromTripFile, 0);
    size--;
    /* progress bar */
    uint32_t d = 0b11;
    if ((size & d) == d)
      cli_print(".");
  }
  cli_println("");
  cli_println("Erase done");

  /* sotore new value */
  cli_println("Writing new value");
  chFileStreamSeek(fp, 0);
  result = EepromWriteWord(fp, val);
  chDbgCheck(result == sizeof(val), "write to eeprom failed")
  result = EepromWriteWord(fp, val);
  chDbgCheck(result == sizeof(val), "write to eeprom failed")

  return CH_SUCCESS;
}

/**
 *
 */
static void __storage_start(void){
  /* trip */
  acquire_trip();
  EepromFileOpen(&EepromTripFile, &eeprom_trip_cfg);
  fsck(&EepromTripFile);
  /* read twice to have aligned pointer in file */
  Trip = EepromReadWord(&EepromTripFile);
  Trip = EepromReadWord(&EepromTripFile);
  Trip_prev = Trip;
  release_trip();

  /* uptime */
  acquire_uptime();
  EepromFileOpen(&EepromUptimeFile, &eeprom_uptime_cfg);
  fsck(&EepromUptimeFile);
  /* read twice to have aligned pointer in file */
  Uptime = EepromReadWord(&EepromUptimeFile);
  Uptime = EepromReadWord(&EepromUptimeFile);
  Uptime_prev = Uptime;
  release_uptime();

  /* fork thread */
  Storage_tp = chThdCreateStatic(StorageThreadWA,
                                sizeof(StorageThreadWA),
                                NORMALPRIO,
                                StorageThread,
                                NULL);
}

/*
 ******************************************************************************
 * EXPORTED FUNCTIONS
 ******************************************************************************
 */

void StorageInit(void){

  chSysLock();
  uptime_start_vt();
  chSysUnlock();

  /**/
  chBSemInit(&Trip_sem, FALSE);
  chBSemInit(&Uptime_sem, FALSE);

  __storage_start();
}

/**
 * Stop writing thread for maintaince.
 */
static void __storage_stop(void){
  if ((Storage_tp != NULL) && (Storage_tp->p_state != THD_STATE_FINAL)){
    chThdTerminate(Storage_tp);
    chThdWait(Storage_tp);
  }
}

/**
 *
 */
void update_trip(uint32_t delta){
  Trip += delta;
}

uint32_t get_uptime(void){
  return Uptime;
}

uint32_t get_trip(void){
  return Trip;
}

/**
 *
 */
static void storage_cli_dump(EepromFileStream *fp, SerialDriver *sdp){
  uint32_t i = 0;
  uint32_t v1, v2;

  chFileStreamSeek(fp, 0);
  while (i < chFileStreamGetSize(fp)){
    v1 = EepromReadWord(fp);
    v2 = EepromReadWord(fp);
    chprintf((BaseSequentialStream *)sdp, "%u: %u %u", i, v1, v2);
    if (v1 != v2)
      cli_print(" <---");
    cli_println("");
    i += RECORD_SIZE;
    chThdSleepMilliseconds(20);
  }
}

/**
 * shell util to manipulate with trip and uptime values
 */
Thread* storage_clicmd(int argc, const char * const * argv, SerialDriver *sdp){
  (void)sdp;
  (void)argc;
  (void)argv;
  int sscanf_status;
  uint32_t N;

  /* no arguments */
  if (argc == 0){
    storage_cli_print(sdp);
  }

  else if (argc == 1){
    if (strcmp(argv[0], "dump") == 0){
      __storage_stop();

      cli_println("Dump trip ----------------------------------");
      acquire_trip();
      storage_cli_dump(&EepromTripFile, sdp);
      release_trip();

      cli_println("Dump uptime --------------------------------");
      acquire_uptime();
      storage_cli_dump(&EepromUptimeFile, sdp);
      release_uptime();

      chFileStreamClose(&EepromTripFile);
      chFileStreamClose(&EepromUptimeFile);
      __storage_start();
      return NULL;
    }
  }

  /* three arguments */
  else if (argc == 3){

    /* check password */
    if (strcmp(argv[2], "Kamerton") != 0){
      cli_println("ERROR: wrong password");
      goto ERROR;
    }

    /* read value */
    sscanf_status = sscanf(argv[1], "%u", (unsigned int *)&N);
    if (sscanf_status != 1){
      cli_println("ERROR. Entered value inconsistent");
      goto ERROR;
    }

    /* apply value */
    if (strcmp(argv[0], "trip") == 0){
      cli_println("Erasing trip");
      acquire_trip();
      storage_cli_set(&EepromTripFile, N);
      release_trip();
      cli_println("Done");
    }
    else if(strcmp(argv[0], "uptime") == 0){
      cli_println("Erase uptime");
      acquire_uptime();
      storage_cli_set(&EepromUptimeFile, N);
      release_uptime();
      cli_println("Done");
    }
    else
      goto ERROR;
  }

  /* stub */
ERROR:
  storage_cli_help();
  return NULL;
}
