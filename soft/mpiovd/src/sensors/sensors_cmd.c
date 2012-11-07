#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "ch.h"
#include "hal.h"

#include "main.h"
#include "message.h"
#include "utils.h"
#include "sensors.h"
#include "cli.h"
#include "cli_cmd.h"


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
extern MemoryHeap ThdHeap;
extern const RawData raw_data;
extern const mavlink_mpiovd_sensors_raw_t     mpiovd_sensors_raw_struct;

/*
 ******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************
 */
static Thread *sensors_cmd_tp;

/*
 *******************************************************************************
 * EXPORTED FUNCTIONS
 *******************************************************************************
 */

/**
 *
 */
static WORKING_AREA(SensorsCmdThreadWA, 768);
static msg_t SensorsCmdThread(void *arg){
  chRegSetThreadName("SensorsCmd");
  (void)arg;
  uint32_t i;
  uint8_t data[5];

  const int n = 70;
  int nres = 0;
  char str[n];

  while(!chThdShouldTerminate()){
    i = mpiovd_sensors_raw_struct.analog01;
    putinrange(i, 0, 255);
    data[0] = i;
    i = mpiovd_sensors_raw_struct.analog02;
    putinrange(i, 0, 255);
    data[1] = i;
    i = mpiovd_sensors_raw_struct.analog03;
    putinrange(i, 0, 255);
    data[2] = i;
    i = mpiovd_sensors_raw_struct.analog04;
    putinrange(i, 0, 255);
    data[3] = i;
    /* (>> 1) to exclude "dead" bit from SPI*/
    data[4] = (raw_data.discrete >> 1) & 0xFF;

    nres = snprintf(str, n, "A0=%u, A1=%u, A2=%u, A3=%u, D=%u",
        data[0], data[1], data[2], data[3], data[4]);
    cli_print_long(str, n, nres);
    cli_println("");

//    cli_println("asdfasdgf");
    chThdSleepMilliseconds(200);
  }

  chThdExit(0);
  return 0;
}

/**
 *
 */
Thread* sensors_clicmd(int argc, const char * const * argv, SerialDriver *sdp){
  (void)sdp;
  (void)argc;
  (void)argv;

  sensors_cmd_tp = chThdCreateFromHeap(&ThdHeap,
                                  sizeof(SensorsCmdThreadWA),
                                  CMD_THREADS_PRIO - 1,
                                  SensorsCmdThread,
                                  NULL);
  if (sensors_cmd_tp == NULL)
    chDbgPanic("Can not allocate memory");

  return sensors_cmd_tp;
}
