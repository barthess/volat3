/*
 *  Works with persistant storage (EEPROM)
 */

#include <string.h>

#include "ch.h"
#include "hal.h"

#include "message.h"
#include "param.h"
#include "main.h"
#include "persistant.h"
#include "eeprom.h"


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
extern EepromFileStream EepromSettingsFile;
extern GlobalParam_t global_data[];
extern const uint32_t OnboardParamCount;

/*
 ******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************
 */
static uint8_t eeprombuf[PARAM_ID_SIZE + sizeof(global_data[0].value)];
static uint8_t confirmbuf[sizeof(eeprombuf)];

/*
 *******************************************************************************
 *******************************************************************************
 * LOCAL FUNCTIONS
 *******************************************************************************
 *******************************************************************************
 */


/**
 *
 */
bool_t load_params_from_eeprom(void){
  uint32_t i = 0;
  int32_t  index = -1;
  uint32_t status = 0;
  uint32_t v = 0;

  chFileStreamSeek(&EepromSettingsFile, 0);

  for (i = 0; i < OnboardParamCount; i++){

    /* reade field from EEPROM and check number of read bytes */
    status = chFileStreamRead(&EepromSettingsFile, eeprombuf, sizeof(eeprombuf));
    if (status < sizeof(eeprombuf))
      return PARAM_FAILED;

    /* search value by key and set it if found */
    index = _key_index_search((char *)eeprombuf);
    if (index != -1){
      v = eeprombuf[PARAM_ID_SIZE + 0] << 24 |
          eeprombuf[PARAM_ID_SIZE + 1] << 16 |
          eeprombuf[PARAM_ID_SIZE + 2] << 8  |
          eeprombuf[PARAM_ID_SIZE + 3];
    }

    /* check value acceptability */
    set_global_param(&v, &(global_data[i]));
  }
  return PARAM_SUCCESS;
}

/**
 *
 */
bool_t save_params_to_eeprom(void){
  uint32_t i, j;
  uint32_t status = 0;
  uint32_t v = 0;

  chFileStreamSeek(&EepromSettingsFile, 0);

  for (i = 0; i < OnboardParamCount; i++){
    palClearPad(GPIOE, GPIOE_LED);
    /* first copy parameter name in buffer */
    memcpy(eeprombuf, global_data[i].name, PARAM_ID_SIZE);

    /* now write data */
    v = global_data[i].value.u32;
    eeprombuf[PARAM_ID_SIZE + 0] = (v >> 24) & 0xFF;
    eeprombuf[PARAM_ID_SIZE + 1] = (v >> 16) & 0xFF;
    eeprombuf[PARAM_ID_SIZE + 2] = (v >> 8)  & 0xFF;
    eeprombuf[PARAM_ID_SIZE + 3] = (v >> 0)  & 0xFF;

    status = chFileStreamWrite(&EepromSettingsFile, eeprombuf, sizeof(eeprombuf));
    if (status < sizeof(eeprombuf))
      chDbgPanic("write failed");

    /* check written data */
    chFileStreamSeek(&EepromSettingsFile, chFileStreamGetPosition(&EepromSettingsFile) - sizeof(eeprombuf));
    status = chFileStreamRead(&EepromSettingsFile, confirmbuf, sizeof(confirmbuf));
    for (j = 0; j < (PARAM_ID_SIZE + sizeof(v)); j++){
      if (confirmbuf[j] != eeprombuf[j])
        chDbgPanic("veryfication failed");
    }
  }
  palSetPad(GPIOE, GPIOE_LED);
  return 0;
}


