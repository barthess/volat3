#include "ch.h"
#include "hal.h"
#include "mavlink.h"

#include "message.h"
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
extern EventSource BnapEvent;
extern EventSource HeartbeatEvent;

/*
 ******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************
 */

/*
 *******************************************************************************
 *******************************************************************************
 * LOCAL FUNCTIONS
 *******************************************************************************
 *******************************************************************************
 */
//static void UnpackCycle(SerialDriver *sdp){
//  mavlink_message_t msg;
//  mavlink_status_t status;
//  msg_t c = 0;
//
//  while (!chThdShouldTerminate()) {
//    // Try to get a new message
//    c = sdGetTimeout((SerialDriver *)sdp, MS2ST(200));
//    if (c != Q_TIMEOUT){
//      uint8_t s = 0;
//      s = mavlink_parse_char(MAVLINK_COMM_0, (uint8_t)c, &msg, &status);
//      if (s) {
//        if (msg.sysid == GROUND_STATION_ID){ /* нас запрашивает наземная станция */
//          sort_input_messages(&msg);
//        }
//      }
//    }
//  }
//}

/**
 * Поток разбора входящих данных.
 */
static WORKING_AREA(DmUnpackerThreadWA, 2048);
static msg_t DmUnpackerThread(void *sdp){
  chRegSetThreadName("DmUnpacker");
//  UnpackCycle((SerialDriver *)sdp);
  chThdExit(0);
  return 0;
}

/**
 * Функция в цикле ждет, пока ей в почтовый ящик не упадет сообщение.
 * Как только сообщение упало - оно заворачивается в мавлинковый пакет
 * и пихается в последовательный порт.
 *
 * param[in]  sdp pointer to associated serial driver
 * param[in]  mbp pointer to associated message box
 */
static struct EventListener el_gps, el_heartbeat;

static void PackCycle(SerialDriver *sdp){

  const eventmask_t msk = EVENT_GPS_MSG_READY | EVENT_HERTBEAT_MSG_READY;
  chEvtRegisterMask(&BnapEvent, &el_gps, EVENT_GPS_MSG_READY);
  chEvtRegisterMask(&HeartbeatEvent, &el_heartbeat, EVENT_HERTBEAT_MSG_READY);
  eventmask_t evt;

  /* Переменная для формирования сообщения. Одна на всех,
     поскольку сообещиня все равно обрабатываются по одному. */
  mavlink_message_t mavlink_message_struct;

  /* выходной буфер для отправки данных */
  uint8_t sendbuf[MAVLINK_MAX_PACKET_LEN];
  uint16_t len = 0;

  while (!chThdShouldTerminate()) {
    evt = chEvtWaitOne(msk);
    switch(evt){
    case EVENT_GPS_MSG_READY:
      sdWrite((SerialDriver *)sdp, (uint8_t *)"Hello GPS   \r\n", 14);
      break;
    case EVENT_HERTBEAT_MSG_READY:
      sdWrite((SerialDriver *)sdp, (uint8_t *)"Heartbeat   \r\n", 14);
      break;
    default:
      break;
    }
  }
}

/**
 * Упаковка данных для модуля индюкации.
 * Подписано на сообщения:
 * gps,
 */
static WORKING_AREA(DmPackerThreadWA, 1024);
static msg_t DmPackerThread(void *sdp){
  chRegSetThreadName("DmPacker");
  PackCycle((SerialDriver *)sdp);
  chThdExit(0);
  return 0;
}

/*
 *******************************************************************************
 * EXPORTED FUNCTIONS
 *******************************************************************************
 */

/**
 * Fork link threads for mpiovd.
 */
void link_dm_up(SerialDriver *sdp){

  chThdCreateStatic(DmUnpackerThreadWA,
          sizeof(DmUnpackerThreadWA),
          DM_THREAD_PRIO,
          DmUnpackerThread,
          sdp);

  chThdCreateStatic(DmPackerThreadWA,
          sizeof(DmPackerThreadWA),
          DM_THREAD_PRIO,
          DmPackerThread,
          sdp);
}








