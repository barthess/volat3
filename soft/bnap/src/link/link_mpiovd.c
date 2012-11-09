#include "ch.h"
#include "hal.h"
#include "mavlink.h"

#include "link_mpiovd.h"
#include "message.h"
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

/**
 *
 */
static void __link_out_cycle(SerialDriver *sdp){
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
      sdWrite(sdp, sendbuf, len);
    }
  }
}

/**
 * Поток отправки сообещиний через канал связи на землю.
 */
static WORKING_AREA(LinkOutThreadWA, 1024);
static msg_t LinkOutThread(void *sdp){
  chRegSetThreadName("MpiovdLinkOut");
  __link_out_cycle((SerialDriver *)sdp);
  chThdExit(0);
  return 0;
}


/**
 * Поток разбора входящих данных.
 */
static WORKING_AREA(LinkInThreadWA, 2048);
static msg_t LinkInThread(void *sdp){
  chRegSetThreadName("MpiovdLinkIn");

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
          //sort_input_messages(&msg);
        }
      }
    }
  }

  chThdExit(0);
  return 0;
}


/*
 *******************************************************************************
 * EXPORTED FUNCTIONS
 *******************************************************************************
 */
void link_mpiovd_down(void){
  chDbgPanic("unimplemented yet");
}

/**
 * Fork link threads for mpiovd.
 */
void link_mpiovd_up(SerialDriver *sdp){

  chThdCreateStatic(LinkInThreadWA,
          sizeof(LinkInThreadWA),
          MPIOVD_THREAD_PRIO,
          LinkInThread,
          sdp);

  chThdCreateStatic(LinkOutThreadWA,
          sizeof(LinkOutThreadWA),
          MPIOVD_THREAD_PRIO,
          LinkOutThread,
          sdp);
}








