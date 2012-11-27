#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "ch.h"
#include "hal.h"
#include "mavlink.h"

#include "main.h"
#include "message.h"
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

/*
 ******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************
 */
static Thread *loop_clicmd_tp;

/*
 *******************************************************************************
 * EXPORTED FUNCTIONS
 *******************************************************************************
 */

/**
 *
 */
Thread* clear_clicmd(int argc, const char * const * argv, SerialDriver *sdp){
  (void)sdp;
  (void)argc;
  (void)argv;
  cli_print("\033[2J");    // ESC seq for clear entire screen
  cli_print("\033[H");     // ESC seq for move cursor at left-top corner
  return NULL;
}

/**
 *
 */
static WORKING_AREA(LoopCmdThreadWA, 1024);
static msg_t LoopCmdThread(void *arg){
  chRegSetThreadName("LoopCmd");
  (void)arg;

  cli_print("This is loop function test. Press ^C to stop it.\n\r");
  while (!chThdShouldTerminate()){
    int n = 16;
    char str[n];

    snprintf(str, n, "%i\r\n", 6666);
    cli_print(str);
    chThdSleepMilliseconds(25);
  }

  chThdExit(0);
  return 0;
}

/**
 *
 */
Thread* loop_clicmd(int argc, const char * const * argv, SerialDriver *sdp){
  (void)sdp;
  (void)argc;
  (void)argv;

  loop_clicmd_tp = chThdCreateFromHeap(&ThdHeap,
                                  sizeof(LoopCmdThreadWA),
                                  CMD_THREADS_PRIO,
                                  LoopCmdThread,
                                  NULL);

  if (loop_clicmd_tp == NULL)
    chDbgPanic("can not allocate memory");

  return loop_clicmd_tp;
}

/**
 *
 */
Thread* selftest_clicmd(int argc, const char * const * argv, SerialDriver *sdp){
  (void)sdp;
  (void)argv;
  (void)argc;

  cli_print("GPS - OK\r\nModem - OK\r\nEEPROM - OK\r\nStorage - OK\r\nServos - OK\r\n");
  return NULL;
}

/**
 *
 */
Thread* uname_clicmd(int argc, const char * const * argv, SerialDriver *sdp){
  (void)sdp;
  (void)argc;
  (void)argv;

  int n = 80;
  int nres = 0;
  char str[n];

  nres = snprintf(str, n, "Kernel:       %s\r\n", CH_KERNEL_VERSION);
  cli_print_long(str, n, nres);

#ifdef CH_COMPILER_NAME
  nres = snprintf(str, n, "Compiler:     %s\r\n", CH_COMPILER_NAME);
  cli_print_long(str, n, nres);
#endif

  nres = snprintf(str, n, "Architecture: %s\r\n", CH_ARCHITECTURE_NAME);
  cli_print_long(str, n, nres);

#ifdef CH_CORE_VARIANT_NAME
  nres = snprintf(str, n, "Core Variant: %s\r\n", CH_CORE_VARIANT_NAME);
  cli_print_long(str, n, nres);
#endif

#ifdef CH_PORT_INFO
  nres = snprintf(str, n, "Port Info:    %s\r\n", CH_PORT_INFO);
  cli_print_long(str, n, nres);
#endif

#ifdef PLATFORM_NAME
  nres = snprintf(str, n, "Platform:     %s\r\n", PLATFORM_NAME);
  cli_print_long(str, n, nres);
#endif

#ifdef BOARD_NAME
  nres = snprintf(str, n, "Board:        %s\r\n", BOARD_NAME);
  cli_print_long(str, n, nres);
#endif

#ifdef __DATE__
#ifdef __TIME__
  nres = snprintf(str, n, "Build time:   %s%s%s\r\n", __DATE__, " - ", __TIME__);
  cli_print_long(str, n, nres);
#endif
#endif

  return NULL;
}







