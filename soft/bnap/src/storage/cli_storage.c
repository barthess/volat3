#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "ch.h"
#include "hal.h"

#include "main.h"
#include "storage.h"
#include "cli.h"
#include "microsd.h"

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

extern Thread *microsd_writer_tp;
extern Thread *microsd_reader_cc_tp;
extern Thread *microsd_reader_dm_tp;

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
  cli_println("  'wipe' full erase");
  cli_println("  'stat' print some statistics about storage");
  chThdSleepMilliseconds(100);
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
static void storage_cli_wipe(BnapStorage_t *bsp, SerialDriver *sdp){

  cli_terminate_thread(microsd_writer_tp,     sdp);
  cli_terminate_thread(microsd_reader_cc_tp,  sdp);
  cli_terminate_thread(microsd_reader_dm_tp,  sdp);

  cli_print("Acquiring mutual access to storage... ");
  bnapStorageAcquire(bsp);
  cli_println("done.");

  bnapStorageWipe(bsp);

  cli_print("Start back storage threads... ");
  bnapStorageRelease(bsp);
  MicrosdInit();
  cli_println("done.");
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
    if (0 == strcmp("wipe", argv[0])){
      storage_cli_wipe(&Storage, sdp);
      return NULL;
    }
    else if (0 == strcmp("stat", argv[0])){
      cli_println("FIXME: something wrong with mutual exclusion");
      storage_cli_stat(&Storage, sdp);
      return NULL;
    }
  }

  cli_storage_print_help();
  return NULL;
}
