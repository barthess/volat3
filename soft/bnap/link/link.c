#include "ch.h"
#include "hal.h"
#include "mavlink.h"

#include "link.h"
#include "message.h"
#include "main.h"

//#include "link_sortin.h"
//#include "link_sortout.h"

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
static const SerialConfig gsm_ser_cfg = {
    GSM_BAUDRATE,
    AT91C_US_USMODE_HWHSH | AT91C_US_CLKS_CLOCK | AT91C_US_CHRL_8_BITS |
                              AT91C_US_PAR_NONE | AT91C_US_NBSTOP_1_BIT
};
static const SerialConfig mpiovd_ser_cfg = {
    MPIOVD_BAUDRATE,
    AT91C_US_USMODE_NORMAL | AT91C_US_CLKS_CLOCK | AT91C_US_CHRL_8_BITS |
                              AT91C_US_PAR_NONE | AT91C_US_NBSTOP_1_BIT
};
static const SerialConfig dm_ser_cfg = {
    DM_BAUDRATE,
    AT91C_US_USMODE_NORMAL | AT91C_US_CLKS_CLOCK | AT91C_US_CHRL_8_BITS |
                              AT91C_US_PAR_NONE | AT91C_US_NBSTOP_1_BIT
};

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


/*
 *******************************************************************************
 * EXPORTED FUNCTIONS
 *******************************************************************************
 */

void link_dm_down(void){return ;}
void link_cc_down(void){return ;}


static void link_dm_up(SerialDriver *sdp){return ;}
static void link_cc_up(SerialDriver *sdp){return ;}


void LinkInit(void){
  sdStart(&SDGSM, &gsm_ser_cfg);
  link_cc_up(&SDGSM);

  sdStart(&SDMPIOVD, &mpiovd_ser_cfg);
  link_mpiovd_up(&SDMPIOVD);

  sdStart(&SDDM, &dm_ser_cfg);
  link_dm_up(&SDDM);
}





