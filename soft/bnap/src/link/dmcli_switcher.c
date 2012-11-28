#include "ch.h"
#include "hal.h"
#include "chprintf.h"

#include "main.h"
#include "link_dm.h"
#include "cli.h"
#include "param.h"

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
static uint32_t const *sh_enable;

/*
 *******************************************************************************
 *******************************************************************************
 * LOCAL FUNCTIONS
 *******************************************************************************
 *******************************************************************************
 */

/**
 * Track changes of sh_enable flag and fork appropriate threads
 */
static WORKING_AREA(DmCliSwitcherThreadWA, 64);
static msg_t DmCliSwitcherThread(void *arg){
  chRegSetThreadName("DmCliSwitcher");

  bool_t shell_active = FALSE;

  /* define what we need to run based on flag */
  if (*sh_enable == 0){
    spawn_dm_threads((SerialDriver *)arg);
    shell_active = FALSE;
  }
  else{
    spawn_shell_threads((SerialDriver *)arg);
    shell_active = TRUE;
  }

  /* now track changes of flag and fork appropriate threads */
  while (TRUE) {
    chThdSleepMilliseconds(100);
    if(shell_active == TRUE){
      if(*sh_enable == 0){
        kill_shell_threads();
        spawn_dm_threads((SerialDriver *)arg);
        shell_active = FALSE;
      }
    }
    else{
      if(*sh_enable == 1){
        kill_dm_threads();
        spawn_shell_threads((SerialDriver *)arg);
        shell_active = TRUE;
      }
    }
  }

  return 0;
}

/*
 *******************************************************************************
 * EXPORTED FUNCTIONS
 *******************************************************************************
 */
void DmCliSwitcherInit(SerialDriver *sdp){
  sh_enable = ValueSearch("SH_enable");

  chThdCreateStatic(DmCliSwitcherThreadWA,
          sizeof(DmCliSwitcherThreadWA),
          NORMALPRIO,
          DmCliSwitcherThread,
          sdp);
}
