#include <math.h>
#include <string.h>

#include "ch.h"
#include "hal.h"

#include "message.h"
#include "param.h"
#include "persistant.h"
#include "main.h"
#include "eeprom.h"
#include "link.h"
#include "utils.h"

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
extern Mailbox mavlink_param_set_mb;
extern Mailbox tolink_mb;
extern mavlink_system_t mavlink_system_struct;

/**
 * Default boundary checker.
 */
static bool_t default_setval(float value,  GlobalParam_t *param){
  float initial_value = value;

  putinrange(value, param->min, param->max);
  param->value = value;

  if (value == initial_value)
    return PARAM_SUCCESS;
  else
    return PARAM_FAILED;
}

/**
 * Default boundary checker for integer values.
 */
static bool_t int_setval(float value,  GlobalParam_t *param){
  float initial_value = value;

  putinrange(value, param->min, param->max);
  param->value = roundf(value);

  if (value == initial_value)
    return PARAM_SUCCESS;
  else
    return PARAM_FAILED;
}

GlobalParam_t global_data[] = {
  /*  key             min         val         max         type                    checker_fucntion   */
  /*--------------------------------------------------------------------------------------------------*/
  {"SYS_ID",          1,          20,         255,        MAVLINK_TYPE_UINT32_T,  default_setval},
  /* тип "автопилота" (см. MAV_TYPE enum)
   * дл€ возможности переключени€ между машиной и самолетом. »зменени€
   * вступают в силу только после ребута. */
  {"SYS_mavtype",     0,          1,          16,         MAVLINK_TYPE_UINT32_T,  default_setval},

  // оэффициенты полиномов дл€ пересчета показаний с датчиков
  {"AN_ch01_c1",      -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   default_setval},
  {"AN_ch01_c2",      -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   default_setval},
  {"AN_ch01_c3",      -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   default_setval},

  {"AN_ch02_c1",      -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   default_setval},
  {"AN_ch02_c2",      -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   default_setval},
  {"AN_ch02_c3",      -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   default_setval},

  {"AN_ch03_c1",      -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   default_setval},
  {"AN_ch03_c2",      -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   default_setval},
  {"AN_ch03_c3",      -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   default_setval},

  {"AN_ch04_c1",      -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   default_setval},
  {"AN_ch04_c2",      -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   default_setval},
  {"AN_ch04_c3",      -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   default_setval},

  {"AN_ch05_c1",      -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   default_setval},
  {"AN_ch05_c2",      -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   default_setval},
  {"AN_ch05_c3",      -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   default_setval},

  {"AN_ch06_c1",      -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   default_setval},
  {"AN_ch06_c2",      -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   default_setval},
  {"AN_ch06_c3",      -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   default_setval},

  {"AN_ch07_c1",      -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   default_setval},
  {"AN_ch07_c2",      -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   default_setval},
  {"AN_ch07_c3",      -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   default_setval},

  {"AN_ch08_c1",      -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   default_setval},
  {"AN_ch08_c2",      -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   default_setval},
  {"AN_ch08_c3",      -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   default_setval},

  {"AN_ch09_c1",      -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   default_setval},
  {"AN_ch09_c2",      -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   default_setval},
  {"AN_ch09_c3",      -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   default_setval},

  {"AN_ch10_c1",      -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   default_setval},
  {"AN_ch10_c2",      -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   default_setval},
  {"AN_ch10_c3",      -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   default_setval},

  {"AN_ch11_c1",      -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   default_setval},
  {"AN_ch11_c2",      -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   default_setval},
  {"AN_ch11_c3",      -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   default_setval},

  {"AN_ch12_c1",      -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   default_setval},
  {"AN_ch12_c2",      -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   default_setval},
  {"AN_ch12_c3",      -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   default_setval},

  {"AN_ch13_c1",      -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   default_setval},
  {"AN_ch13_c2",      -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   default_setval},
  {"AN_ch13_c3",      -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   default_setval},

  {"AN_ch14_c1",      -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   default_setval},
  {"AN_ch14_c2",      -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   default_setval},
  {"AN_ch14_c3",      -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   default_setval},

  {"AN_ch15_c1",      -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   default_setval},
  {"AN_ch15_c2",      -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   default_setval},
  {"AN_ch15_c3",      -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   default_setval},

  {"AN_ch16_c1",      -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   default_setval},
  {"AN_ch16_c2",      -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   default_setval},
  {"AN_ch10_c3",      -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   default_setval},

  /****** разные временнџе интервалы в mS */
  // пакеты с телеметрией
  {"T_tlm",           SEND_OFF,   100,        SEND_MAX,   MAVLINK_TYPE_UINT32_T,  int_setval},
  // интервал между сохранени€ми пробега и моточасов в eeprom
  {"T_save_trip",     SEND_OFF,   100,        SEND_MAX,   MAVLINK_TYPE_UINT32_T,  int_setval},
  // пакеты heartbeat
  {"T_heartbeat",     SEND_OFF,   100,        SEND_MAX,   MAVLINK_TYPE_UINT32_T,  int_setval},
  {"T_reserved1",     SEND_OFF,   100,        SEND_MAX,   MAVLINK_TYPE_UINT32_T,  int_setval},
  {"T_reserved2",     SEND_OFF,   100,        SEND_MAX,   MAVLINK_TYPE_UINT32_T,  int_setval},
  {"T_reserved3",     SEND_OFF,   100,        SEND_MAX,   MAVLINK_TYPE_UINT32_T,  int_setval},
  {"T_reserved4",     SEND_OFF,   100,        SEND_MAX,   MAVLINK_TYPE_UINT32_T,  int_setval},
  {"T_reserved5",     SEND_OFF,   100,        SEND_MAX,   MAVLINK_TYPE_UINT32_T,  int_setval},
  {"T_reserved6",     SEND_OFF,   100,        SEND_MAX,   MAVLINK_TYPE_UINT32_T,  int_setval},

  /* fake field with 14 symbols name */
  {"fake_14_bytes_",  1,          1048,       1224,       MAVLINK_TYPE_UINT32_T,  default_setval},
};

const uint32_t ONBOARD_PARAM_COUNT = (sizeof(global_data) / sizeof(GlobalParam_t));

/*
 ******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************
 */
static Mailbox param_confirm_mb;
static msg_t param_confirm_mb_buf[1];


/*
 *******************************************************************************
 *******************************************************************************
 * LOCAL FUNCTIONS
 *******************************************************************************
 *******************************************************************************
 */

/**
 * Checks parameter and writes it to global struct.
 */
static bool_t set_parameter(mavlink_param_set_t *paramset){
  int32_t index = -1;

  index = _key_index_search(paramset->param_id);

  if (index >= 0){
    // Only write and emit changes if there is actually a difference
    // AND only write if new value is NOT "not-a-number"
    // AND is NOT infinity
    if ((paramset->param_type == MAVLINK_TYPE_FLOAT) &&
        (isnan(paramset->param_value) || isinf(paramset->param_value))){
      return PARAM_FAILED;
    }
    if (global_data[index].value == paramset->param_value){
      return PARAM_FAILED;
    }

    /* If value fall out of min..max bound than just set nearest allowable value */
    return global_data[index].setval(paramset->param_value, &global_data[index]);
  }

  /* default returning value */
  return PARAM_FAILED;
}

/**
 * @brief   Sends mails to communication thread
 *
 * @param[in] key   if NULL than perform search by index
 * @param[in] n     search index
 */
static bool_t send_value(Mail *param_value_mail,
                         mavlink_param_value_t *param_value_struct,
                         char *key,
                         uint32_t n){
  int32_t index = -1;
  msg_t status = RDY_TIMEOUT;
  msg_t tmp = 0;
  uint32_t j = 0;

  if (key != NULL)
    index = _key_index_search(key);
  else
    index = n;

  if ((index >= 0) && (index <= (int)ONBOARD_PARAM_COUNT)){
    /* fill all fields */
    param_value_struct->param_value = global_data[index].value;
    param_value_struct->param_count = ONBOARD_PARAM_COUNT;
    param_value_struct->param_index = index;
    param_value_struct->param_type  = MAVLINK_TYPE_FLOAT;
    for (j = 0; j < ONBOARD_PARAM_NAME_LENGTH; j++)
      param_value_struct->param_id[j] = global_data[index].name[j];

    /* send */
    param_value_mail->payload = param_value_struct;
    //status = chMBPostAhead(&tolink_mb, (msg_t)param_value_mail, MS2ST(5));
    status = chMBPost(&tolink_mb, (msg_t)param_value_mail, MS2ST(5));
    if (status != RDY_OK)
      return PARAM_FAILED;

    /* wait until message processed */
    chMBFetch(&param_confirm_mb, &tmp, TIME_INFINITE);
  }
  else
    return PARAM_FAILED;

  return SUCCESS;
}

/**
 * Send all values one by one.
 */
static void send_all_values(Mail *mail, mavlink_param_value_t *param_struct){
  uint32_t i = 0;
  for (i = 0; i < ONBOARD_PARAM_COUNT; i++){
    send_value(mail, param_struct, NULL, i);
  }
}

/**
 * ѕоток принимающий сообщени€ с параметрами и отправл€ющий параметры по запросу.
 */
static WORKING_AREA(ParametersThreadWA, 512);
static msg_t ParametersThread(void *arg){
  chRegSetThreadName("Parameters");
  (void)arg;

  /* переменные дл€ отправки установленных параметров */
  mavlink_param_value_t param_value_struct;
  Mail param_value_mail = {NULL, MAVLINK_MSG_ID_PARAM_VALUE, &param_confirm_mb};

  /* переменные дл€ приема параметров */
  msg_t tmp = 0;
  Mail *input_mail = NULL;
  mavlink_param_set_t *set = NULL;
  mavlink_param_request_list_t *list = NULL;
  mavlink_param_request_read_t *read = NULL;
  bool_t status = PARAM_FAILED;

  while (TRUE) {
    chMBFetch(&mavlink_param_set_mb, &tmp, TIME_INFINITE);
    input_mail = (Mail*)tmp;

    switch (input_mail->invoice){
    /*
     * согласно протоколу, при успешной установке параметра, мы должны
     * вычитать и выслать в ответ этот параметр в качестве подтверждени€
     */
    case MAVLINK_MSG_ID_PARAM_SET:
      set = (mavlink_param_set_t *)(input_mail->payload);
      input_mail->payload = NULL;
      status = set_parameter(set);
      if (status == SUCCESS){
        send_value(&param_value_mail, &param_value_struct, set->param_id, 0);
      }
      break;

    /*
     * запрос всех параметров
     */
    case MAVLINK_MSG_ID_PARAM_REQUEST_LIST:
      list = (mavlink_param_request_list_t *)(input_mail->payload);
      input_mail->payload = NULL;
      if (list->target_system == mavlink_system_struct.sysid)
        send_all_values(&param_value_mail, &param_value_struct);
      break;

    /*
     * запрос одного параметра
     */
    case MAVLINK_MSG_ID_PARAM_REQUEST_READ:
      read = (mavlink_param_request_read_t *)(input_mail->payload);
      input_mail->payload = NULL;
      if (read->param_index >= 0)
        send_value(&param_value_mail, &param_value_struct, NULL, read->param_index);
      else
        send_value(&param_value_mail, &param_value_struct, read->param_id, 0);
      break;
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
 * @brief   Performs key-value search. Low level function
 *
 * @return      Index in dictionary.
 * @retval -1   key not found.
 */
int32_t _key_index_search(char* key){
  int32_t i = 0;

  for (i = 0; i < (int)ONBOARD_PARAM_COUNT; i++){
    if (strcmp(key, global_data[i].name) == 0)
      return i;
  }
  return -1;
}

/**
 * ¬озвращает указатель пр€мо на значение.
 *
 * ƒанный функционал вынесен в отдельную функцию на тот случай, если
 * приложению понадобитс€ знать другие пол€ структуры
 */
float *ValueSearch(char *str){
  int32_t i = -1;

  i = _key_index_search(str);
  if (i == -1){
    chDbgPanic("key not found");
    return NULL;
  }
  else
    return &(global_data[i].value);
}



void ParametersInit(void){

  chMBInit(&param_confirm_mb, param_confirm_mb_buf, (sizeof(param_confirm_mb_buf)/sizeof(msg_t)));

  /* check hardcoded values */
  uint32_t i = 0;
  for (i = 0; i < ONBOARD_PARAM_COUNT; i++){
    if (sizeof (*(global_data[i].name)) > ONBOARD_PARAM_NAME_LENGTH)
      chDbgPanic("name too long");
    if ((global_data[i].value < global_data[i].min) ||
        (global_data[i].value > global_data[i].max))
      chDbgPanic("value out of bounds");
  }

  /* check allowed size in EEPROM */
  uint32_t len = PARAM_ID_SIZE;
  len += sizeof(global_data[0].value);
  if (ONBOARD_PARAM_COUNT * len > EEPROM_SETTINGS_SIZE)
    chDbgPanic("not enough space in EEPROM settings slice");

  /* read data from eeprom to memory mapped structure */
  load_params_from_eeprom();

  chThdCreateStatic(ParametersThreadWA,
          sizeof(ParametersThreadWA),
          LINK_THREADS_PRIO + 1,
          ParametersThread,
          NULL);
}

