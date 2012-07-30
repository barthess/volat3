#include "ch.h"
#include "hal.h"

#include "discrete.h"
#include "sensors.h"
#include "param.h"
#include "utils.h"
#include "main.h"

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
/* указатели на коэффициенты */
static uint32_t *rel_z_0,   *rel_z_32;
static uint32_t *rel_vcc_0, *rel_vcc_32;
static uint32_t *rel_gnd_0, *rel_gnd_32;
static uint32_t *brd_revision;
static uint32_t *rel_test;

/*
 ******************************************************************************
 * PROTOTYPES
 ******************************************************************************
 */

/*
 ******************************************************************************
 ******************************************************************************
 * LOCAL FUNCTIONS
 ******************************************************************************
 ******************************************************************************
 */
/**
 * perform inversion of needed bits
 *
 * Как работает эта магия:
 * Вход датчика может висеть на +24, на корпусе, в воздухе.
 * Только одно из трех состояний считается нормой, в противном случае - авария.
 * Состояния входов читаются дважды: с включенным z_check и с выключенным.
 * В результате получется следующая таблица истинности:
 *
 * +24   GND   hi_z
 *  1     0     1      z_on
 *  1     0     0      z_off
 *
 * Далее к полученным данным применяются логические операции и откусываются
 * масками ненужные биты.
 * Функция принимает 3 разные маски, 0 в маске означает норму, 1 - аварию, например:
 * 1100 z
 * 1011 gnd
 * 0111 vcc
 * т.е. в одном столбце может быть один и только один ноль.
 *
 * Возвращает аварийную битовую маску (1 - авария, 0 - норма)
 */
uint32_t _rel_normalize32(uint32_t z_on, uint32_t z_off,
                          uint32_t z_mask, uint32_t gnd_mask, uint32_t vcc_mask){

  uint32_t vcc, gnd, z;

  switch (*rel_test){
  case 0:
    /* проверяем кошерность масок */
    if (((z_mask ^ gnd_mask ^ vcc_mask) != 0) || ((z_mask | gnd_mask | vcc_mask) == 0))
      return -1;
    vcc = (z_on & z_off)    & (~vcc_mask);
    gnd = (~(z_on | z_off)) & (~gnd_mask);
    z   = (z_on ^ z_off)    & (~z_mask);
    break;

  case 1:
    vcc = (z_on & z_off)    & (0xFFFFFFFF);
    gnd = (~(z_on | z_off)) & (0xFFFFFFFF);
    z   = (z_on ^ z_off)    & (0);
    break;

  case 2:
    vcc = (z_on & z_off)    & (0);
    gnd = (~(z_on | z_off)) & (0xFFFFFFFF);
    z   = (z_on ^ z_off)    & (0xFFFFFFFF);
    break;

  case 3:
    vcc = (z_on & z_off)    & (0xFFFFFFFF);
    gnd = (~(z_on | z_off)) & (0);
    z   = (z_on ^ z_off)    & (0xFFFFFFFF);
    break;

  default:
    vcc = 0xFFFFFFFF;
    gnd = 0xFFFFFFFF;
    z   = 0xFFFFFFFF;
    break;
  }

  return vcc | gnd | z;
}

/**
 * Из-за ошибок в разводке платы необходимое перевенуть кажду тетраду результата.
 */
uint32_t board_workaround(uint32_t v){
  uint32_t result = 0;

  if (*brd_revision == 1){
    const uint32_t m1 = 0b00010001000100010001000100010001;
    const uint32_t m2 = 0b00100010001000100010001000100010;
    const uint32_t m3 = 0b01000100010001000100010001000100;
    const uint32_t m4 = 0b10001000100010001000100010001000;

    result  = (v & m4) >> 3;
    result |= (v & m3) >> 1;
    result |= (v & m2) << 1;
    result |= (v & m1) << 3;

    return result;
  }
  else
    return v;
}


/*
 ******************************************************************************
 * EXPORTED FUNCTIONS
 ******************************************************************************
 */

/**
 * Вычисляет аварийную ситуацию и выставляет аварийные биты.
 */
void rel_normalize(uint32_t *z_on, uint32_t *z_off, uint32_t *out){
  out[0] = _rel_normalize32(board_workaround(z_on[0]), board_workaround(z_off[0]),
                            *rel_z_0,  *rel_gnd_0,  *rel_vcc_0);
  out[1] = _rel_normalize32(board_workaround(z_on[1]), board_workaround(z_off[1]),
                            *rel_z_32, *rel_gnd_32, *rel_vcc_32);
}

/**
 *
 */
void DiscreteInitLocal(void){

  brd_revision  = ValueSearch("BRD_revision");

  rel_z_0       = ValueSearch("REL_Z_0");
  rel_z_32      = ValueSearch("REL_Z_32");
  rel_vcc_0     = ValueSearch("REL_VCC_0");
  rel_vcc_32    = ValueSearch("REL_VCC_32");
  rel_gnd_0     = ValueSearch("REL_GND_0");
  rel_gnd_32    = ValueSearch("REL_GND_32");

  rel_test      = ValueSearch("REL_test");
}


