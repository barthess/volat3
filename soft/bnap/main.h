#ifndef MAIN_H_
#define MAIN_H_

typedef unsigned char uint8_t;    /* eclipse indexer workaround */
typedef unsigned short uint16_t;  /* eclipse indexer workaround */
typedef unsigned long uint32_t;  /* eclipse indexer workaround */

/*******************************************************************/
#define GSM_BAUDRATE        115200
#define MPIOVD_BAUDRATE     115200
#define DM_BAUDRATE         115200

/*******************************************************************/
#define THREAD_HEAP_SIZE    4096

/******************************************************************
 * global bool flags */
typedef struct GlobalFlags_t{
  //0
  uint32_t allow_softreset:1;   /* system performs soft reset instead of halting in panic */
  uint32_t messaging_ready:1;   /* message subsystem initialized */
  uint32_t stub1:1;
  uint32_t modem_connected:1;   /* */
  //4
  uint32_t logger_ready:1;      /* logger found end of circular buffer and ready to receive data */
  uint32_t link_cc_ready:1;     /* connection with control center via GSM up */
  uint32_t dm_port_ready:1;     /* connection with dysplay module up */
  uint32_t mpiovd_port_ready:1; /* */
  //8
  uint32_t gps_ready:1;         /* GPS send first packet */
  uint32_t gps_valid:1;         /* coordinates valid */
  uint32_t parameters_got:1;    /* parameters successfully retrieved from EEPROM */
  uint32_t i2c_ready:1;         /* i2c bus initialized */
  //12
  uint32_t storage_connected:1; /* MMC card connected and initialized */
  uint32_t time_proved:1;       /* time is GPS proved */
  uint32_t time_good:1;         /* time was successfully read from RTC and looks good (it may be not precise) */
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

/***************************************************************************
 *  */

/******************************************************************
 * приоритеты для потоков */
#define I2C_THREADS_PRIO          (NORMALPRIO + 5)
#define TIMEKEEPER_THREAD_PRIO    (I2C_THREADS_PRIO - 1)
#define CMD_THREADS_PRIO          (NORMALPRIO + 0)
#define GPS_THREAD_PRIO           (NORMALPRIO - 2)
#define MMC_THREAD_PRIO           (NORMALPRIO - 5)
#define MPIOVD_THREAD_PRIO        (NORMALPRIO - 5)
#define DM_THREAD_PRIO            (NORMALPRIO - 2)
#define CC_THREAD_PRIO            (NORMALPRIO + 0)

/******************************************************************
* статусы возвращаемые разными подсистемами */
#define PARAM_SUCCESS     CH_SUCCESS
#define PARAM_FAILED      CH_FAILED
#define LINK_SUCCESS      CH_SUCCESS
#define LINK_FAILED       CH_FAILED
#define STORAGE_SUCCESS   CH_SUCCESS
#define STORAGE_FAILED    CH_FAILED
#define GSM_SUCCESS       CH_SUCCESS
#define GSM_FAILED        CH_FAILED

/*******************************************************************
 * region for settings file */
#define EEPROM_SETTINGS_START         0
#define EEPROM_SETTINGS_SIZE          1024
#define EEPROM_SETTINGS_END           (EEPROM_SETTINGS_START + EEPROM_SETTINGS_SIZE)

#define EEPROM_MODEM_SETTINGS_START   EEPROM_SETTINGS_END
#define EEPROM_MODEM_SETTINGS_SIZE    1024
#define EEPROM_MODEM_SETTINGS_END     (EEPROM_MODEM_SETTINGS_START + EEPROM_MODEM_SETTINGS_SIZE)

/******************************************************************
* константы для мавлинка */
#define GROUND_STATION_ID   255

/* метка времени для пакетов */
#if (CH_FREQUENCY) >= 1000
#define TIME_BOOT_MS ((chTimeNow()) / ((CH_FREQUENCY) / 1000))
#endif

#endif /* MAIN_H_ */
