#include "ch.h"
#include "hal.h"

#include "main.h"
#include "eeprom.h"
#include "storage.h"

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
 * Консольная утилита для отматывания километража:
 * 1) требует пароль
 * 2) полность зануляет буффер и записывает в первую ячейку нужное значение
 */

/*
 ******************************************************************************
 * DEFINES
 ******************************************************************************
 */
#define RECORD_SIZE       (2 * (sizeof(uint32_t)))

/*
 ******************************************************************************
 * EXTERNS
 ******************************************************************************
 */
extern uint32_t GlobalFlags;

EepromFileStream EepromTripFile;
EepromFileStream EepromUptimeFile;

uint32_t Trip;
uint32_t Uptime;

/*
 ******************************************************************************
 * PROTOTYPES
 ******************************************************************************
 */

/*
 ******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************
 */
static uint32_t Trip_prev;
static uint32_t Uptime_prev;

static uint8_t eeprom_trip_buf[EEPROM_TX_DEPTH];
static const I2CEepromFileConfig eeprom_trip_cfg = {
  &EEPROM_I2CD,
  EEPROM_TRIP_START,
  EEPROM_TRIP_END,
  EEPROM_SIZE,
  EEPROM_PAGE_SIZE,
  EEPROM_I2C_ADDR,
  MS2ST(EEPROM_WRITE_TIME_MS),
  TRUE,
  eeprom_trip_buf,
};

static uint8_t eeprom_uptime_buf[EEPROM_TX_DEPTH];
static const I2CEepromFileConfig eeprom_uptime_cfg = {
  &EEPROM_I2CD,
  EEPROM_UPTIME_START,
  EEPROM_UPTIME_END,
  EEPROM_SIZE,
  EEPROM_PAGE_SIZE,
  EEPROM_I2C_ADDR,
  MS2ST(EEPROM_WRITE_TIME_MS),
  TRUE,
  eeprom_uptime_buf,
};

/*
 ******************************************************************************
 ******************************************************************************
 * LOCAL FUNCTIONS
 ******************************************************************************
 ******************************************************************************
 */

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

  chFileStreamSeek(fp, 0);

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
  chDbgCheck((tip % 4) == 0,"file pointer must be divided by 4 without remainder");

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
    while(t != *curr); /* case of changing when reading */

    /* write twice */
    result = EepromWriteWord(fp, t);
    if (result < sizeof(t))
      chDbgPanic("failed");

    result = EepromWriteWord(fp, t);
    if (result < sizeof(t))
      chDbgPanic("failed");

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

  while (TRUE) {
    chThdSleepMilliseconds(1000);
    save(&EepromTripFile,   &Trip,   &Trip_prev);
    save(&EepromUptimeFile, &Uptime, &Uptime_prev);

    /* selftesting */
    Trip++;
    Uptime++;
  }
  return 0;
}

/*
 ******************************************************************************
 * EXPORTED FUNCTIONS
 ******************************************************************************
 */

void StorageInit(void){
  /* trip */
  EepromFileOpen(&EepromTripFile, &eeprom_trip_cfg);
  fsck(&EepromTripFile);
  Trip = EepromReadWord(&EepromTripFile);
  Trip_prev = Trip;

  /* uptime */
  EepromFileOpen(&EepromUptimeFile, &eeprom_uptime_cfg);
  fsck(&EepromUptimeFile);
  Uptime = EepromReadWord(&EepromUptimeFile);
  Uptime_prev = Uptime;

  /* fork thread */
  chThdCreateStatic(StorageThreadWA,
          sizeof(StorageThreadWA),
          NORMALPRIO,
          StorageThread,
          NULL);
}

/**
 *
 */
static void storage_cli_help(void){
  cli_print("Help stub");
}

/**
 * shell util to manipulate with trip and uptime values
 */
Thread* storage_clicmd(int argc, const char * const * argv, SerialDriver *sdp){
  (void)sdp;
  (void)argc;
  (void)argv;

  /* no arguments */
  if (argc == 0)
    storage_cli_help();



  /* one argument */
  else if (argc == 1){
    if (strcmp(*argv, "help") == 0)
      storage_cli_help();

    else if (strcmp(*argv, "erase") == 0){
      cli_println("Erase trip");
      uint32_t i = 0;
      while (i < 256){
        EepromWriteWord(&EepromTripFile, 0);
        i++;
        cli_print(".");
      }
      cli_println("Done");
      cli_println("Erase uptime");
      i = 0;
      while (i < 256){
        EepromWriteWord(&EepromUptimeFile, 0);
        i++;
        cli_print(".");
      }
      cli_println("");
    }
    else{
      storage_cli_help();
    }
  }



  /* two arguments */
  else if (argc == 2){
    storage_cli_help();
  }

  else{
    storage_cli_help();
  }

  /* stub */
  return NULL;
}
