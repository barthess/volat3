#include "ch.h"
#include "hal.h"

#include "link.h"
#include "linkmgr.h"
#include "cli.h"
#include "usb_local.h"
#include "can_usb.h"
#include "uart_local.h"
#include "message.h"
#include "mavsender.h"
#include "main.h"

#include "link_sortin.h"
#include "link_sortout.h"

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
extern Mailbox tolink_mb;
extern uint32_t GlobalFlags;
extern MemoryHeap ThdHeap;

/*
 ******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************
 */
/* pointers to spawned threads */
static Thread *linkout_tp = NULL;
static Thread *linkin_tp = NULL;

/*
 *******************************************************************************
 *******************************************************************************
 * LOCAL FUNCTIONS
 *******************************************************************************
 *******************************************************************************
 */

/**
 * Поток отправки сообещиний через канал связи на землю.
 */
static WORKING_AREA(LinkOutThreadWA, 1024);
static msg_t LinkOutThread(void *sdp){
  chRegSetThreadName("MAVLinkOut");

  /* Переменная для формирования сообщения. Одна на всех,
     поскольку сообещиня обрабатываются по одному. */
  mavlink_message_t mavlink_msgbuf;
  /* выходной буфер для отправки данных */
  uint8_t sendbuf[MAVLINK_MAX_PACKET_LEN];
  uint16_t len = 0;
  Mail *mailp;
  msg_t tmp = 0;

  while (!chThdShouldTerminate()) {
    if (chMBFetch(&tolink_mb, &tmp, MS2ST(200)) == RDY_OK){
      mailp = (Mail*)tmp;
      sort_output_mail(mailp, &mavlink_msgbuf);
      len = mavlink_msg_to_send_buffer(sendbuf, &mavlink_msgbuf);
      sdWrite((SerialDriver *)sdp, sendbuf, len);
    }
  }

  chThdExit(0);
  return 0;
}


/**
 * Поток разбора входящих данных.
 */
static WORKING_AREA(LinkInThreadWA, 2048);
static msg_t LinkInThread(void *sdp){
  chRegSetThreadName("MAVLinkIn");

  mavlink_message_t msg;
  mavlink_status_t status;
  msg_t c = 0;

  while (!chThdShouldTerminate()) {
    // Try to get a new message
    c = sdGetTimeout((SerialDriver *)sdp, MS2ST(200));
    if (c != Q_TIMEOUT){
      uint8_t s = 0;
      s = mavlink_parse_char(MAVLINK_COMM_0, (uint8_t)c, &msg, &status);
      if (s) {
        if (msg.sysid == GROUND_STATION_ID){ /* нас запрашивает наземная станция */
          sort_input_messages(&msg);
        }
      }
    }
  }

  chThdExit(0);
  return 0;
}

/**
 * Kills previously spawned threads
 */
void KillMavlinkThreads(void){
  clearGlobalFlag(TLM_ACTIVE_FLAG);

  chThdTerminate(linkout_tp);
  chThdTerminate(linkin_tp);

  chThdWait(linkout_tp);
  chThdWait(linkin_tp);
}

/**
 * порождает потоки сортировки\парсинга сообщений
 */
void SpawnMavlinkThreads(void *sdp){
  linkout_tp = chThdCreateFromHeap(&ThdHeap,
                            sizeof(LinkOutThreadWA),
                            LINK_THREADS_PRIO,
                            LinkOutThread,
                            sdp);
  if (linkout_tp == NULL)
    chDbgPanic("Can not allocate memory");

  linkin_tp = chThdCreateFromHeap(&ThdHeap,
                            sizeof(LinkInThreadWA),
                            LINK_THREADS_PRIO,
                            LinkInThread,
                            sdp);
  if (linkin_tp == NULL)
    chDbgPanic("Can not allocate memory");

  setGlobalFlag(TLM_ACTIVE_FLAG);
}

/*
 *******************************************************************************
 * EXPORTED FUNCTIONS
 *******************************************************************************
 */

void LinkInit(void){
  LinkMgrInit(UartInitLocal());

  CanUsbMgrInit();
  MavSenderInit();
}





