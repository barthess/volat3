#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "ch.h"
#include "hal.h"

#include "main.h"
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

/*
 *******************************************************************************
 * EXPORTED FUNCTIONS
 *******************************************************************************
 */

Thread* clear_clicmd(int argc, const char * const * argv, SerialDriver *sdp){
  (void)sdp;
  (void)argc;
  (void)argv;
  cli_print("\033[2J");    // ESC seq for clear entire screen
  cli_print("\033[H");     // ESC seq for move cursor at left-top corner
  return NULL;
}


static WORKING_AREA(LoopCmdThreadWA, 128);
static msg_t LoopCmdThread(void *arg){
  chRegSetThreadName("LoopCmd");
  (void)arg;
  int i = 20;

  while (i > 0){
    cli_print("This is loop function test. Press ^C to stop it.\n\r");
    chThdSleepMilliseconds(1000);
    i--;
    if (chThdShouldTerminate())
      chThdExit(0);
  }
  return 0;
}

Thread* loop_cmd(int argc, const char * const * argv, SerialDriver *sdp){
  (void)sdp;
  (void)argc;
  (void)argv;

  Thread *tp = chThdCreateFromHeap(&ThdHeap,
                                  sizeof(LoopCmdThreadWA),
                                  CMD_THREADS_PRIO - 1,
                                  LoopCmdThread,
                                  NULL);
  return tp;
}

Thread* logout_cmd(int argc, const char * const * argv, SerialDriver *sdp){
  (void)sdp;
  (void)argv;
  (void)argc;
  cli_print("Logout command stub...\r\n");
  chThdSleepMilliseconds(100);
  NVIC_SystemReset();
  return NULL;
}

Thread* selftest_clicmd(int argc, const char * const * argv, SerialDriver *sdp){
  (void)sdp;
  (void)argv;
  (void)argc;

  cli_print("GPS - OK\r\nModem - OK\r\nEEPROM - OK\r\nStorage - OK\r\nServos - OK\r\n");
  return NULL;
}

Thread* sensor_cmd(int argc, const char * const * argv, SerialDriver *sdp){
  (void)sdp;
  (void)argv;
  (void)argc;
  cli_print("temperature is ... \n\r");
  return NULL;
}


/**
 * helper function
 */
void long_cli_print(const char * str, int n, int nres){
  cli_print(str);
  if (nres > n)
    cli_print("\n\r");
}

Thread* uname_clicmd(int argc, const char * const * argv, SerialDriver *sdp){
  (void)sdp;
  (void)argc;
  (void)argv;

  int n = 64;
  int nres = 0;
  char str[n];

  nres = snprintf(str, n, "Kernel:       %s\r\n", CH_KERNEL_VERSION);
  long_cli_print(str, n, nres);

#ifdef CH_COMPILER_NAME
  nres = snprintf(str, n, "Compiler:     %s\r\n", CH_COMPILER_NAME);
  long_cli_print(str, n, nres);
#endif

  nres = snprintf(str, n, "Architecture: %s\r\n", CH_ARCHITECTURE_NAME);
  long_cli_print(str, n, nres);

#ifdef CH_CORE_VARIANT_NAME
  nres = snprintf(str, n, "Core Variant: %s\r\n", CH_CORE_VARIANT_NAME);
  long_cli_print(str, n, nres);
#endif

#ifdef CH_PORT_INFO
  nres = snprintf(str, n, "Port Info:    %s\r\n", CH_PORT_INFO);
  long_cli_print(str, n, nres);
#endif

#ifdef PLATFORM_NAME
  nres = snprintf(str, n, "Platform:     %s\r\n", PLATFORM_NAME);
  long_cli_print(str, n, nres);
#endif

#ifdef BOARD_NAME
  nres = snprintf(str, n, "Board:        %s\r\n", BOARD_NAME);
  long_cli_print(str, n, nres);
#endif

#ifdef __DATE__
#ifdef __TIME__
  nres = snprintf(str, n, "Build time:   %s%s%s\r\n", __DATE__, " - ", __TIME__);
  long_cli_print(str, n, nres);
#endif
#endif

  return NULL;
}

Thread* ps_clicmd(int argc, const char * const * argv, SerialDriver *sdp){
  (void)sdp;
  (void)argc;
  (void)argv;
  Thread *curr = NULL;

#if !CH_USE_REGISTRY
  cli_print("In order to use this function you must set CH_USE_REGISTRY to TRUE\n\r");
  return NULL;

#else
  curr = chRegFirstThread();

  cli_print("name\t\tstate\tprio\ttime\n\r");
  cli_print("------------------------------------------\n\r");
  while (curr->p_refs > 0){
    cli_print(curr->p_name);
    cli_print("\t");
    //cli_print(curr->p_state);
    cli_print("\n\r");
    curr = chRegNextThread(curr);
  }
  return NULL;
#endif
}





