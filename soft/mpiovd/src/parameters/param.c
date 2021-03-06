#include <math.h>
#include <string.h>

#include "ch.h"
#include "hal.h"

#include "message.h"
#include "param.h"
#include "persistant.h"
#include "main.h"
#include "eeprom.h"
#include "eeprom_conf.h"
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

EepromFileStream EepromSettingsFile;

extern Mailbox mavlink_param_set_mb;
extern Mailbox tolink_mb;
extern mavlink_system_t mavlink_system_struct;

int32_t OnboardParamCount = 0;

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
 *
 */
GlobalParam_t global_data[] = {
  /*  key             min                  val                    max                  type                   */
  /*----------------------------------------------------------------------------------------------------------*/
  {"SYS_ID",          {.u32 = 1},          {.u32 = 20},           {.u32 = 255},        MAVLINK_TYPE_UINT32_T},
  /* 16 denotes ground rover */
  {"SYS_mavtype",     {.u32 = 0},          {.u32 = 10},           {.u32 = 16},         MAVLINK_TYPE_UINT32_T},
  /* enable shell instead of telemetry */
  {"SH_enable",       {.u32 = 0},          {.u32 = 0},            {.u32 = 1},          MAVLINK_TYPE_UINT32_T},
  // длины фильтров для каналов АЦП
  {"AN_ch00_flen",    {.u32 = 0},          {.u32 = 8},            {.u32 = 30},         MAVLINK_TYPE_UINT32_T},
  {"AN_ch01_flen",    {.u32 = 0},          {.u32 = 8},            {.u32 = 30},         MAVLINK_TYPE_UINT32_T},
  {"AN_ch02_flen",    {.u32 = 0},          {.u32 = 8},            {.u32 = 30},         MAVLINK_TYPE_UINT32_T},
  {"AN_ch03_flen",    {.u32 = 0},          {.u32 = 8},            {.u32 = 30},         MAVLINK_TYPE_UINT32_T},
  {"AN_ch04_flen",    {.u32 = 0},          {.u32 = 8},            {.u32 = 30},         MAVLINK_TYPE_UINT32_T},
  {"AN_ch05_flen",    {.u32 = 0},          {.u32 = 8},            {.u32 = 30},         MAVLINK_TYPE_UINT32_T},
  {"AN_ch06_flen",    {.u32 = 0},          {.u32 = 8},            {.u32 = 30},         MAVLINK_TYPE_UINT32_T},
  {"AN_ch07_flen",    {.u32 = 0},          {.u32 = 8},            {.u32 = 30},         MAVLINK_TYPE_UINT32_T},
  {"AN_ch08_flen",    {.u32 = 0},          {.u32 = 8},            {.u32 = 30},         MAVLINK_TYPE_UINT32_T},
  {"AN_ch09_flen",    {.u32 = 0},          {.u32 = 8},            {.u32 = 30},         MAVLINK_TYPE_UINT32_T},
  {"AN_ch10_flen",    {.u32 = 0},          {.u32 = 8},            {.u32 = 30},         MAVLINK_TYPE_UINT32_T},
  {"AN_ch11_flen",    {.u32 = 0},          {.u32 = 8},            {.u32 = 30},         MAVLINK_TYPE_UINT32_T},
  {"AN_ch12_flen",    {.u32 = 0},          {.u32 = 8},            {.u32 = 30},         MAVLINK_TYPE_UINT32_T},
  {"AN_ch13_flen",    {.u32 = 0},          {.u32 = 8},            {.u32 = 30},         MAVLINK_TYPE_UINT32_T},
  {"AN_ch14_flen",    {.u32 = 0},          {.u32 = 8},            {.u32 = 30},         MAVLINK_TYPE_UINT32_T},
  {"AN_ch15_flen",    {.u32 = 0},          {.u32 = 8},            {.u32 = 30},         MAVLINK_TYPE_UINT32_T},

  /****** разные временнЫе интервалы в mS */
  // пакеты с телеметрией
  {"T_tlm",           {.u32 = SEND_OFF},    {.u32 = 33},          {.u32 = SEND_MAX},    MAVLINK_TYPE_UINT32_T},
  // интервал между сохранениями пробега и моточасов в eeprom
  {"T_save_trip",     {.u32 = 2000},        {.u32 = 10000},       {.u32 = 60000},       MAVLINK_TYPE_UINT32_T},
  // пакеты heartbeat
  {"T_heartbeat",     {.u32 = 100},         {.u32 = 1000},        {.u32 = SEND_MAX},    MAVLINK_TYPE_UINT32_T},
  {"T_sys_status",    {.u32 = SEND_OFF},    {.u32 = 100},         {.u32 = SEND_MAX},    MAVLINK_TYPE_UINT32_T},
  {"T_reserved2",     {.u32 = SEND_OFF},    {.u32 = 100},         {.u32 = SEND_MAX},    MAVLINK_TYPE_UINT32_T},
  {"T_reserved3",     {.u32 = SEND_OFF},    {.u32 = 100},         {.u32 = SEND_MAX},    MAVLINK_TYPE_UINT32_T},
  {"T_reserved4",     {.u32 = SEND_OFF},    {.u32 = 100},         {.u32 = SEND_MAX},    MAVLINK_TYPE_UINT32_T},
  {"T_reserved5",     {.u32 = SEND_OFF},    {.u32 = 100},         {.u32 = SEND_MAX},    MAVLINK_TYPE_UINT32_T},
  {"T_reserved6",     {.u32 = SEND_OFF},    {.u32 = 100},         {.u32 = SEND_MAX},    MAVLINK_TYPE_UINT32_T},

  /* masks for relay sensors */
  {"REL_Z_0",         {.u32 = 0},           {.u32 = 0},           {.u32 = 0xFFFFFFFF},  MAVLINK_TYPE_UINT32_T},
  {"REL_Z_32",        {.u32 = 0},           {.u32 = 0},           {.u32 = 0xFFFFFFFF},  MAVLINK_TYPE_UINT32_T},
  {"REL_VCC_0",       {.u32 = 0},           {.u32 = 0xFFFFFFFF},  {.u32 = 0xFFFFFFFF},  MAVLINK_TYPE_UINT32_T},
  {"REL_VCC_32",      {.u32 = 0},           {.u32 = 0xFFFFFFFF},  {.u32 = 0xFFFFFFFF},  MAVLINK_TYPE_UINT32_T},
  {"REL_GND_0",       {.u32 = 0},           {.u32 = 0xFFFFFFFF},  {.u32 = 0xFFFFFFFF},  MAVLINK_TYPE_UINT32_T},
  {"REL_GND_32",      {.u32 = 0},           {.u32 = 0xFFFFFFFF},  {.u32 = 0xFFFFFFFF},  MAVLINK_TYPE_UINT32_T},
  // 0 - normal mode, 1 - Z, 2 - VCC, 3 - GND
  {"REL_test",        {.u32 = 0},           {.u32 = 0},           {.u32 = 3},           MAVLINK_TYPE_UINT32_T},
  {"REL_stm32_fix",   {.u32 = 0},           {.u32 = 0},           {.u32 = 1},           MAVLINK_TYPE_UINT32_T},
  {"REL_reserved2",   {.u32 = 0},           {.u32 = 0},           {.u32 = 3},           MAVLINK_TYPE_UINT32_T},
  {"REL_reserved3",   {.u32 = 0},           {.u32 = 0},           {.u32 = 3},           MAVLINK_TYPE_UINT32_T},

  /* ревизия печатной платы для включения программных костылей
   * 1 == перевернуть тетрады битов чтобы исправить неправильную разводку оптронов */
  {"BRD_revision",    {.u32 = 0},           {.u32 = 1},           {.u32 = 0xFF},        MAVLINK_TYPE_UINT32_T},

  /* fake field with 14 symbols name */
  {"fake_14_bytes_",  {.u32 = 1},           {.u32 = 3},	          {.u32 = 1224},        MAVLINK_TYPE_UINT32_T},
};

/*
 ******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************
 */
static Mailbox param_confirm_mb;
static msg_t param_confirm_mb_buf[1];

static uint8_t eeprom_buf[EEPROM_TX_DEPTH];

static const I2CEepromFileConfig eeprom_settings_cfg = {
  &EEPROM_I2CD,
  EEPROM_SETTINGS_START,
  EEPROM_SETTINGS_END,
  EEPROM_SIZE,
  EEPROM_PAGE_SIZE,
  EEPROM_I2C_ADDR,
  MS2ST(EEPROM_WRITE_TIME_MS),
  eeprom_buf,
};

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
  floatint v;

  index = _key_index_search(paramset->param_id);
  v.f32 = paramset->param_value;

  if (index >= 0){
    switch (paramset->param_type){
    case MAVLINK_TYPE_FLOAT:
      // Only write and emit changes if there is actually a difference
      // AND only write if new value is NOT "not-a-number"
      // AND is NOT infinity
      if (isnan(v.f32) || isinf(v.f32))
        return PARAM_FAILED;
      if (global_data[index].value.f32 == v.f32)
        return PARAM_FAILED;
      break;

    case MAVLINK_TYPE_INT32_T:
      if (global_data[index].value.i32 == v.i32)
        return PARAM_FAILED;
      break;

    case MAVLINK_TYPE_UINT32_T:
      if (global_data[index].value.u32 == v.u32)
        return PARAM_FAILED;
      break;

    default:
      chDbgPanic("Usupported variable type");
      break;
    }

    /* If value fall out of min..max bound than just set nearest allowable value */
    return set_global_param(&(paramset->param_value), &global_data[index]);
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

  if ((index >= 0) && (index <= (int)OnboardParamCount)){
    /* fill all fields */
    param_value_struct->param_value = global_data[index].value.f32;
    param_value_struct->param_count = OnboardParamCount;
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
  int32_t i = 0;
  for (i = 0; i < OnboardParamCount; i++){
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
      set_parameter(set);
      send_value(&param_value_mail, &param_value_struct, set->param_id, 0);
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
int32_t _key_index_search(const char* key){
  int32_t i = 0;

  for (i = 0; i < OnboardParamCount; i++){
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
void *ValueSearch(const char *str){
  int32_t i = -1;

  i = _key_index_search(str);
  if (i == -1){
    chDbgPanic("key not found");
    return NULL;
  }
  else
    return &(global_data[i].value);
}

/**
 * @brief   Global boundary checker.
 *
 * @param[in] value   input value that must be checked and saved in
 * @param[in] param   appropriate parameter in array
 *
 * @return            operation status.
 */
bool_t set_global_param(void *value,  GlobalParam_t *param){
  switch(param->param_type){
  case MAVLINK_TYPE_FLOAT:
    return _float_setval(value, param);
    break;
  case MAVLINK_TYPE_UINT32_T:
    return _uint_setval(value, param);
    break;
  case MAVLINK_TYPE_INT32_T:
    return _int_setval(value, param);
    break;
  default:
    chDbgPanic("Unsupported variable type");
    break;
  }
  return PARAM_FAILED;
}

/**
 *
 */
void ParametersInit(void){
  uint8_t eeprombuf[PARAM_ID_SIZE + sizeof(global_data[0].value)];
  uint32_t status;

  OnboardParamCount = (sizeof(global_data) / sizeof(GlobalParam_t));

  chMBInit(&param_confirm_mb, param_confirm_mb_buf, (sizeof(param_confirm_mb_buf)/sizeof(msg_t)));

  /* check hardcoded param names */
  int32_t i = 0;
  for (i = 0; i < OnboardParamCount; i++){
    if (sizeof (*(global_data[i].name)) > ONBOARD_PARAM_NAME_LENGTH)
      chDbgPanic("name too long");
  }

  /* check allowed size in EEPROM */
  uint32_t len = PARAM_ID_SIZE;
  len += sizeof(global_data[0].value);
  if (OnboardParamCount * len > EEPROM_SETTINGS_SIZE)
    chDbgPanic("not enough space in EEPROM settings slice");

  /* open file and check presence of settings in it */
  EepromFileOpen(&EepromSettingsFile, &eeprom_settings_cfg);

  /* check first param in EEPROM */
  chFileStreamSeek(&EepromSettingsFile, 0);
  status = chFileStreamRead(&EepromSettingsFile, eeprombuf, sizeof(eeprombuf));
  if (status != sizeof(eeprombuf))
    chDbgPanic("read failed");

  if (strcmp((char *)eeprombuf, global_data[0].name) != 0)
    save_params_to_eeprom();    /* store values as defaults */
  else
    load_params_from_eeprom();  /* load previously stored */

  /**/
  chThdCreateStatic(ParametersThreadWA,
          sizeof(ParametersThreadWA),
          LINK_THREADS_PRIO + 1,
          ParametersThread,
          NULL);
}

