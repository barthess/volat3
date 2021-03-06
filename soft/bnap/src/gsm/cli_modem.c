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
#include "eeprom.h"
#include "eeprom_conf.h"
#include "settings_modem.h"
#include "cross.h"
#include "link.h"

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
extern EepromFileStream ModemSettingsFile;

extern Thread *link_cc_unpacker_tp;
extern Thread *link_cc_packer_tp;
//extern Thread *link_dm_unpacker_tp;
//extern Thread *link_dm_packer_tp;
extern Thread *link_mpiovd_unpacker_tp;
extern Thread *microsd_writer_tp;
extern Thread *modem_tp;
extern Thread *gps_tp;

/*
 ******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************
 */
static uint8_t eeprombuf[EEPROM_MODEM_MAX_FIELD_LEN];

/*
 ******************************************************************************
 * LOCAL FUNCTIONS
 ******************************************************************************
 */

/**
 *
 */
static void cli_modem_print_help(void){
  cli_println("Available subcommands:");
  cli_println("  'pin XXXX' set PIN code");
  cli_println("  'apn XXXX' set APN");
  cli_println("  'user XXXX' set user name");
  cli_println("  'pass XXXX' set password corresponding to user");
  cli_println("  'server XXXX' set server address (URI of IP");
  cli_println("  'port XXXX' set remote destination port on server (14550 default for QGC)");
  cli_println("  'listen XXXX' set local port to listen server respose (14555 default for QGC)");
  cli_println("");
  cli_println("  'print' pritns currently stored in EEPROM settings");
  cli_println("  'erase' erase current settings from EEPROM");
  cli_println("  'cross' start crossed connection between modem uart and dysplay manager uart");
  chThdSleepMilliseconds(100);
}

/**
 *
 */
static bool_t __write_and_check(const char * val, EepromFileStream *f, size_t offset){
  uint32_t status = 0;
  size_t len = strlen(val) + 1; /* 1 NULL char */

  chFileStreamSeek(f, offset);
  status = chFileStreamWrite(f, (uint8_t *)val, len);
  if (status != len){
    cli_println("ERROR: performing eeprom write failed");
    return CH_FAILED;
  }

  /* check written data */
  chFileStreamSeek(f, offset);
  memset(eeprombuf, 0, sizeof(eeprombuf));
  status = chFileStreamRead(f, eeprombuf, len);
  if (status != len){
    cli_println("ERROR: performing eeprom read failed");
    return CH_FAILED;
  }
  if (0 != strcmp(val, (char *)eeprombuf)){
    cli_println("ERROR: checking written data failed");
    return CH_FAILED;
  }
  return CH_SUCCESS;
}

/**
 *
 */
static bool_t __set(const char * val, EepromFileStream *f, size_t size, size_t offset){
  size_t len = strlen(val);

  if ((offset == EEPROM_MODEM_PIN_OFFSET) && (len != 4)){
    cli_println("ERROR: PIN code incorrect");
    return CH_FAILED;
  }
  else if (len > size - 1){
    cli_println("ERROR: string too long");
    return CH_FAILED;
  }
  else
    return __write_and_check(val, f, offset);
}

/**
 *
 */
static bool_t modem_cli_set_param(const char * const * argv, EepromFileStream *f, SerialDriver *sdp){
  (void)sdp;

  if (0 == strcmp(argv[0], "pin"))
    return __set(argv[1], f, EEPROM_MODEM_PIN_SIZE, EEPROM_MODEM_PIN_OFFSET);

  else if (0 == strcmp(argv[0], "apn"))
    return __set(argv[1], f, EEPROM_MODEM_APN_SIZE, EEPROM_MODEM_APN_OFFSET);

  else if (0 == strcmp(argv[0], "user"))
    return __set(argv[1], f, EEPROM_MODEM_USER_SIZE, EEPROM_MODEM_USER_OFFSET);

  else if (0 == strcmp(argv[0], "pass"))
    return __set(argv[1], f, EEPROM_MODEM_PASS_SIZE, EEPROM_MODEM_PASS_OFFSET);

  else if (0 == strcmp(argv[0], "server"))
    return __set(argv[1], f, EEPROM_MODEM_SERVER_SIZE, EEPROM_MODEM_SERVER_OFFSET);

  else if (0 == strcmp(argv[0], "port"))
    return __set(argv[1], f, EEPROM_MODEM_PORT_SIZE, EEPROM_MODEM_PORT_OFFSET);

  else if (0 == strcmp(argv[0], "listen"))
    return __set(argv[1], f, EEPROM_MODEM_LISTEN_SIZE, EEPROM_MODEM_LISTEN_OFFSET);

  return CH_FAILED;
}

/**
 *
 */
static bool_t modem_cli_do_erase(EepromFileStream *f){
  uint32_t n = 0;
  uint8_t b[1] = {0};
  size_t len = chFileStreamGetSize(f);

  cli_print("Erasing. Please wait... ");
  chFileStreamSeek(f, 0);
  while (n < len){
    chFileStreamWrite(f, b, 1);
    n++;
  }
  cli_println("Done!");
  return CH_SUCCESS;
}

/**
 *
 */
static bool_t __print(const char * str, EepromFileStream *f, size_t size, size_t offset){
  cli_print(str);
  memset(eeprombuf, 0, sizeof(eeprombuf));
  if (CH_SUCCESS == read_modem_param(eeprombuf, f, size, offset))
    cli_println((const char *)eeprombuf);
  else
    cli_println("ERROR: read failed!");
  chThdSleepMilliseconds(25);
  return CH_SUCCESS;
}

/**
 *
 */
static bool_t cli_modem_print_all(EepromFileStream *f){
  __print("pin:    ", f, EEPROM_MODEM_PIN_SIZE,     EEPROM_MODEM_PIN_OFFSET);
  __print("apn:    ", f, EEPROM_MODEM_APN_SIZE,     EEPROM_MODEM_APN_OFFSET);
  __print("user:   ", f, EEPROM_MODEM_USER_SIZE,    EEPROM_MODEM_USER_OFFSET);
  __print("pass:   ", f, EEPROM_MODEM_PASS_SIZE,    EEPROM_MODEM_PASS_OFFSET);
  __print("server: ", f, EEPROM_MODEM_SERVER_SIZE,  EEPROM_MODEM_SERVER_OFFSET);
  __print("port:   ", f, EEPROM_MODEM_PORT_SIZE,    EEPROM_MODEM_PORT_OFFSET);
  __print("listen: ", f, EEPROM_MODEM_LISTEN_SIZE,  EEPROM_MODEM_LISTEN_OFFSET);
  return CH_SUCCESS;
}

/**
 *
 */
static bool_t cli_modem_start_cross(SerialDriver *sdp){

#if !defined(NEED_MODEM_CROSS)
  cli_println("ERROR!");
  cli_println("You can not use crossing because firmware compiled with debugging enabled.");
  cli_println("This cause not enough perfomance to exchange data without loss.");
  cli_println("Define 'NEED_MODEM_CROSS' in 'chconf.h' and rebuild firmware.");
  chThdSleepMilliseconds(50);
  return CH_FAILED;
#endif

  cli_println("WARNING!");
  cli_println("  There is no software way to disable cross and return to shell.");
  cli_println("  You must hard reboot device after the work done.");
  cli_println("Notes:");
  cli_println("* to enable echo print 'ATE1'");
  cli_println("* to correctly stop ongoing internet connection (if any) use '+++' combo");
  cli_println("with one second guard intervals, than 'AT+WIPCLOSE=1,1'");

  cli_println("");
  chThdSleepMilliseconds(50);

  /* stop all threads using needed ports */
  cli_terminate_thread(modem_tp, sdp);
  cli_terminate_thread(link_cc_packer_tp, sdp);
  cli_terminate_thread(link_cc_unpacker_tp, sdp);
  /* NOTE! After starting shell the DM threads allready killed and joined to Shell thread */
  //  _cli_modem_terminate_thd(link_dm_packer_tp, sdp);
  //  _cli_modem_terminate_thd(link_dm_unpacker_tp, sdp);

  /* stop other heavy weight threads */
  cli_terminate_thread(link_mpiovd_unpacker_tp, sdp);
  cli_terminate_thread(microsd_writer_tp, sdp);
  cli_terminate_thread(gps_tp, sdp);

  /* gain mutual exclusion on ports */
  cli_print("acquiring mutual access to serial drivers... ");
  acquire_cc_out();
  acquire_cc_in();
  acquire_dm_out();
  acquire_dm_in();
  cli_println("done");

  /* fire up cross */
  ModemCrossInit();

  /* program will never return to shell from here. You need hard reboot. */
  chThdExit(0);

  return CH_SUCCESS;
}

/**
 *
 */
static bool_t cli_modem_do(const char * const * argv, EepromFileStream *f, SerialDriver *sdp){

  if (0 == strcmp(argv[0], "erase"))
    return modem_cli_do_erase(f);

  else if (0 == strcmp(argv[0], "print"))
    return cli_modem_print_all(f);

  else if (0 == strcmp(argv[0], "cross"))
    return cli_modem_start_cross(sdp);

  else if (0 == strcmp(argv[0], "down"))
    return CH_FAILED;

  else if (0 == strcmp(argv[0], "up"))
    return CH_FAILED;

  return CH_FAILED;
}


/*
 ******************************************************************************
 * EXPORTED FUNCTIONS
 ******************************************************************************
 */

/**
 * Working with parameters from CLI.
 */
Thread* modem_clicmd(int argc, const char * const * argv, SerialDriver *sdp){
  (void)sdp;
  bool_t status = CH_SUCCESS;

  /* no arguments */
  if (argc == 0)
    cli_modem_print_help();

  /* one argument */
  else if (argc == 1)
    status = cli_modem_do(argv, &ModemSettingsFile, sdp);

  /* two arguments */
  else if (argc == 2)
    status = modem_cli_set_param(argv, &ModemSettingsFile, sdp);

  /*  */
  else
    status = CH_FAILED;

  chThdSleepMilliseconds(200);
  if (status != CH_SUCCESS){
    cli_println("ERROR: can not understand what do you want");
    cli_modem_print_help();
  }
  return NULL;
}
