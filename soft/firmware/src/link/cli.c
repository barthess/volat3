#include <string.h>
#include <stdlib.h>

#include "ch.h"
#include "hal.h"

#include "../microrl/src/microrl.h"
#include "chprintf.h"

#include "main.h"
#include "message.h"
#include "cli.h"
#include "cli_cmd.h"
#include "sensors_cmd.h"
#include "storage.h"
#include "param_cli.h"
#include "param.h"

/*
 ******************************************************************************
 * DEFINES
 ******************************************************************************
 */
#define _NUM_OF_CMD (sizeof(chibiutils)/sizeof(ShellCmd_t))

/*
 ******************************************************************************
 * EXTERNS
 ******************************************************************************
 */
extern MemoryHeap ThdHeap;

/*
 *******************************************************************************
 * PROTOTYPES
 *******************************************************************************
 */
int recursive_execute(int argc, const char * const * argv, const ShellCmd_t *cmdarray);
static Thread* logout_cmd(int argc, const char * const * argv, const ShellCmd_t *cmdarray);

/*
 ******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************
 */

static const ShellCmd_t chibiutils[] = {
    {"ps",        &ps_cmd,        NULL},
    {"uname",     &uname_cmd,     NULL},
    {"help",      &help_cmd,      NULL},
    {"clear",     &clear_cmd,     NULL},
    {"list",      &list_cmd,      NULL},
    {"logout",    &logout_cmd,    NULL},
    {"selftest",  &selftest_cmd,  NULL},
    {"sensors",   &sensors_cmd,   NULL},
    {"storage",   &storage_cmd,   NULL},
    {"param",     &param_cmd,     NULL},
    {NULL,        NULL,           NULL}/* end marker */
};

static SerialDriver *shell_sdp;

// array for comletion
static char *compl_world[_NUM_OF_CMD + 1];

/* thread pointer to currently executing command */
static Thread *CurrentCmd_tp = NULL;

/* pointer to shell thread */
static Thread *shell_tp = NULL;

/*
 *******************************************************************************
 * LOCAL FUNCTIONS
 *******************************************************************************
 */

/**
 *
 */
static Thread* logout_cmd(int argc, const char * const * argv, const ShellCmd_t *cmdarray){
  (void)argc;
  (void)argv;
  (void)cmdarray;

  *(uint32_t*)ValueSearch("SH_enable") = 0;

  return NULL;
}

/**
 * Search value (pointer to function) by key (name string)
 */
int32_t cmd_search(const char* key, const ShellCmd_t *cmdarray){
  uint32_t i = 0;

  while (cmdarray[i].name != NULL){
    if (strcmp(key, cmdarray[i].name) == 0)
      return i;
    i++;
  }
  return -1;
}

/**
 * Print routine
 */
void cli_print(const char *str){
  int i = 0;
  while (str[i] != 0) {
    sdPutTimeout(shell_sdp, str[i], MS2ST(100));
    i++;
  }
}

/**
 * Convenience function
 */
void cli_println(const char *str){
  cli_print(str);
  cli_print(ENDL);
}

/**
 * Read routine
 */
char get_char (void){
  return sdGet(shell_sdp);
}

/**
 * helper function
 * Inserts new line symbol if passed string does not contain NULL termination.
 * Must be used in combination with snprintf() function.
 */
void cli_print_long(const char * str, int n, int nres){
  cli_print(str);
  if (nres > n)
    cli_print(ENDL);
}

//*****************************************************************************
// execute callback for microrl library
// do what you want here, but don't write to argv!!! read only!!
int execute (int argc, const char * const * argv){
  if (recursive_execute(argc, argv, chibiutils) == -1){
    cli_print ("command: '");
    cli_print ((char*)argv[0]);
    cli_print ("' Not found.\n\r");
  }
  return 0;
}

int recursive_execute(int argc, const char * const * argv, const ShellCmd_t *cmdarray){
  int i = 0;

  /* search first token */
  i = cmd_search(argv[0], cmdarray);

  /* search token in cmd array */
  if (i != -1){
    if (argc > 1)
      CurrentCmd_tp = cmdarray[i].func(argc - 1, &argv[1], cmdarray);
    else
      CurrentCmd_tp = cmdarray[i].func(argc - 1, NULL, cmdarray);
  }
  return i;
}



#ifdef _USE_COMPLETE
//*****************************************************************************
// completion callback for microrl library
char ** complete(int argc, const char * const * argv)
{
  int j = 0;
  int i = 0;

  compl_world[0] = NULL;

  // if there is token in cmdline
  if (argc == 1) {
    // get last entered token
    char * bit = (char*)argv [argc-1];
    // iterate through our available token and match it
    while (chibiutils[i].name != NULL){
      if (strstr(chibiutils[i].name, bit) == chibiutils[i].name)
        compl_world[j++] = chibiutils[i].name;
      i++;
    }
  }
  else { // if there is no token in cmdline, just print all available token
    while (chibiutils[j].name != NULL){
      compl_world[j] = chibiutils[j].name;
      j++;
    }
  }

  // note! last ptr in array always must be NULL!!!
  compl_world[j] = NULL;
  // return set of variants
  return compl_world;
}
#endif


/**
 *
 */
void sigint (void){
  if (CurrentCmd_tp != NULL){
    chThdTerminate(CurrentCmd_tp);
    chThdWait(CurrentCmd_tp);
    CurrentCmd_tp = NULL;
  }
  cli_print("--> ^C pressed. Hit enter to return in console.");
}

/**
 * Thread function
 */
static WORKING_AREA(ShellThreadWA, 1536);
static msg_t ShellThread(void *arg){
  chRegSetThreadName("Shell");

  /* init static pointer for serial driver with received pointer */
  shell_sdp = (SerialDriver *)arg;
  chThdSleepMilliseconds(1000);
  // create and init microrl object
  microrl_t microrl_shell;
  // cli_print("@@*** Super cool device, version 1.2.3, for help type help... ***@@\r\n");
  microrl_init(&microrl_shell, cli_print);

  // set callback for execute
  microrl_set_execute_callback(&microrl_shell, execute);

  // set callback for completion (optionally)
  microrl_set_complete_callback(&microrl_shell, complete);

  // set callback for ctrl+c handling (optionally)
  microrl_set_sigint_callback(&microrl_shell, sigint);

  while (TRUE){
    // put received char from stdin to microrl lib
    msg_t c = sdGetTimeout(shell_sdp, MS2ST(50));
    if (c != Q_TIMEOUT)
      microrl_insert_char(&microrl_shell, (char)c);

    /* умираем по всем правилам, не забываем убить потомков */
    if (chThdShouldTerminate()){
      if ((CurrentCmd_tp != NULL) && (CurrentCmd_tp->p_state != THD_STATE_FINAL)){
        chThdTerminate(CurrentCmd_tp);
        chThdWait(CurrentCmd_tp);
      }
      chThdExit(0);
    }
  }
  return 0;
}



/*
 *******************************************************************************
 * EXPORTED FUNCTIONS
 *******************************************************************************
 */
/**
 *
 */
void KillShellThreads(void){
  if (shell_tp != NULL){
    chThdTerminate(shell_tp);
    chThdWait(shell_tp);
  }
}

/**
 *
 */
Thread* SpawnShellThreads(SerialDriver *sdp){

  shell_tp = chThdCreateFromHeap(&ThdHeap,
                                sizeof(ShellThreadWA),
                                LINK_THREADS_PRIO - 2,
                                ShellThread,
                                sdp);

  if (shell_tp == NULL)
    chDbgPanic("Can not allocate memory");

  return shell_tp;
}







