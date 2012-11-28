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
#include "eeprom.h"
#include "eeprom_conf.h"
#include "settings_modem.h"

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
  cli_println("  'port XXXX' set remote destination port number on server");
  cli_println("  'listen XXXX' set local port to listen server respose");

  cli_println("  'print' pritns currently stored in EEPROM settings");
  cli_println("  'erase' erase current settings from EEPROM");
  chThdSleepMilliseconds(100);
}

/**
 *
 */
static bool_t __write_and_check(const char * val, EepromFileStream *f, size_t offset, size_t len){
  uint32_t status = 0;

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
    return __write_and_check(val, f, offset, len);
}

/**
 *
 */
static bool_t modem_cli_set_param(const char * const * argv, EepromFileStream *f){
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
  chThdSleepMilliseconds(50);
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
static bool_t cli_modem_do(const char * const * argv, EepromFileStream *f){

  if (0 == strcmp(argv[0], "erase"))
    return modem_cli_do_erase(f);

  else if (0 == strcmp(argv[0], "print"))
    return cli_modem_print_all(f);

  else if (0 == strcmp(argv[0], "cross"))
    return CH_FAILED;

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
    status = cli_modem_do(argv, &ModemSettingsFile);

  /* two arguments */
  else if (argc == 2)
    status = modem_cli_set_param(argv, &ModemSettingsFile);

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
