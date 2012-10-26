#ifndef MAIN_H_
#define MAIN_H_

/* mavlink includes */
#include "mavlink.h"
#include "common.h"

/******************************************************************
 * Ревизия платы для программной коррекции ошибок разводки */
#define BOARD_REV   1

/******************************************************************
 * humanreadable names for different serial drivers */
#define MAVSD   SD1
#define SHELLSD SDU1

/* размер кучи под всякие нужды */
#define LINK_THD_HEAP_SIZE    8192

/******************************************************************
 * приоритеты для потоков */
#define I2C_THREADS_PRIO          (NORMALPRIO + 5)
#define TIMEKEEPER_THREAD_PRIO    (I2C_THREADS_PRIO - 1)
#define LINK_THREADS_PRIO         (NORMALPRIO - 5)
#define CMD_THREADS_PRIO          (LINK_THREADS_PRIO - 2)
#define GPS_THREAD_PRIO           (NORMALPRIO - 2)


/******************************************************************
 * глобальные битовые флаги */
#define SPI_SAMPLE_FLAG        (1UL << 0)  /* если установлен в единицу, значит подано питание на дискретные входы */
#define CAN_ACTIVE_FLAG        (1UL << 1)  /* если установлен в единицу, значит запущен CAN и запуск USB невозможен */
#define TLM_ACTIVE_FLAG        (1UL << 2)
#define SHELL_ACTIVE_FLAG      (1UL << 3)
#define STORAGE_READY_FLAG     (1UL << 4)   /* система записи пробега и моточасов готова к работе */

#define setGlobalFlag(flag)   {chSysLock(); GlobalFlags |= (flag); chSysUnlock();}
#define clearGlobalFlag(flag) {chSysLock(); GlobalFlags &= (~(flag)); chSysUnlock();}

/******************************************************************
* статусы возвращаемые разными подсистемами */
#define PARAM_SUCCESS     CH_SUCCESS
#define PARAM_FAILED      CH_FAILED
#define LINK_SUCCESS      CH_SUCCESS
#define LINK_FAILED       CH_FAILED
#define STORAGE_SUCCESS   CH_SUCCESS
#define STORAGE_FAILED    CH_FAILED

/******************************************************************
* константы для мавлинка */
#define GROUND_STATION_ID   255

/* метка времени для пакетов */
#if (CH_FREQUENCY) >= 1000
#define TIME_BOOT_MS ((chTimeNow()) / ((CH_FREQUENCY) / 1000))
#endif

/* build test suit for EEPROM abstraction layer */
#define USE_EEPROM_TEST_SUIT    TRUE

/******************************************************************/
#define EEPROM_PAGE_SIZE          32/* page size in bytes. Consult datasheet. */
#define EEPROM_SIZE               4096/* total amount of memory in bytes */
#define EEPROM_I2CD               I2CD1/* ChibiOS I2C driver used to communicate with EEPROM */
#define EEPROM_I2C_ADDR           0b1010000/* EEPROM address on bus */
#define EEPROM_WRITE_TIME_MS      5/* time to write one page in mS. Consult datasheet! */
#define EEPROM_TX_DEPTH           (EEPROM_PAGE_SIZE + 2)/* temporal transmit buffer depth for eeprom driver */

/* data offsets in eeprom "file" */
#define EEPROM_SETTINGS_START     0
#define EEPROM_SETTINGS_SIZE      1024
#define EEPROM_SETTINGS_END       (EEPROM_SETTINGS_START + EEPROM_SETTINGS_SIZE)
/* save here trip */
#define EEPROM_TRIP_START         EEPROM_SETTINGS_END
#define EEPROM_TRIP_SIZE          1024
#define EEPROM_TRIP_END           (EEPROM_TRIP_START + EEPROM_TRIP_SIZE)
/* save here engine uptime */
#define EEPROM_UPTIME_START       EEPROM_TRIP_END
#define EEPROM_UPTIME_SIZE        1024
#define EEPROM_UPTIME_END         (EEPROM_UPTIME_START + EEPROM_UPTIME_SIZE)

/******************************************************************
* дефайны для модема */
#define BAUDRATE_MAV 115200

/******************************************************************/

/* stop watchdog timer in debugging mode */
/*unlock PR register*/
/*set 1.6384s timeout*/
/*start watchdog*/
#define WATCHDOG_INIT {\
    DBGMCU->CR |= DBGMCU_CR_DBG_IWDG_STOP;\
    IWDG->KR = 0x5555;\
    IWDG->PR = 16;\
    IWDG->KR = 0xCCCC;}

#define WATCHDOG_RELOAD {IWDG->KR = 0xAAAA;}


#endif /* MAIN_H_ */
