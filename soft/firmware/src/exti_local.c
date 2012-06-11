#include <stdlib.h>

#include "ch.h"
#include "hal.h"
#include "sensors.h"
#include "exti_local.h"
#include "rtc.h"

/*
 ******************************************************************************
 * DEFINES
 ******************************************************************************
 */

/*
 ******************************************************************************
 * EXTERNS
 ******************************************************************************
 */

/*
 ******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************
 */

/*
 *******************************************************************************
 *******************************************************************************
 * LOCAL FUNCTIONS
 *******************************************************************************
 *******************************************************************************
 */

static const EXTConfig extcfg = {
  {
    {EXT_CH_MODE_DISABLED, NULL},// секундная метка с GPS
    {EXT_CH_MODE_DISABLED, NULL},// оптрон тахометра
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL}, // микросдшка
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},//5
    {EXT_CH_MODE_DISABLED, NULL}, // датчик давления
    {EXT_CH_MODE_DISABLED, NULL}, // magnetometer
    {EXT_CH_MODE_DISABLED, NULL}, // первое прерывание с акселерометра
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL}, // хероскоп
    {EXT_CH_MODE_DISABLED, NULL},//11
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},//15
    {EXT_CH_MODE_DISABLED, NULL},//PVD
    {EXT_CH_MODE_DISABLED, NULL},// RTC alarms
    {EXT_CH_MODE_DISABLED, NULL},
  }
};



/*
EXT_MODE_GPIOE,// GPS_PPS
0,
0,
EXT_MODE_GPIOE,// microSD detect
0,
0,//5
EXT_MODE_GPIOE,// давленометр
EXT_MODE_GPIOE,// магнитометр
EXT_MODE_GPIOE,// accelerometer int1
0,
EXT_MODE_GPIOE,// gyro
0,
EXT_MODE_GPIOE,//оптрон
0,
0,//14
EXT_MODE_GPIOE)// accelerometer int2
*/


/*
 *******************************************************************************
 * EXPORTED FUNCTIONS
 *******************************************************************************
 */

void ExtiInitLocal(){
  extStart(&EXTD1, &extcfg);
}




