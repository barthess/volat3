#include "ch.h"
#include "hal.h"
#include "mavlink.h"

#include "main.h"
#include "message.h"
#include "sanity.h"
#include "timekeeper.h"

/*
 ******************************************************************************
 * EXTERNS
 ******************************************************************************
 */
extern EventSource event_mavlink_heartbeat_bnap;
extern EventSource event_mavlink_sys_status;

extern mavlink_system_t       mavlink_system_struct;
extern mavlink_heartbeat_t    mavlink_heartbeat_bnap_struct;
extern mavlink_sys_status_t   mavlink_sys_status_struct;

/*
 ******************************************************************************
 * DEFINES
 ******************************************************************************
 */
#define HEARTBEAT_PERIOD    MS2ST(1000)
#define SYS_STATUS_PERIOD   MS2ST(100)

/*
 ******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************
 */
/* указатель на Idle поток. Оттуда мы будем брать данные для расчета загрузки проца */
static Thread *IdleThread_p = NULL;

/* переменные для оценки загруженности процессора */
static uint32_t last_sys_ticks = 0;
static uint32_t last_idle_ticks = 0;

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
static WORKING_AREA(HeartbeatThreadWA, 48);
static msg_t HeartbeatThread(void *arg) {
  chRegSetThreadName("Heartbeat");
  (void)arg;

  mavlink_heartbeat_bnap_struct.autopilot = MAV_AUTOPILOT_GENERIC;
  mavlink_heartbeat_bnap_struct.custom_mode = 0;

  systime_t t = chTimeNow();

  while (TRUE) {
    t += HEARTBEAT_PERIOD;

    mavlink_heartbeat_bnap_struct.type           = mavlink_system_struct.type;
    mavlink_heartbeat_bnap_struct.base_mode      = mavlink_system_struct.mode;
    mavlink_heartbeat_bnap_struct.system_status  = mavlink_system_struct.state;

    chEvtBroadcastFlags(&event_mavlink_heartbeat_bnap, EVMSK_MAVLINK_HEARTBEAT_MPIOVD);
    chThdSleepUntil(t);
  }
  return 0;
}

/**
 *
 */
static WORKING_AREA(SysStatusThreadWA, 48);
static msg_t SysStatusThread(void *arg) {
  chRegSetThreadName("SysStatus");
  (void)arg;

  systime_t t = chTimeNow();

  while (TRUE) {
    t += SYS_STATUS_PERIOD;

    mavlink_sys_status_struct.load = get_cpu_load();
    chEvtBroadcastFlags(&event_mavlink_sys_status, EVMSK_MAVLINK_SYS_STATUS);
    chThdSleepUntil(t);
  }
  return 0;
}

/*
 *******************************************************************************
 * EXPORTED FUNCTIONS
 *******************************************************************************
 */
/**
 *
 */
void SanityControlInit(void){
  IdleThread_p = chSysGetIdleThread();

  /**/
  chThdCreateStatic(HeartbeatThreadWA,
          sizeof(HeartbeatThreadWA),
          NORMALPRIO,
          HeartbeatThread,
          NULL);
  chThdCreateStatic(SysStatusThreadWA,
          sizeof(SysStatusThreadWA),
          NORMALPRIO,
          SysStatusThread,
          NULL);
}

/**
 * Рассчитывает загрузку проца.
 * Возвращает десятые доли процента.
 */
uint16_t get_cpu_load(void){

  uint32_t i, s; /* "мгновенные" значения количества тиков idle, system */

  /* получаем мгновенное значение счетчика из Idle */
  if (chThdGetTicks(IdleThread_p) >= last_idle_ticks)
    i = chThdGetTicks(IdleThread_p) - last_idle_ticks;
  else /* overflow */
    i = chThdGetTicks(IdleThread_p) + (0xFFFFFFFF - last_idle_ticks);

  last_idle_ticks = chThdGetTicks(IdleThread_p);

  /* получаем мгновенное значение счетчика из системы */
  s = GetTimeInterval(&last_sys_ticks);

  return ((s - i) * 1000) / s;
}

