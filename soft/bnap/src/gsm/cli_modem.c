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

/*
 ******************************************************************************
 * DEFINES
 ******************************************************************************
 */
#define EEPROM_MODEM_MAX_FIELD_LEN    64

#define EEPROM_MODEM_PIN_OFFSET       0
#define EEPROM_MODEM_PIN_SIZE         sizeof("1234")

#define EEPROM_MODEM_APN_OFFSET       (EEPROM_MODEM_PIN_OFFSET + EEPROM_MODEM_PIN_SIZE)
#define EEPROM_MODEM_APN_SIZE         EEPROM_MODEM_MAX_FIELD_LEN

#define EEPROM_MODEM_USER_OFFSET      (EEPROM_MODEM_APN_OFFSET + EEPROM_MODEM_APN_SIZE)
#define EEPROM_MODEM_USER_SIZE        16

#define EEPROM_MODEM_PASS_OFFSET      (EEPROM_MODEM_USER_OFFSET + EEPROM_MODEM_USER_SIZE)
#define EEPROM_MODEM_PASS_SIZE        16

#define EEPROM_MODEM_SERVER_OFFSET    (EEPROM_MODEM_PASS_SIZE + EEPROM_MODEM_PASS_OFFSET)
#define EEPROM_MODEM_SERVER_SIZE      EEPROM_MODEM_MAX_FIELD_LEN

#define EEPROM_MODEM_PORT_OFFSET      (EEPROM_MODEM_SERVER_OFFSET + EEPROM_MODEM_SERVER_SIZE)
#define EEPROM_MODEM_PORT_SIZE        sizeof("65536")

#define EEPROM_MODEM_LISTEN_OFFSET    (EEPROM_MODEM_PORT_OFFSET + EEPROM_MODEM_PORT_SIZE)
#define EEPROM_MODEM_LISTEN_SIZE      sizeof("65536")

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
static uint8_t eeprom_buf[EEPROM_MODEM_MAX_FIELD_LEN];

static EepromFileStream ModemSettingsFile;

static const I2CEepromFileConfig modem_settings_file_cfg = {
  &EEPROM_I2CD,
  EEPROM_MODEM_SETTINGS_START,
  EEPROM_MODEM_SETTINGS_END,
  EEPROM_SIZE,
  EEPROM_PAGE_SIZE,
  EEPROM_I2C_ADDR,
  MS2ST(EEPROM_WRITE_TIME_MS),
  eeprom_buf,
};

/*
 ******************************************************************************
 * LOCAL FUNCTIONS
 ******************************************************************************
 */

/**
 *
 */
static void _modem_print_help(void){
  cli_println("Run without parameters to get current settings.");
//  cli_println("'param save' to save parameters to EEPROM.");
//  cli_println("'param help' to get this message.");
//  cli_println("'param PARAM_name' to get value of parameter.");
//  cli_println("'param PARAM_name N' to set value of parameter to N.");
}


/**
 *
 */
static bool_t _write_and_check(const char * val, EepromFileStream *f, size_t offset, size_t len){
  uint32_t status = 0;

  chFileStreamSeek(f, offset);
  status = chFileStreamWrite(f, (uint8_t *)val, len);
  if (status != len){
    cli_println("ERROR: performing eeprom write failed");
    return CH_FAILED;
  }

  /* check written data */
  chFileStreamSeek(f, offset);
  memset(eeprom_buf, 0, sizeof(eeprom_buf));
  status = chFileStreamRead(f, eeprom_buf, len);
  if (status != len){
    cli_println("ERROR: performing eeprom read failed");
    return CH_FAILED;
  }
  if (0 != strcmp(val, (char *)eeprom_buf)){
    cli_println("ERROR: checking written data failed");
    return CH_FAILED;
  }
  return CH_SUCCESS;
}

/**
 *
 */
static bool_t _set(const char * val, EepromFileStream *f, size_t size, size_t offset){
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
    return _write_and_check(val, f, offset, len);
}

/**
 *
 */
static bool_t _cli_modem_set(const char * const * argv, EepromFileStream *f){
  if (0 == strcmp(argv[0], "pin"))
    return _set(argv[1], f, EEPROM_MODEM_PIN_SIZE, EEPROM_MODEM_PIN_OFFSET);

  else if (0 == strcmp(argv[0], "apn"))
    return _set(argv[1], f, EEPROM_MODEM_APN_SIZE, EEPROM_MODEM_APN_OFFSET);

  else if (0 == strcmp(argv[0], "user"))
    return _set(argv[1], f, EEPROM_MODEM_USER_SIZE, EEPROM_MODEM_USER_OFFSET);

  else if (0 == strcmp(argv[0], "pass"))
    return _set(argv[1], f, EEPROM_MODEM_PASS_SIZE, EEPROM_MODEM_PASS_OFFSET);

  else if (0 == strcmp(argv[0], "server"))
    return _set(argv[1], f, EEPROM_MODEM_SERVER_SIZE, EEPROM_MODEM_SERVER_OFFSET);

  else if (0 == strcmp(argv[0], "port"))
    return _set(argv[1], f, EEPROM_MODEM_PORT_SIZE, EEPROM_MODEM_PORT_OFFSET);

  else if (0 == strcmp(argv[0], "listen"))
    return _set(argv[1], f, EEPROM_MODEM_LISTEN_SIZE, EEPROM_MODEM_LISTEN_OFFSET);

  return CH_FAILED;
}

/**
 *
 */
static bool_t _do_erase(EepromFileStream *f){
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
static bool_t _read_setting(uint8_t *buf, EepromFileStream *f,  size_t maxsize, size_t offset){
  size_t len;

  chFileStreamSeek(f, offset);
  len = chFileStreamRead(f, buf, maxsize);
  if (len == 0){
    cli_print("ERROR: failed to read data from EEPROM");
    return CH_FAILED;
  }
  else
    return CH_SUCCESS;
}

/**
 *
 */
static bool_t __printer(const char * str, EepromFileStream *f, size_t size, size_t offset){
  cli_print(str);
  memset(eeprom_buf, 0, sizeof(eeprom_buf));
  if (CH_SUCCESS == _read_setting(eeprom_buf, f, size, offset))
    cli_println((const char *)eeprom_buf);
  else
    cli_println("ERROR: read failed!");
  chThdSleepMilliseconds(50);
  return CH_SUCCESS;
}

/**
 *
 */
static bool_t _modem_print_all(EepromFileStream *f){
  __printer("pin:    ", f, EEPROM_MODEM_PIN_SIZE,     EEPROM_MODEM_PIN_OFFSET);
  __printer("apn:    ", f, EEPROM_MODEM_APN_SIZE,     EEPROM_MODEM_APN_OFFSET);
  __printer("user:   ", f, EEPROM_MODEM_USER_SIZE,    EEPROM_MODEM_USER_OFFSET);
  __printer("pass:   ", f, EEPROM_MODEM_PASS_SIZE,    EEPROM_MODEM_PASS_OFFSET);
  __printer("server: ", f, EEPROM_MODEM_SERVER_SIZE,  EEPROM_MODEM_SERVER_OFFSET);
  __printer("port:   ", f, EEPROM_MODEM_PORT_SIZE,    EEPROM_MODEM_PORT_OFFSET);
  __printer("listen: ", f, EEPROM_MODEM_LISTEN_SIZE,  EEPROM_MODEM_LISTEN_OFFSET);
  return CH_SUCCESS;
}

/**
 *
 */
static bool_t _modem_do(const char * const * argv, EepromFileStream *f){

  if (0 == strcmp(argv[0], "erase"))
    return _do_erase(f);

  else if (0 == strcmp(argv[0], "print"))
    return _modem_print_all(f);

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

  EepromFileOpen(&ModemSettingsFile, &modem_settings_file_cfg);

  /* no arguments */
  if (argc == 0)
    _modem_print_help();

  /* one argument */
  else if (argc == 1)
    status = _modem_do(argv, &ModemSettingsFile);

  /* two arguments */
  else if (argc == 2)
    status = _cli_modem_set(argv, &ModemSettingsFile);

  /*  */
  else
    status = CH_FAILED;

  chThdSleepMilliseconds(200);
  chFileStreamClose(&ModemSettingsFile);
  if (status != CH_SUCCESS){
    cli_println("ERROR: can not understand what do you want");
    _modem_print_help();
  }
  return NULL;
}
