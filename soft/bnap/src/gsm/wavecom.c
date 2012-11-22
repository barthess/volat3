#include <string.h>
#include <stdio.h>

#include "ch.h"
#include "hal.h"
#include "chprintf.h"

#include "main.h"
#include "wavecom.h"
#include "cross.h"

/*
 ******************************************************************************
 * DEFINES
 ******************************************************************************
 */
/* special character for data flow control */
#define ETX                 3  /* End of text */
#define DLE                 16 /*  Data link escape */

/*timeout definiton */
#define POWERON_TRY         10
#define POWERON_TMO         MS2ST(500)
#define CPIN_TRY            10
#define CPIN_TMO            MS2ST(1000)
#define CREG_TRY            60
#define CREG_TMO            MS2ST(1000)
#define WOPEN_TRY           10
#define WOPEN_TMO           MS2ST(1000)
#define WIPCFG_TRY          10
#define WIPCFG_TMO          MS2ST(1000)
#define BEARER_TRY          10
#define BEARER_TMO          MS2ST(1000)

#define SDMODEMTRACE        SDDM /* serial port for printing debug info during modem init */

/*
 ******************************************************************************
 * EXTERNS
 ******************************************************************************
 */
extern GlobalFlags_t GlobalFlags;

/*
 ******************************************************************************
 * PROTOTYPES
 ******************************************************************************
 */

/*
 ******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************
 */
/* buffer for collectin answers from */
static uint8_t gsmbuf[128];

/*
 ******************************************************************************
 ******************************************************************************
 * LOCAL FUNCTIONS
 ******************************************************************************
 ******************************************************************************
 */

static void _trace_print(char *st, bool_t direction){
#if defined(SDMODEMTRACE)
  size_t len = strlen(st);
  uint32_t i = 0;

  if(direction)
    sdWrite(&SDMODEMTRACE, (uint8_t *)"<", 1);
  else
    sdWrite(&SDMODEMTRACE, (uint8_t *)" ", 1);

  while(i < len){
    uint8_t c = st[i];
    if (c == '\r'){
      sdPut(&SDMODEMTRACE, '\\');
      sdPut(&SDMODEMTRACE, 'r');
    }
    else if (c == '\n'){
      sdPut(&SDMODEMTRACE, '\\');
      sdPut(&SDMODEMTRACE, 'n');
    }
    else{
      sdPut(&SDMODEMTRACE, c);
    }
    i++;
  }
  sdWrite(&SDMODEMTRACE, (uint8_t *)"\r\n", 2);
#else
  (void)sdp;
  (void)st;
#endif
}

/**
 *
 */
static void _say_to_modem(SerialDriver *sdp, char *st){
  chSysLock();
  chIQResetI(&(sdp->iqueue));
  chOQResetI(&(sdp->oqueue));
  chSysUnlock();

  chprintf((BaseSequentialStream *)sdp, "%s", st);
  _trace_print(st, TRUE);
}

/**
 * Store AT answer from modem in buffer.
 * return size of got bytes.
 */
static size_t _collect_answer(SerialDriver *sdp, uint8_t *buf, size_t lim, systime_t timeout){

  systime_t start = chTimeNow();
  const uint16_t sign = ('\r' << 8) | '\n';
  uint16_t w = 0;
  uint32_t i;
  msg_t c;

  /* wait starting \r\n */
  while ((chTimeNow() - start) < timeout){
    c = sdGetTimeout(sdp, MS2ST(1));
    if (c > 0){ /* no timeout */
      w = (w << 8) | (c & 0xFF);
    }
    if (w == sign)
      break;
  }

  /* collect remaining part until \r\n */
  i = 0;
  while(((chTimeNow() - start) < timeout) && (i < (lim - 1))){
    c = sdGetTimeout(sdp, MS2ST(1));
    if (c > 0){ /* no timeout */
      buf[i] = c;
      w = (w << 8) | (c & 0xFF);
      i++;
    }
    if (w == sign)
      break;
  }

  buf[i] = 0x00; /* very important to denote end of string with NULL simbol */
  _trace_print((char *)buf, FALSE);
  return i;
}

/**
 * Poll modem wiht "AT\r" string to check its presence.
 */
static bool_t _wait_poweron(SerialDriver *sdp){
  uint32_t try = POWERON_TRY;

  chThdSleepMilliseconds(1200);
  _say_to_modem(sdp, "+++");
  chThdSleepMilliseconds(1200);
//  _say_to_modem(sdp, "AT+WIPCLOSE=1,1\r");
//  chThdSleepMilliseconds(200);

  /* reset all setting to defaults */
  _say_to_modem(sdp, "AT+CFUN=1\r");
  chThdSleepMilliseconds(3000);

  while(try--){
    _say_to_modem(sdp, "AT\r");
    _collect_answer(sdp, gsmbuf, sizeof(gsmbuf), POWERON_TMO);

    /* check results */
    if (NULL != strstr((char *)gsmbuf, "OK"))
      return GSM_SUCCESS;
    chThdSleep(POWERON_TMO);
  }
  return GSM_FAILED;
}

/**
 *
 */
static void _set_verbosity(SerialDriver *sdp){
  /* echo off */
  _say_to_modem(sdp, "ATE0\r");
  chThdSleepMilliseconds(50);
  /* do not print automatically information about registration on network */
  _say_to_modem(sdp, "AT+CGREG=0\r");
  chThdSleepMilliseconds(50);
}

/**
 *
 */
static bool_t _wait_sim(SerialDriver *sdp){
  uint32_t try = CPIN_TRY;

  while(try--){
    _say_to_modem(sdp, "AT+CPIN?\r");
    _collect_answer(sdp, gsmbuf, sizeof(gsmbuf), CPIN_TMO);

    /* check results */
    if (NULL != strstr((char *)gsmbuf, ": READY"))
      return GSM_SUCCESS;
    chThdSleep(CPIN_TMO);
  }
  return GSM_FAILED;
}

/**
 * Wait registration on operator network
 */
static bool_t _wait_creg(SerialDriver *sdp){
  uint32_t try = CREG_TRY;
  int stat, mode;
  int scanfstat;

  while(try--){
    _say_to_modem(sdp, "AT+CREG?\r");
    _collect_answer(sdp, gsmbuf, sizeof(gsmbuf), CREG_TMO);

    /* check results */
    scanfstat = siscanf((char *)gsmbuf, "+CREG: %d,%d", &mode, &stat);
    if (scanfstat == 2){
      if ((stat == 1) || (stat == 5))
        return GSM_SUCCESS;
    }
    chThdSleep(CREG_TMO);
  }
  return GSM_FAILED;
}

/**
 *
 */
static bool_t _start_wopen(SerialDriver *sdp){
  uint32_t try = WOPEN_TRY;
  int stat;
  int scanfstat;

  while(try--){
    _say_to_modem(sdp, "AT+WOPEN?\r");
    _collect_answer(sdp, gsmbuf, sizeof(gsmbuf), WOPEN_TMO);

    /* check results */
    scanfstat = siscanf((char *)gsmbuf, "+WOPEN: %d", &stat);
    if (scanfstat == 1){
      if (stat == 1)
        return GSM_SUCCESS;
      else{
        chprintf((BaseSequentialStream *)sdp, "%s", "AT+WOPEN=1\r");
        chThdSleepMilliseconds(2000);
        chprintf((BaseSequentialStream *)sdp, "%s", "AT+CFUN=1\r");
        return GSM_SUCCESS;
      }
    }
    chThdSleep(WOPEN_TMO);
  }
  return GSM_FAILED;
}

/**
 *
 */
static bool_t _start_wipcfg(SerialDriver *sdp){
  uint32_t try = WIPCFG_TRY;

  while(try--){
    _say_to_modem(sdp, "AT+WIPCFG=1\r");
    _collect_answer(sdp, gsmbuf, sizeof(gsmbuf), WIPCFG_TMO);

    /* check results */
    if (NULL != strstr((char *)gsmbuf, "OK"))
      return GSM_SUCCESS;
    if (NULL != strstr((char *)gsmbuf, "+CME ERROR: 844"))
      return GSM_SUCCESS;

    chThdSleep(WIPCFG_TMO);
  }
  return GSM_FAILED;
}

/**
 *
 */
static bool_t _load_bearer(SerialDriver *sdp){
  uint32_t try = BEARER_TRY;

  while(try--){
    _say_to_modem(sdp, "AT+WIPBR=1,6\r");
    _collect_answer(sdp, gsmbuf, sizeof(gsmbuf), BEARER_TMO);
    if (NULL != strstr((char *)gsmbuf, "OK"))
      return GSM_SUCCESS;
    if (NULL != strstr((char *)gsmbuf, "+CME ERROR: 804"))
      return GSM_SUCCESS;
    chThdSleep(BEARER_TMO);
  }
  return GSM_FAILED;
}

/**
 *
 */
static bool_t _set_apn(SerialDriver *sdp){
  uint32_t try;

  /* apn name */
  try = BEARER_TRY;
  while(try--){
    _say_to_modem(sdp, "AT+WIPBR=2,6,11,\"m2m30.velcom.by\"\r");
    _collect_answer(sdp, gsmbuf, sizeof(gsmbuf), BEARER_TMO);
    if (NULL != strstr((char *)gsmbuf, "OK"))
      break;
    chThdSleep(BEARER_TMO);
  }
  if (try == 0)
    return GSM_FAILED;

  /* user */
  try = BEARER_TRY;
  while(try--){
    _say_to_modem(sdp, "AT+WIPBR=2,6,0,\"m2m30\"\r");
    _collect_answer(sdp, gsmbuf, sizeof(gsmbuf), BEARER_TMO);
    if (NULL != strstr((char *)gsmbuf, "OK"))
      break;
    chThdSleep(BEARER_TMO);
  }
  if (try == 0)
    return GSM_FAILED;

  /* password */
  try = BEARER_TRY;
  while(try--){
    _say_to_modem(sdp, "AT+WIPBR=2,6,1,\"m2m30\"\r");
    _collect_answer(sdp, gsmbuf, sizeof(gsmbuf), BEARER_TMO);
    if (NULL != strstr((char *)gsmbuf, "OK"))
      break;
    chThdSleep(BEARER_TMO);
  }
  if (try == 0)
    return GSM_FAILED;
  else
    return GSM_SUCCESS;
}

/**
 *
 */
static bool_t _start_bearer(SerialDriver *sdp){
  uint32_t try = BEARER_TRY;

  while(try--){
    _say_to_modem(sdp, "AT+WIPBR=4,6,0\r");
    _collect_answer(sdp, gsmbuf, sizeof(gsmbuf), BEARER_TMO);
    if (NULL != strstr((char *)gsmbuf, "OK"))
      return GSM_SUCCESS;
    chThdSleep(BEARER_TMO);
  }
  return GSM_FAILED;
}

/**
 *
 */
static bool_t _create_connection(SerialDriver *sdp){
  uint32_t try = BEARER_TRY;

  while(try--){
    _say_to_modem(sdp, "AT+WIPCREATE=1,1,14551,\"77.67.201.231\",14550\r");
    _collect_answer(sdp, gsmbuf, sizeof(gsmbuf), BEARER_TMO);
    if (NULL != strstr((char *)gsmbuf, "OK"))
      return GSM_SUCCESS;
    if (NULL != strstr((char *)gsmbuf, "+CME ERROR: 840"))
      return GSM_SUCCESS;
    chThdSleep(BEARER_TMO);
  }
  return GSM_FAILED;
}

/**
 *
 */
static bool_t _start_connection(SerialDriver *sdp){
  uint32_t try = BEARER_TRY;

  while(try--){
    _say_to_modem(sdp, "AT+WIPDATA=1,1,1\r");
    _collect_answer(sdp, gsmbuf, sizeof(gsmbuf), BEARER_TMO);
    if (NULL != strstr((char *)gsmbuf, "CONNECT"))
      return GSM_SUCCESS;
    chThdSleep(BEARER_TMO);
  }
  return GSM_FAILED;
}

/**
 *
 */
static WORKING_AREA(ModemThreadWA, 1024);
static msg_t ModemThread(void *sdp) {
  chRegSetThreadName("Modem");

  if (GSM_FAILED == _wait_poweron(sdp))
    goto ERROR;
  _set_verbosity(sdp);
  if(GSM_FAILED == _wait_sim(sdp))
    goto ERROR;
  if (GSM_FAILED == _wait_creg(sdp))
    goto ERROR;
  if (GSM_FAILED == _start_wopen(sdp))
    goto ERROR;
  if (GSM_FAILED == _start_wipcfg(sdp))
    goto ERROR;
  if (GSM_FAILED == _load_bearer(sdp))
    goto ERROR;
  if (GSM_FAILED == _set_apn(sdp))
    goto ERROR;
  if (GSM_FAILED == _start_bearer(sdp))
    goto ERROR;
  if (GSM_FAILED == _create_connection(sdp))
    goto ERROR;
  chThdSleepMilliseconds(1000);
  if (GSM_FAILED == _start_connection(sdp))
    goto ERROR;

  chprintf((BaseSequentialStream *)&SDDM, "%s", "*** SUCCESS! Connection established.\r\n");
  setGlobalFlag(GlobalFlags.modem_connected);
  //chThdExit(0);

  while (!chThdShouldTerminate()) {
//    chprintf((BaseSequentialStream *)sdp, "%s - %U\r\n", "bnap test", chTimeNow());
//    sdPut((SerialDriver *)sdp, ETX); /* end of packet */
    chThdSleepMilliseconds(1000);
  }

ERROR:
  chThdSleepMilliseconds(100);
  chprintf((BaseSequentialStream *)&SDDM, "%s", "*** FAILED! Automatic init timed out\r\n");
  chprintf((BaseSequentialStream *)&SDDM, "%s", "*** Do it yourself manually\r\n");
  chprintf((BaseSequentialStream *)&SDDM, "%s", "*** Hint: to enable echo print 'ATE1'\r\n");
  ModemCrossInit();
  return RDY_RESET;
}

/*
 ******************************************************************************
 * EXPORTED FUNCTIONS
 ******************************************************************************
 */
/**
 * Note! All modem initialization will be done in background thread.
 */
void ModemInit(void){
  chThdCreateStatic(ModemThreadWA,
          sizeof(ModemThreadWA),
          NORMALPRIO,
          ModemThread,
          &SDGSM);
}

/**
 * @brief   UDP write with timeout.
 */
void UdpSdWrite(SerialDriver *sdp, const uint8_t *bp, size_t n) {

  uint32_t i = 0;
  uint8_t b;

  while (i < n){
    b = bp[i];
    if ((b == ETX) || (b == DLE))
      sdPut(sdp, DLE);
    sdPut(sdp, b);
    i++;
  }
  sdPut(sdp, ETX); /* end of message */
}




