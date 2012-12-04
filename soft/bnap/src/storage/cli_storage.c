#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "ch.h"
#include "hal.h"

#include "main.h"
#include "storage.h"
#include "cli.h"

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
extern MMCDriver MMCD1;
extern BnapStorage_t Storage;
extern MemoryHeap ThdHeap;

/*
 ******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************
 */

/*
 ******************************************************************************
 * LOCAL FUNCTIONS
 ******************************************************************************
 */

/**
 *
 */
static void cli_storage_print_help(void){
  cli_println("Available subcommands:");
  cli_println("  'void' fast erase (not safe)");
  cli_println("  'wipe' safe full erase (slow)");
  cli_println("  'stat' print some statistics about storage");
  chThdSleepMilliseconds(100);
}

/**
 * Wiping thread
 */
static WORKING_AREA(WipeCmdThreadWA, 1024);
static msg_t WipeCmdThread(void *arg){
  chRegSetThreadName("Wipe");

  bnapStorageAcquire(&Storage );
  bnapStorageWipe(&Storage ,arg);
  bnapStorageRelease(&Storage );

  chThdExit(0);
  return 0;
}

/**
 *
 */
static Thread *storage_cli_wipe(BnapStorage_t *bsp, SerialDriver *sdp){
  (void)bsp;
  Thread *tp = chThdCreateFromHeap(&ThdHeap,
                                    sizeof(WipeCmdThreadWA),
                                    CMD_THREADS_PRIO,
                                    WipeCmdThread,
                                    sdp);
  if (tp == NULL)
    chDbgPanic("can not allocate memory");
  return tp;
}

/**
 *
 */
static void storage_cli_stat(BnapStorage_t *bsp, SerialDriver *sdp){
  (void)sdp;
  (void)bsp;
  bnapStorageAcquire(bsp);
  cli_println("unimplemented yet");
  bnapStorageRelease(bsp);
}

/**
 *
 */
static void storage_cli_void(BnapStorage_t *bsp, SerialDriver *sdp){
  (void)sdp;
  (void)bsp;
  bnapStorageAcquire(bsp);
  bnapStorageVoid(bsp);
  bnapStorageRelease(bsp);
}

/*
 ******************************************************************************
 * EXPORTED FUNCTIONS
 ******************************************************************************
 */

/**
 * Working with parameters from CLI.
 */
Thread* storage_clicmd(int argc, const char * const * argv, SerialDriver *sdp){

  /* no arguments */
  if (argc == 0)
    cli_storage_print_help();

  /* one argument */
  else if (argc == 1){
    if (!mmcIsCardInserted(&MMCD1)){
      cli_println("ERROR: card not inserted");
      chThdSleepMilliseconds(100);
      return NULL;
    }
    if (0 == strcmp("void", argv[0])){
      storage_cli_void(&Storage, sdp);
      return NULL;
    }
    else if (0 == strcmp("wipe", argv[0]))
      return storage_cli_wipe(&Storage, sdp);
    else if (0 == strcmp("stat", argv[0])){
      storage_cli_stat(&Storage, sdp);
      return NULL;
    }
  }

  cli_storage_print_help();
  return NULL;
}
