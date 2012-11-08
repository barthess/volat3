#include "ch.h"
#include "hal.h"
#include "mavlink.h"

#include "main.h"
#include "message.h"
#include "sanity.h"
#include "timekeeping.h"

/*
 ******************************************************************************
 * EXTERNS
 ******************************************************************************
 */
extern GlobalFlags_t GlobalFlags;
extern Mailbox tolinkdm_mb;

extern mavlink_system_t       mavlink_system_struct;
extern mavlink_heartbeat_t    mavlink_heartbeat_struct;
extern mavlink_sys_status_t   mavlink_sys_status_struct;

/*
 ******************************************************************************
 * DEFINES
 ******************************************************************************
 */
#define HEART_BEAT_PERIOD   MS2ST(1000)

/* how much time led is ON during flash */
#define BLUE_LED_ON_TIME    MS2ST(50)
#define BLUE_LED_OFF_TIME   MS2ST(100)

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

static BinarySemaphore blink_sem;

/*
 *******************************************************************************
 *******************************************************************************
 * LOCAL FUNCTIONS
 *******************************************************************************
 *******************************************************************************
 */

/**
 * посылает heartbeat пакеты и моргает светодиодиком
 */
static WORKING_AREA(SanityControlThreadWA, 128);
static msg_t SanityControlThread(void *arg) {
  chRegSetThreadName("Sanity");
  (void)arg;

  BinarySemaphore sanity_sem;     /* to sync with tlm sender */
  chBSemInit(&sanity_sem, FALSE); /* semaphore is not taken */
  Mail heartbeat_mail = {NULL, MAVLINK_MSG_ID_HEARTBEAT, &sanity_sem};
  mavlink_heartbeat_struct.autopilot = MAV_AUTOPILOT_GENERIC;
  mavlink_heartbeat_struct.custom_mode = 0;

  systime_t t = chTimeNow();

  while (TRUE) {
    t += HEART_BEAT_PERIOD;

    /* fill data fields and send struct to message box */
    chBSemWaitTimeout(&sanity_sem, MS2ST(1));
    if (GlobalFlags.link_cc_ready){
      mavlink_heartbeat_struct.type           = mavlink_system_struct.type;
      mavlink_heartbeat_struct.base_mode      = mavlink_system_struct.mode;
      mavlink_heartbeat_struct.system_status  = mavlink_system_struct.state;
      heartbeat_mail.payload = &mavlink_heartbeat_struct;
      chMBPost(&tolinkdm_mb, (msg_t)&heartbeat_mail, TIME_IMMEDIATE);
    }
    chBSemSignal(&sanity_sem);
    mavlink_sys_status_struct.load = get_cpu_load();
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
  chBSemInit(&blink_sem,  TRUE);
  IdleThread_p = chSysGetIdleThread();

  /**/
  chThdCreateStatic(SanityControlThreadWA,
          sizeof(SanityControlThreadWA),
          NORMALPRIO,
          SanityControlThread,
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

