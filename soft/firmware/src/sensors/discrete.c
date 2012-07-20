#include "ch.h"
#include "hal.h"

#include "discrete.h"
#include "sensors.h"
#include "param.h"
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

/*
 ******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************
 */
/* указатели на коэффициенты */
static float *rel_z_0,   *rel_z_32;
static float *rel_vcc_0, *rel_vcc_32;
static float *rel_gnd_0, *rel_gnd_32;

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

/*
 ******************************************************************************
 * EXPORTED FUNCTIONS
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
uint32_t _rel_normalize32(uint8_t *rxbuf_z_on, uint8_t *rxbuf_z_off,
                          uint32_t z_mask, uint32_t gnd_mask, uint32_t vcc_mask){

  /* проверяем кошерность масок */
  if (((z_mask ^ gnd_mask ^ vcc_mask) != 0) || ((z_mask | gnd_mask | vcc_mask) == 0))
    return -1;

  uint32_t on  = pack8to32(rxbuf_z_on);
  uint32_t off = pack8to32(rxbuf_z_off);

  uint32_t vcc = (on & off)    & (~vcc_mask);
  uint32_t gnd = (~(on | off)) & (~gnd_mask);
  uint32_t z   = (on ^ off)    & (~z_mask);

  return vcc | gnd | z;
}
uint64_t _rel_normalize64(uint8_t *rxbuf_z_on, uint8_t *rxbuf_z_off,
                          uint64_t z_mask, uint64_t gnd_mask, uint64_t vcc_mask){

  /* проверяем кошерность масок */
  if (((z_mask ^ gnd_mask ^ vcc_mask) != 0) || ((z_mask | gnd_mask | vcc_mask) == 0))
    return -1;

  uint64_t on  = pack8to64(rxbuf_z_on);
  uint64_t off = pack8to64(rxbuf_z_off);

  uint64_t vcc = (on & off)    & (~vcc_mask);
  uint64_t gnd = (~(on | off)) & (~gnd_mask);
  uint64_t z   = (on ^ off)    & (~z_mask);

  return vcc | gnd | z;
}

/**
 *
 */
void rel_normalize(uint8_t *rxbuf_z_on, uint8_t *rxbuf_z_off, uint32_t *out){
  out[0] = _rel_normalize32(rxbuf_z_on, rxbuf_z_off, *rel_z_0, *rel_gnd_0, *rel_vcc_0);
  out[1] = _rel_normalize32(&rxbuf_z_on[4], &rxbuf_z_off[4], *rel_z_32, *rel_gnd_32, *rel_vcc_32);
}

/**
 *
 */
void DiscreteInitLocal(void){
  rel_z_0     = ValueSearch("REL_Z_0");
  rel_z_32    = ValueSearch("REL_Z_32");
  rel_vcc_0   = ValueSearch("REL_VCC_0");
  rel_vcc_32  = ValueSearch("REL_VCC_32");
  rel_gnd_0   = ValueSearch("REL_GND_0");
  rel_gnd_32  = ValueSearch("REL_GND_32");
}


