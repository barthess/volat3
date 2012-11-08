#ifndef MAIN_H_
#define MAIN_H_

/* mavlink includes */
//#include "mavlink.h"
//#include "common.h"

/******************************************************************
 * Ревизия платы для программной коррекции ошибок разводки */
#define BOARD_REV   1

/******************************************************************
 * global bool flags */
typedef struct GlobalFlags_t{
  //0
  uint32_t allow_softreset:1; /* system performs soft reset instead of halting in panic */
  uint32_t gyro_cal:1;
  uint32_t accel_cal:1;
  uint32_t mag_cal:1;
  //4
  uint32_t eeprom_failed:1;
  uint32_t tlm_link_ready:1;
  uint32_t modem_ready:1;
  uint32_t logger_ready:1;
  //8
  uint32_t sighalt:1;
  uint32_t mag_data_ready:1;
  uint32_t mission_takeoff:1;
  uint32_t mission_loiter:1;
  //12
  uint32_t mission_abort:1;
  uint32_t parameters_got:1;  /* parameters successfully retrieved from EEPROM */
  uint32_t i2c_ready:1;       /* i2c bus initialized */
  uint32_t stub3:1;
  //16
  uint32_t stub4:1;
  uint32_t stub5:1;
  uint32_t stub6:1;
  uint32_t stub7:1;
  //20
  uint32_t stub8:1;
  uint32_t stub9:1;
  uint32_t stuba:1;
  uint32_t stubb:1;
  //24
  uint32_t stubc:1;
  uint32_t stubd:1;
  uint32_t stube:1;
  uint32_t stubf:1;
  //28
  uint32_t stubg:1;
  uint32_t stubh:1;
  uint32_t stubi:1;
  uint32_t stubj:1;
}GlobalFlags_t;

#define setGlobalFlagI(flag)    do{chDbgCheckClassI(); flag = 1;}while(0)
#define clearGlobalFlagI(flag)  do{chDbgCheckClassI(); flag = 0;}while(0)

#define setGlobalFlag(flag)     do{chSysLock(); setGlobalFlagI(flag);   chSysUnlock();}while(0)
#define clearGlobalFlag(flag)   do{chSysLock(); clearGlobalFlagI(flag); chSysUnlock();}while(0)

/******************************************************************
 * приоритеты для потоков */
#define I2C_THREADS_PRIO          (NORMALPRIO + 5)
#define TIMEKEEPER_THREAD_PRIO    (I2C_THREADS_PRIO - 1)
#define LINK_THREADS_PRIO         (NORMALPRIO - 5)
#define CMD_THREADS_PRIO          (LINK_THREADS_PRIO - 2)
#define GPS_THREAD_PRIO           (NORMALPRIO - 2)

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

#endif /* MAIN_H_ */