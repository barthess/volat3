#include <string.h>

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

/*
 ******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************
 */
EepromFileStream ModemSettingsFile;

static uint8_t __buf[EEPROM_TX_DEPTH];

static const I2CEepromFileConfig modem_settings_file_cfg = {
  &EEPROM_I2CD,
  EEPROM_MODEM_SETTINGS_START,
  EEPROM_MODEM_SETTINGS_END,
  EEPROM_SIZE,
  EEPROM_PAGE_SIZE,
  EEPROM_I2C_ADDR,
  MS2ST(EEPROM_WRITE_TIME_MS),
  __buf,
};

/*
 ******************************************************************************
 * LOCAL FUNCTIONS
 ******************************************************************************
 */

/*
 ******************************************************************************
 * EXPORTED FUNCTIONS
 ******************************************************************************
 */

void ModemSettingsInit(void){
  EepromFileOpen(&ModemSettingsFile, &modem_settings_file_cfg);
}

/**
 *
 */
bool_t read_modem_param(uint8_t *buf, EepromFileStream *f,  size_t maxsize, size_t offset){
  size_t len;

  memset(buf, 0, maxsize);
  chFileStreamSeek(f, offset);
  len = chFileStreamRead(f, buf, maxsize);
  if (len == 0)
    return CH_FAILED;
  else
    return CH_SUCCESS;
}
