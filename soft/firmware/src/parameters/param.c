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
 * Float boundary checker.
 */
static bool_t _float_setval(void *value,  GlobalParam_t *param){
  float initial_value = *(float*)value;
  float v = initial_value;

  putinrange(v, param->min.f32, param->max.f32);
  param->value.f32 = v;

  if (v == initial_value)
    return PARAM_SUCCESS;
  else
    return PARAM_FAILED;
}

/**
 * Int32 boundary checker.
 */
static bool_t _int_setval(void *value,  GlobalParam_t *param){
  int32_t initial_value = *(int32_t*)value;
  int32_t v = initial_value;

  putinrange(v, param->min.i32, param->max.i32);
  param->value.i32 = v;

  if (v == initial_value)
    return PARAM_SUCCESS;
  else
    return PARAM_FAILED;
}

/**
 * Uint32 boundary checker.
 */
static bool_t _uint_setval(void *value,  GlobalParam_t *param){
  uint32_t initial_value = *(uint32_t*)value;
  uint32_t v = initial_value;

  putinrange(v, param->min.u32, param->max.u32);
  param->value.u32 = v;

  if (v == initial_value)
    return PARAM_SUCCESS;
  else
    return PARAM_FAILED;
}

/**
 * Global boundary checker.
 */
static bool_t setval(void *value,  GlobalParam_t *param){
  uint32_t initial_value = *(uint32_t*)value;
  uint32_t v = initial_value;

  putinrange(v, param->min.u32, param->max.u32);
  param->value.u32 = v;

  if (v == initial_value)
    return PARAM_SUCCESS;
  else
    return PARAM_FAILED;
}




/**
 *
 */
GlobalParam_t global_data[] = {
  /*  key             min         val         max         type                    checker_fucntion   */
  /*--------------------------------------------------------------------------------------------------*/
  {"SYS_ID",          {1UL},          {20UL},     {255UL},        MAVLINK_TYPE_UINT32_T,  _uint_setval},
  /* 16 denotes ground rover */
  {"SYS_mavtype",     0,          1,          16,         MAVLINK_TYPE_UINT32_T,  _uint_setval},

  //Коэффициенты полиномов для пересчета показаний с датчиков, и длины фильтров
  {"AN_ch1_c1",       -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   _int_setval},
  {"AN_ch1_c2",       -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   _int_setval},
  {"AN_ch1_c3",       -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   _int_setval},
  {"AN_ch1_flen",            0,    4,         64,         MAVLINK_TYPE_INT32_T,   _int_setval},

  {"AN_ch2_c1",       -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   _int_setval},
  {"AN_ch2_c2",       -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   _int_setval},
  {"AN_ch2_c3",       -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   _int_setval},
  {"AN_ch2_flen",            0,    4,         64,         MAVLINK_TYPE_INT32_T,   _int_setval},

  {"AN_ch3_c1",       -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   _int_setval},
  {"AN_ch3_c2",       -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   _int_setval},
  {"AN_ch3_c3",       -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   _int_setval},
  {"AN_ch3_flen",            0,    4,         64,         MAVLINK_TYPE_INT32_T,   _int_setval},

  {"AN_ch4_c1",       -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   _int_setval},
  {"AN_ch4_c2",       -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   _int_setval},
  {"AN_ch4_c3",       -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   _int_setval},
  {"AN_ch4_flen",            0,    4,         64,         MAVLINK_TYPE_INT32_T,   _int_setval},

  {"AN_ch5_c1",       -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   _int_setval},
  {"AN_ch5_c2",       -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   _int_setval},
  {"AN_ch5_c3",       -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   _int_setval},
  {"AN_ch5_flen",            0,    4,         64,         MAVLINK_TYPE_INT32_T,   _int_setval},

  {"AN_ch6_c1",       -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   _int_setval},
  {"AN_ch6_c2",       -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   _int_setval},
  {"AN_ch6_c3",       -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   _int_setval},
  {"AN_ch6_flen",            0,    4,         64,         MAVLINK_TYPE_INT32_T,   _int_setval},

  {"AN_ch7_c1",       -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   _int_setval},
  {"AN_ch7_c2",       -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   _int_setval},
  {"AN_ch7_c3",       -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   _int_setval},
  {"AN_ch7_flen",            0,    4,         64,         MAVLINK_TYPE_INT32_T,   _int_setval},

  {"AN_ch8_c1",       -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   _int_setval},
  {"AN_ch8_c2",       -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   _int_setval},
  {"AN_ch8_c3",       -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   _int_setval},
  {"AN_ch8_flen",            0,    4,         64,         MAVLINK_TYPE_INT32_T,   _int_setval},

  {"AN_ch9_c1",       -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   _int_setval},
  {"AN_ch9_c2",       -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   _int_setval},
  {"AN_ch9_c3",       -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   _int_setval},
  {"AN_ch9_flen",            0,    4,         64,         MAVLINK_TYPE_INT32_T,   _int_setval},

  {"AN_ch10_c1",      -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   _int_setval},
  {"AN_ch10_c2",      -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   _int_setval},
  {"AN_ch10_c3",      -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   _int_setval},
  {"AN_ch10_flen",           0,    4,         64,         MAVLINK_TYPE_INT32_T,   _int_setval},

  {"AN_ch11_c1",      -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   _int_setval},
  {"AN_ch11_c2",      -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   _int_setval},
  {"AN_ch11_c3",      -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   _int_setval},
  {"AN_ch11_flen",           0,    4,         64,         MAVLINK_TYPE_INT32_T,   _int_setval},

  {"AN_ch12_c1",      -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   _int_setval},
  {"AN_ch12_c2",      -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   _int_setval},
  {"AN_ch12_c3",      -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   _int_setval},
  {"AN_ch12_flen",           0,    4,         64,         MAVLINK_TYPE_INT32_T,   _int_setval},

  {"AN_ch13_c1",      -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   _int_setval},
  {"AN_ch13_c2",      -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   _int_setval},
  {"AN_ch13_c3",      -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   _int_setval},
  {"AN_ch13_flen",           0,    4,         64,         MAVLINK_TYPE_INT32_T,   _int_setval},

  {"AN_ch14_c1",      -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   _int_setval},
  {"AN_ch14_c2",      -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   _int_setval},
  {"AN_ch14_c3",      -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   _int_setval},
  {"AN_ch14_flen",           0,    4,         64,         MAVLINK_TYPE_INT32_T,   _int_setval},

  {"AN_ch15_c1",      -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   _int_setval},
  {"AN_ch15_c2",      -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   _int_setval},
  {"AN_ch15_c3",      -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   _int_setval},
  {"AN_ch15_flen",           0,    4,         64,         MAVLINK_TYPE_INT32_T,   _int_setval},

  {"AN_ch16_c1",      -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   _int_setval},
  {"AN_ch16_c2",      -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   _int_setval},
  {"AN_ch16_c3",      -2000000,   -9,         2000000,    MAVLINK_TYPE_INT32_T,   _int_setval},
  {"AN_ch16_flen",           0,    4,         64,         MAVLINK_TYPE_INT32_T,   _int_setval},

  /****** разные временнЫе интервалы в mS */
  // пакеты с телеметрией
  {"T_tlm",           SEND_OFF,   100,        SEND_MAX,   MAVLINK_TYPE_UINT32_T,  _uint_setval},
  // интервал между сохранениями пробега и моточасов в eeprom
  {"T_save_trip",     SEND_OFF,   100,        SEND_MAX,   MAVLINK_TYPE_UINT32_T,  _uint_setval},
  // пакеты heartbeat
  {"T_heartbeat",     SEND_OFF,   100,        SEND_MAX,   MAVLINK_TYPE_UINT32_T,  _uint_setval},
  {"T_reserved1",     SEND_OFF,   100,        SEND_MAX,   MAVLINK_TYPE_UINT32_T,  _uint_setval},
  {"T_reserved2",     SEND_OFF,   100,        SEND_MAX,   MAVLINK_TYPE_UINT32_T,  _uint_setval},
  {"T_reserved3",     SEND_OFF,   100,        SEND_MAX,   MAVLINK_TYPE_UINT32_T,  _uint_setval},
  {"T_reserved4",     SEND_OFF,   100,        SEND_MAX,   MAVLINK_TYPE_UINT32_T,  _uint_setval},
  {"T_reserved5",     SEND_OFF,   100,        SEND_MAX,   MAVLINK_TYPE_UINT32_T,  _uint_setval},
  {"T_reserved6",     SEND_OFF,   100,        SEND_MAX,   MAVLINK_TYPE_UINT32_T,  _uint_setval},

  /* masks for relay sensors */
  {"REL_Z_0",         0,          0,          0xFFFFFFFF, MAVLINK_TYPE_UINT32_T,  _uint_setval},
  {"REL_Z_32",        0,          0,          0xFFFFFFFF, MAVLINK_TYPE_UINT32_T,  _uint_setval},
  {"REL_VCC_0",       0,          0xFFFFFFFF, 0xFFFFFFFF, MAVLINK_TYPE_UINT32_T,  _uint_setval},
  {"REL_VCC_32",      0,          0xFFFFFFFF, 0xFFFFFFFF, MAVLINK_TYPE_UINT32_T,  _uint_setval},
  {"REL_GND_0",       0,          0xFFFFFFFF, 0xFFFFFFFF, MAVLINK_TYPE_UINT32_T,  _uint_setval},
  {"REL_GND_32",      0,          0xFFFFFFFF, 0xFFFFFFFF, MAVLINK_TYPE_UINT32_T,  _uint_setval},

  /* ревизия печатной платы для включения программных костылей */
  {"BRD_revision",    0,          1,          0xFFFFFFFF, MAVLINK_TYPE_UINT32_T,  _uint_setval},

  /* fake field with 14 symbols name */
  {"fake_14_bytes_",  1,          {1.3F},     1224,       MAVLINK_TYPE_FLOAT,     _float_setval},
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
  union{
    float    f32;
    uint32_t u32;
    int32_t  i32;
  }v;

  index = _key_index_search(paramset->param_id);
  v.f32 = paramset->param_value;

  if (index >= 0){
    // Only write and emit changes if there is actually a difference
    // AND only write if new value is NOT "not-a-number"
    // AND is NOT infinity
    if (paramset->param_type == MAVLINK_TYPE_FLOAT){
      if (isnan(v.f32) || isinf(v.f32))
        return PARAM_FAILED;
      if (global_data[index].value.f32 == v.f32)
        return PARAM_FAILED;
    }
    else if (paramset->param_type == MAVLINK_TYPE_INT32_T){
      if (global_data[index].value.i32 == v.i32)
        return PARAM_FAILED;
    }
    else if (paramset->param_type == MAVLINK_TYPE_UINT32_T){
      if (global_data[index].value.u32 == v.u32)
        return PARAM_FAILED;
    }

    /* If value fall out of min..max bound than just set nearest allowable value */
    return global_data[index].setval(&(paramset->param_value), &global_data[index]);
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
    param_value_struct->param_value = global_data[index].value.f32;
    param_value_struct->param_count = ONBOARD_PARAM_COUNT;
    param_value_struct->param_index = index;
    param_value_struct->param_type  = global_data[index].param_type;
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
 * Поток принимающий сообщения с параметрами и отправляющий параметры по запросу.
 */
static WORKING_AREA(ParametersThreadWA, 512);
static msg_t ParametersThread(void *arg){
  chRegSetThreadName("Parameters");
  (void)arg;

  /* переменные для отправки установленных параметров */
  mavlink_param_value_t param_value_struct;
  Mail param_value_mail = {NULL, MAVLINK_MSG_ID_PARAM_VALUE, &param_confirm_mb};

  /* переменные для приема параметров */
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
     * вычитать и выслать в ответ этот параметр в качестве подтверждения
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
 * Возвращает указатель прямо на значение.
 *
 * Данный функционал вынесен в отдельную функцию на тот случай, если
 * приложению понадобится знать другие поля структуры
 */
void *ValueSearch(char *str){
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

