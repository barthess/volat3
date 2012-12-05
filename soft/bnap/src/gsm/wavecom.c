#include <string.h>
#include <stdio.h>

#include "ch.h"
#include "hal.h"
#include "chprintf.h"

#include "mavlink.h"

#include "main.h"
#include "wavecom.h"
#include "cross.h"
#include "link_cc.h"
#include "link.h"
#include "mavlink_dbg.h"
#include "settings_modem.h"

/*
 ******************************************************************************
 * DEFINES
 ******************************************************************************
 */

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
extern EepromFileStream ModemSettingsFile;
extern mavlink_oblique_rssi_t mavlink_oblique_rssi_struct;
extern Thread *modem_tp;

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
static uint8_t eeprombuf[EEPROM_MODEM_MAX_FIELD_LEN];

static int fake_rssi = 0; /* value for fake RSSI measurements */
static int fake_ber  = 0; /* value for fake BER measurements */

/*
 ******************************************************************************
 ******************************************************************************
 * LOCAL FUNCTIONS
 ******************************************************************************
 ******************************************************************************
 */

static void _trace_print(char *st, bool_t direction){
  (void)direction;
  size_t len = strlen(st);
  (void)len;

#if defined(SDMODEMTRACE)
  mavlink_dbg_print(MAV_SEVERITY_DEBUG, st);
  chThdSleepMilliseconds(5);
#else
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
    c = sdGetTimeout(sdp, MS2ST(2));
    if (c > 0){ /* was no timeout */
      w = (w << 8) | (c & 0xFF);
    }
    if (w == sign)
      break;
  }

  /* collect remaining part until \r\n */
  i = 0;
  while(((chTimeNow() - start) < timeout) && (i < (lim - 1))){
    c = sdGetTimeout(sdp, MS2ST(1));
    if (c > 0){ /* was no timeout */
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

  gsm_assert_reset();
  chThdSleepMilliseconds(100);
  gsm_release_reset();
  sdGet(sdp); /* wait first letter from modem */

//  chThdSleepMilliseconds(1200);
//  _say_to_modem(sdp, "+++");
//  chThdSleepMilliseconds(1200);

  /* reset all setting to defaults */
  _say_to_modem(sdp, "AT+CFUN=1\r");
  sdGet(sdp);

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
  chThdSleepMilliseconds(200);
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
static bool_t _wait_cgreg(SerialDriver *sdp){
  uint32_t try = CREG_TRY;
  int stat, mode;
  int scanfstat;

  while(try--){
    _say_to_modem(sdp, "AT+CGREG?\r");
    _collect_answer(sdp, gsmbuf, sizeof(gsmbuf), CREG_TMO);

    /* check results */
    scanfstat = siscanf((char *)gsmbuf, "+CGREG: %d,%d", &mode, &stat);
    if (scanfstat == 2){
      if ((stat == 1) || (stat == 5))
        return GSM_SUCCESS;
    }
    chThdSleep(CREG_TMO);
  }
  return GSM_FAILED;
}

/**
 * Wait registration on operator network
 */
static bool_t _update_rssi(SerialDriver *sdp){
  int scanfstat;

  /* not first function run */
  if ((fake_rssi != 0) && (fake_ber != 0))
    return GSM_SUCCESS;

  uint32_t try = 5;
  while(try--){
    _say_to_modem(sdp, "AT+CSQ\r");
    _collect_answer(sdp, gsmbuf, sizeof(gsmbuf), CREG_TMO);

    /* check results */
    scanfstat = siscanf((char *)gsmbuf, "+CSQ: %d,%d", &fake_rssi, &fake_ber);
    if (scanfstat == 2)
      return GSM_SUCCESS;
    else
      continue;
  }
  if (scanfstat != 2){
    fake_rssi = 21;
    fake_ber  = 99;
  }
  return GSM_SUCCESS;
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

  _say_to_modem(sdp, "AT+WIPCLOSE=1,1\r");
  chThdSleep(BEARER_TMO);
  _say_to_modem(sdp, "AT+WIPBR=0,6\r");
  chThdSleep(BEARER_TMO);

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
  read_modem_param(eeprombuf, &ModemSettingsFile, EEPROM_MODEM_APN_SIZE, EEPROM_MODEM_APN_OFFSET);
  try = BEARER_TRY;
  while(try--){
    _say_to_modem(sdp, "AT+WIPBR=2,6,11,\"");
    _say_to_modem(sdp, (char *)eeprombuf);
    _say_to_modem(sdp, "\"\r");
    _collect_answer(sdp, gsmbuf, sizeof(gsmbuf), BEARER_TMO);
    if (NULL != strstr((char *)gsmbuf, "OK"))
      break;
    chThdSleep(BEARER_TMO);
  }
  if (try == 0)
    return GSM_FAILED;

  /* user */
  read_modem_param(eeprombuf, &ModemSettingsFile, EEPROM_MODEM_USER_SIZE, EEPROM_MODEM_USER_OFFSET);
  try = BEARER_TRY;
  while(try--){
    _say_to_modem(sdp, "AT+WIPBR=2,6,0,\"");
    _say_to_modem(sdp, (char *)eeprombuf);
    _say_to_modem(sdp, "\"\r");
    _collect_answer(sdp, gsmbuf, sizeof(gsmbuf), BEARER_TMO);
    if (NULL != strstr((char *)gsmbuf, "OK"))
      break;
    chThdSleep(BEARER_TMO);
  }
  if (try == 0)
    return GSM_FAILED;

  /* password */
  read_modem_param(eeprombuf, &ModemSettingsFile, EEPROM_MODEM_PASS_SIZE, EEPROM_MODEM_PASS_OFFSET);
  try = BEARER_TRY;
  while(try--){
    _say_to_modem(sdp, "AT+WIPBR=2,6,1,\"");
    _say_to_modem(sdp, (char *)eeprombuf);
    _say_to_modem(sdp, "\"\r");
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
    if (NULL != strstr((char *)gsmbuf, "+CME ERROR: 803"))
      _load_bearer(sdp); /* try to reload bearer */
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
    //_say_to_modem(sdp, "AT+WIPCREATE=1,1,14555,\"86.57.157.114\",14550\r");

    _say_to_modem(sdp, "AT+WIPCREATE=1,1,");
    read_modem_param(eeprombuf, &ModemSettingsFile, EEPROM_MODEM_LISTEN_SIZE, EEPROM_MODEM_LISTEN_OFFSET);
    _say_to_modem(sdp, (char *)eeprombuf);
    _say_to_modem(sdp, ",\"");
    read_modem_param(eeprombuf, &ModemSettingsFile, EEPROM_MODEM_SERVER_SIZE, EEPROM_MODEM_SERVER_OFFSET);
    _say_to_modem(sdp, (char *)eeprombuf);
    _say_to_modem(sdp, "\",");
    read_modem_param(eeprombuf, &ModemSettingsFile, EEPROM_MODEM_PORT_SIZE, EEPROM_MODEM_PORT_OFFSET);
    _say_to_modem(sdp, (char *)eeprombuf);
    _say_to_modem(sdp, "\r");

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
static bool_t check_settings(void){
  read_modem_param(eeprombuf, &ModemSettingsFile, EEPROM_MODEM_PIN_SIZE, EEPROM_MODEM_PIN_OFFSET);
  if (0 == strlen((const char *)eeprombuf))
    return GSM_FAILED;
  read_modem_param(eeprombuf, &ModemSettingsFile, EEPROM_MODEM_APN_SIZE, EEPROM_MODEM_APN_OFFSET);
  if (0 == strlen((const char *)eeprombuf))
    return GSM_FAILED;
  read_modem_param(eeprombuf, &ModemSettingsFile, EEPROM_MODEM_USER_SIZE, EEPROM_MODEM_USER_OFFSET);
  if (0 == strlen((const char *)eeprombuf))
    return GSM_FAILED;
  read_modem_param(eeprombuf, &ModemSettingsFile, EEPROM_MODEM_PASS_SIZE, EEPROM_MODEM_PASS_OFFSET);
  if (0 == strlen((const char *)eeprombuf))
    return GSM_FAILED;
  read_modem_param(eeprombuf, &ModemSettingsFile, EEPROM_MODEM_SERVER_SIZE, EEPROM_MODEM_SERVER_OFFSET);
  if (0 == strlen((const char *)eeprombuf))
    return GSM_FAILED;
  read_modem_param(eeprombuf, &ModemSettingsFile, EEPROM_MODEM_PORT_SIZE, EEPROM_MODEM_PORT_OFFSET);
  if (0 == strlen((const char *)eeprombuf))
    return GSM_FAILED;
  read_modem_param(eeprombuf, &ModemSettingsFile, EEPROM_MODEM_LISTEN_SIZE, EEPROM_MODEM_LISTEN_OFFSET);
  if (0 == strlen((const char *)eeprombuf))
    return GSM_FAILED;
  return GSM_SUCCESS;
}

/**
 *
 */
bool_t _init_modem(SerialDriver *sdp){
  if ((GSM_FAILED == _wait_poweron(sdp)) || chThdShouldTerminate())
    return GSM_FAILED;
  _set_verbosity(sdp);
  if((GSM_FAILED == _wait_sim(sdp)) || chThdShouldTerminate())
    return GSM_FAILED;
  if ((GSM_FAILED == _wait_cgreg(sdp)) || chThdShouldTerminate())
    return GSM_FAILED;
  if ((GSM_FAILED == _update_rssi(sdp)) || chThdShouldTerminate())
    return GSM_FAILED;
  if ((GSM_FAILED == _start_wopen(sdp)) || chThdShouldTerminate())
    return GSM_FAILED;
  chThdSleepMilliseconds(100);
  if ((GSM_FAILED == _start_wipcfg(sdp)) || chThdShouldTerminate())
    return GSM_FAILED;
  chThdSleepMilliseconds(1000);
  if ((GSM_FAILED == _load_bearer(sdp)) || chThdShouldTerminate())
    return GSM_FAILED;
  chThdSleepMilliseconds(100);
  if ((GSM_FAILED == _set_apn(sdp)) || chThdShouldTerminate())
    return GSM_FAILED;
  chThdSleepMilliseconds(100);
  if ((GSM_FAILED == _start_bearer(sdp)) || chThdShouldTerminate())
    return GSM_FAILED;
  chThdSleepMilliseconds(1000);
  if ((GSM_FAILED == _create_connection(sdp)) || chThdShouldTerminate())
    return GSM_FAILED;
  chThdSleepMilliseconds(1000);
  if ((GSM_FAILED == _start_connection(sdp)) || chThdShouldTerminate())
    return GSM_FAILED;
  return GSM_SUCCESS;
}

/**
 *
 */
static WORKING_AREA(ModemThreadWA, 768);
static msg_t ModemThread(void *sdp) {
  chRegSetThreadName("Modem");
  uint32_t n;

  mavlink_dbg_print(MAV_SEVERITY_DEBUG, "MODEM: initializing");

  /* check correctness of settings stored in EEPROM */
  if (GSM_FAILED == check_settings()){
    mavlink_dbg_print(MAV_SEVERITY_ERROR, "MODEM: settings stored in EEPROM invalid");
    chThdSleepMilliseconds(5);
    mavlink_dbg_print(MAV_SEVERITY_ERROR, "MODEM: start shell and fix them manually");
    return RDY_RESET;
  }

  /* try to start modem */
  while (GSM_FAILED == _init_modem(sdp)){
    if (chThdShouldTerminate())
      return RDY_OK;

    mavlink_dbg_print(MAV_SEVERITY_ERROR, "*** ERROR! Can not connect.");
    mavlink_dbg_print(MAV_SEVERITY_ERROR, "*** Retry after 60 seconds.");

    n = 60;
    while (n--){
      chThdSleepSeconds(1);
      if (chThdShouldTerminate())
        return RDY_OK;
    }

    /* try to stop connection */
    acquire_cc_out();
    chThdSleepMilliseconds(1500);
    chprintf((BaseSequentialStream *)sdp, "%s", "+++");
    chThdSleepMilliseconds(1500);
    release_cc_out();
  }

  mavlink_dbg_print(MAV_SEVERITY_DEBUG, "*** SUCCESS! Connection established.\r\n");
  setGlobalFlag(GlobalFlags.modem_connected);

  while (!chThdShouldTerminate())
    chThdSleepMilliseconds(100);

  return RDY_OK;
}

/**
 *
 */
static WORKING_AREA(RssiThreadWA, 128);
static msg_t RssiThread(void *sdp) {
  chRegSetThreadName("Rssi");
  (void)sdp;

  while (!chThdShouldTerminate()) {
    chThdSleepMilliseconds(1000);
    if (GlobalFlags.modem_connected == 1)
      _update_rssi(sdp);
    mavlink_oblique_rssi_struct.rssi = fake_rssi;
    mavlink_oblique_rssi_struct.ber  = fake_ber;
  }
  return 0;
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
  modem_tp = chThdCreateStatic(
      ModemThreadWA,
      sizeof(ModemThreadWA),
      NORMALPRIO,
      ModemThread,
      &SDGSM);
  if (modem_tp == NULL)
    chDbgPanic("Can not allocate memory");

  chThdCreateStatic(RssiThreadWA,
          sizeof(RssiThreadWA),
          NORMALPRIO,
          RssiThread,
          &SDGSM);
}

/**
 * @brief   UDP writing routine using escape symbols.
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




