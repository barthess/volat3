#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "ch.h"
#include "hal.h"
#include "chprintf.h"

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
static Thread *selftest_clicmd_tp;

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
extern EventSource event_mavlink_heartbeat_mpiovd;
extern EventSource event_mavlink_heartbeat_cc;
extern EventSource event_mavlink_heartbeat_bnap;
static WORKING_AREA(SelftestCmdThreadWA, 256);
static msg_t SelftestCmdThread(void *arg){
  chRegSetThreadName("SelftestCmd");

  eventmask_t evt = 0;
  struct EventListener el_heartbeat_mpiovd;
  struct EventListener el_heartbeat_cc;
  struct EventListener el_heartbeat_bnap;
  chEvtRegisterMask(&event_mavlink_heartbeat_mpiovd, &el_heartbeat_mpiovd, EVMSK_MAVLINK_HEARTBEAT_MPIOVD);
  chEvtRegisterMask(&event_mavlink_heartbeat_cc, &el_heartbeat_cc, EVMSK_MAVLINK_HEARTBEAT_CC);
  chEvtRegisterMask(&event_mavlink_heartbeat_bnap, &el_heartbeat_bnap, EVMSK_MAVLINK_HEARTBEAT_BNAP);


  cli_print("Press ^C to stop it.\n\r");

  while (!chThdShouldTerminate()){
    evt = chEvtWaitOneTimeout(EVMSK_MAVLINK_HEARTBEAT_MPIOVD |
                              EVMSK_MAVLINK_HEARTBEAT_CC |
                              EVMSK_MAVLINK_HEARTBEAT_BNAP, MS2ST(50));
    switch(evt){
    case EVMSK_MAVLINK_HEARTBEAT_MPIOVD:
      chprintf(arg, "%U - MPIOVD", chTimeNow());
      break;

    case EVMSK_MAVLINK_HEARTBEAT_CC:
      chprintf(arg, "%U - CC", chTimeNow());
      break;

    case EVMSK_MAVLINK_HEARTBEAT_BNAP:
      chprintf(arg, "%U - BNAP", chTimeNow());
      break;

    default:
      break;
    }
  }

  chEvtUnregister(&event_mavlink_heartbeat_mpiovd, &el_heartbeat_mpiovd);
  chEvtUnregister(&event_mavlink_heartbeat_cc, &el_heartbeat_cc);
  chEvtUnregister(&event_mavlink_heartbeat_bnap, &el_heartbeat_bnap);
  chThdExit(0);
  return 0;
}

/**
 *
 */
Thread* selftest_clicmd(int argc, const char * const * argv, SerialDriver *sdp){
  (void)argv;
  (void)argc;

  selftest_clicmd_tp = chThdCreateFromHeap(&ThdHeap,
                                  sizeof(SelftestCmdThreadWA),
                                  CMD_THREADS_PRIO,
                                  SelftestCmdThread,
                                  sdp);

  if (selftest_clicmd_tp == NULL)
    chDbgPanic("can not allocate memory");

  return selftest_clicmd_tp;
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