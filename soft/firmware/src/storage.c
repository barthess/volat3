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
EepromFileStream EepromTripFile;
EepromFileStream EepromUptimeFile;

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
static uint32_t trip;
static uint32_t uptime;

/* положения */
static uint32_t trip_tip;
static uint32_t uptime_tip;

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

//TODO: lock eeprom file with mutex or sempahore

/**
 * Пробежаться по буферу с целью обнаружения и исправления ошибок
 * после ВНЕЗАПНОЙ пропажи питания.
 * Исправить означает записать вместо испорченного значения предыдущее правильное,
 * чтобы последовательность монотонно возрастала.
 *
 * возвращает положение в кольцевом буфере, содержащее последнее корректное
 * значение
 */
static uint16_t fsck(EepromFileStream *File_p){
  uint32_t v1, v2;
  uint32_t prev = 0;
  uint32_t next = 0;
  uint16_t tip = 0;

  chFileStreamSeek(File_p, 0);

//  while (tip < size){
//    v1 = EepromReadWord(File_p);
//    v2 = EepromReadWord(File_p);
//
//    if (v1 == v2){
//      prev = next;
//      next = v1;
//    }
//    else
//      break; // мы наткнулись битую ячейку, что автоматически указывает на начало буфера
//
//    if (next < prev)
//      break; // нашли точку перегиба
//
//    tip += RECORD_SIZE;
//  }

  return tip;
}

/**
 *
 */
static bool_t get_trip(void){

  trip_tip = fsck(&EepromTripFile);
  chFileStreamSeek(&EepromTripFile, 0);

  trip = EepromReadWord(&EepromTripFile);

  return STORAGE_SUCCESS;
}

/**
 *
 */
static bool_t save_trip(void){
  (void)trip;

  return STORAGE_SUCCESS;
}

/**
 *
 */
static bool_t get_uptime(void){
  (void)uptime;
  return STORAGE_SUCCESS;
}

/*
 ******************************************************************************
 * EXPORTED FUNCTIONS
 ******************************************************************************
 */

void StorageInit(void){
  bool_t status = STORAGE_FAILED;

  /**/
  EepromFileOpen(&EepromTripFile,   &eeprom_trip_cfg);
  EepromFileOpen(&EepromUptimeFile, &eeprom_uptime_cfg);

  trip_tip = 0;
  uptime_tip = 0;

  status = get_trip();
  chDbgCheck(status == STORAGE_SUCCESS, "");

  status = get_uptime();
  chDbgCheck(status == STORAGE_SUCCESS, "");
}

/**
 * shell util to manipulate with trip and uptime values
 */
Thread* storage_cmd(int argc, const char * const * argv, const ShellCmd_t *cmdarray){
  (void)cmdarray;
  (void)argc;
  (void)argv;

  return NULL;
}
