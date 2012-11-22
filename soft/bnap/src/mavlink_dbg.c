#include <stdio.h>

#include "ch.h"
#include "hal.h"

#include "mavlink.h"

#include "main.h"
#include "message.h"

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
extern mavlink_statustext_t mavlink_statustext_struct;
extern EventSource event_statustext;

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
 * Send debug message.
 *
 * severity[in]   severity of message
 * text[in]       text to send
 *
 * return         message posting status
 */
void mavlink_dbg_print(uint8_t severity, const char *text){
  uint32_t n = sizeof(mavlink_statustext_struct.text);

  mavlink_statustext_struct.severity = severity;
  memset(mavlink_statustext_struct.text, 0, n);
  memcpy(mavlink_statustext_struct.text, text, n);

  chEvtBroadcastFlags(&event_statustext, EVMSK_STATUSTEXT);
}

