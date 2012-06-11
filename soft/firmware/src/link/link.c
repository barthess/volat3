#include "ch.h"
#include "hal.h"

#include "link.h"
#include "cli.h"
#include "usb_local.h"
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

  while (TRUE) {
    if (chThdShouldTerminate())
      chThdExit(0);

    if (chMBFetch(&tolink_mb, &tmp, MS2ST(200)) == RDY_OK){
      mailp = (Mail*)tmp;
      sort_output_mail(mailp, &mavlink_msgbuf);
      len = mavlink_msg_to_send_buffer(sendbuf, &mavlink_msgbuf);
      sdWrite((SerialDriver *)sdp, sendbuf, len);
    }
  }

  return 0;
}


/**
 * Поток разбора входящих данных.
 */
static WORKING_AREA(LinkInThreadWA, 1024);
static msg_t LinkInThread(void *sdp){
  chRegSetThreadName("MAVLinkIn");

  mavlink_message_t msg;
  mavlink_status_t status;
  msg_t c = 0;

  while (TRUE) {
    if (chThdShouldTerminate())
      chThdExit(0);

    // Try to get a new message
    c = sdGetTimeout((SerialDriver *)sdp, MS2ST(200));
    if (c != Q_TIMEOUT){
      if (mavlink_parse_char(MAVLINK_COMM_0, (uint8_t)c, &msg, &status)) {
        if (msg.sysid == GROUND_STATION_ID){ /* нас запрашивает наземная станция */
          sort_input_messages(&msg);
        }
      }
    }
  }
  return 0;
}


/**
 * порождает потоки сортировки\парсинга сообщений
 */
void MavlinkConnect(void *sdp_mav){
  linkout_tp = chThdCreateStatic(LinkOutThreadWA,
                            sizeof(LinkOutThreadWA),
                            LINK_THREADS_PRIO,
                            LinkOutThread,
                            sdp_mav);
  linkin_tp = chThdCreateStatic(LinkInThreadWA,
                            sizeof(LinkInThreadWA),
                            LINK_THREADS_PRIO,
                            LinkInThread,
                            sdp_mav);
}



/*
 *******************************************************************************
 * EXPORTED FUNCTIONS
 *******************************************************************************
 */

void LinkInit(void){
  MavlinkConnect(UartInitLocal());
  CliConnect(UsbInitLocal());
  MavSenderInit();
}





