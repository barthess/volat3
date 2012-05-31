#ifndef MAIN_H_
#define MAIN_H_

/******************************************************************
 * humanreadable names for different serial drivers */
#define MAVSD   SD2
#define SHELLSD SDU1

/* размер кучи под всякие нужды */
#define LINK_THD_HEAP_SIZE    1024

/******************************************************************
 * приоритеты для потоков */
#define I2C_THREADS_PRIO          (NORMALPRIO + 5)
#define TIMEKEEPER_THREAD_PRIO    (I2C_THREADS_PRIO - 1)
#define LINK_THREADS_PRIO         (NORMALPRIO - 5)
#define CMD_THREADS_PRIO          (LINK_THREADS_PRIO - 2)
#define GPS_THREAD_PRIO           (NORMALPRIO - 2)


/******************************************************************
 * глобальные битовые флаги */
#define GYRO_CAL_FLAG        (1UL << 0)  /* если установлен в единицу, значит идет выставка гироскопов */
#define ACCEL_CAL_FLAG       (1UL << 1)  /* если установлен в единицу, значит идет выставка акселерометров */
#define MAG_CAL_FLAG         (1UL << 2)  /* если установлен в единицу, значит идет выставка магнитометра */
#define EEPROM_FAILED_FLAG   (1UL << 3)  /* единица означает сбой в EEPROM */
#define POSTAGE_FAILED_FLAG  (1UL << 4)  /* Сбой в системе отправки сообщений */
#define I2C_RESTARTED_FLAG   (1UL << 5)  /* I2C шина была перезапущена из-за проблем */

#define setGlobalFlag(flag)   {chSysLock(); GlobalFlags |= (flag); chSysUnlock();}
#define clearGlobalFlag(flag) {chSysLock(); GlobalFlags &= (~(flag)); chSysUnlock();}


/******************************************************************
* статусы возвращаемые разными подсистемами */
#define PARAM_SUCCESS  CH_SUCCESS
#define PARAM_FAILED   CH_FAILED
#define LINK_SUCCESS   CH_SUCCESS
#define LINK_FAILED    CH_FAILED


/******************************************************************
* константы для мавлинка */
#define GROUND_STATION_ID   255

/* метка времени для пакетов */
#if (CH_FREQUENCY) >= 1000
#define TIME_BOOT_MS ((chTimeNow()) / ((CH_FREQUENCY) / 1000))
#endif


/******************************************************************
* data offsets in eeprom "file" */
#define EEPROM_SETTINGS_START    0
#define EEPROM_SETTINGS_SIZE     4096
#define EEPROM_SETTINGS_FINISH   (EEPROM_SETTINGS_START + EEPROM_SETTINGS_SIZE)


/******************************************************************
* дефайны для модема */
#define BAUDRATE_MAV 115200

/******************************************************************
* включить стрессовое тестирование */
#define ENABLE_IRQ_STORM    FALSE

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
