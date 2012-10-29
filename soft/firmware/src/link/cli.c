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

#include "eeprom_conf.h"

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
static Thread* logout_clicmd(int argc, const char * const * argv, SerialDriver *sdp);
static Thread* help_clicmd(int argc, const char * const * argv, SerialDriver *sdp);

/*
 ******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************
 */

static const ShellCmd_t chibiutils[] = {
    {"ps",        &ps_clicmd,        "info about running threads"},
    {"uname",     &uname_clicmd,     "'info' alias"},
    {"help",      &help_clicmd,      "this message"},
    {"clear",     &clear_clicmd,     "clear screen"},
    {"list",      &list_clicmd,      NULL},
    {"logout",    &logout_clicmd,    "close shell threads and fork telemtry threads"},
    {"selftest",  &selftest_clicmd,  "exectute selftests"},
    {"sensors",   &sensors_clicmd,   "get human readable data from onboard sensors"},
    {"storage",   &storage_clicmd,   NULL},
    {"param",     &param_clicmd,     "manage onboard system paramters"},
    #if USE_EEPROM_TEST_SUIT
    {"eepromtest",&eepromtest_clicmd, "run EEPROM testsuit. Uses lots of RAM"},
    #endif
    {NULL,        NULL,               NULL}/* end marker */
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

Thread* help_clicmd(int argc, const char * const * argv, SerialDriver *sdp){
  (void)sdp;
  (void)argc;
  (void)argv;

  int32_t i = 0;

  cli_println("Use TAB key for completion, UpArrow for previous command.");
  cli_println("Available commands are:");
  cli_println("-------------------------------------------------------------");

  while(chibiutils[i].name != NULL){
    cli_print(chibiutils[i].name);
    cli_print(" - ");
    cli_println(chibiutils[i].help);
    i++;
  }

  return NULL;
}

Thread* list_clicmd(int argc, const char * const * argv, SerialDriver *sdp){
  (void)sdp;
  (void)argc;
  (void)argv;

  int i = 0;

  cli_print("available commands:\n\r");
  while(chibiutils[i].name != NULL){
    cli_print("\t");
    cli_print(chibiutils[i].name);
    cli_print("\n\r");
    i++;
  }
  return NULL;
}

/**
 *
 */
static Thread* logout_clicmd(int argc, const char * const * argv, SerialDriver *sdp){
  (void)sdp;
  (void)argc;
  (void)argv;
  *(uint32_t*)ValueSearch("SH_enable") = 0;
  return NULL;
}

/**
 * Search value (pointer to function) by key (name string)
 */
static int32_t cmd_search(const char* key, const ShellCmd_t *cmdarray){
  uint32_t i = 0;

  while (cmdarray[i].name != NULL){
    if (strcmp(key, cmdarray[i].name) == 0)
      return i;
    i++;
  }
  return -1;
}

//*****************************************************************************
// execute callback for microrl library
// do what you want here, but don't write to argv!!! read only!!
static int execute (int argc, const char * const * argv){
  int i = 0;

  /* search first token */
  i = cmd_search(argv[0], chibiutils);
  if (i == -1){
    cli_print ("command: '");
    cli_print ((char*)argv[0]);
    cli_print ("' Not found.\n\r");
  }
  else{
    if (argc > 1)
      CurrentCmd_tp = chibiutils[i].func(argc - 1, &argv[1], shell_sdp);
    else
      CurrentCmd_tp = chibiutils[i].func(0, NULL, shell_sdp);
  }
  return 0;
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







