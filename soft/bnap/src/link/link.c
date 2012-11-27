#include "ch.h"
#include "hal.h"

#include "mavlink.h"

#include "link.h"
#include "link_dm.h"
#include "link_cc.h"
#include "link_mpiovd.h"
#include "message.h"
#include "main.h"
#include "param.h"

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
const uint32_t *to_cc_gps_raw_int_sendperiod;
const uint32_t *to_cc_global_position_int_sendperiod;
const uint32_t *to_cc_system_time_sendperiod;
const uint32_t *to_cc_mpiovd_sensors_sendperiod;
const uint32_t *to_cc_sys_status_sendperiod;
const uint32_t *to_cc_statustext_sendperiod;
const uint32_t *to_cc_param_value_sendperiod;
const uint32_t *to_cc_heartbeat_mpiovd_sendperiod;
const uint32_t *to_cc_heartbeat_dm_sendperiod;
const uint32_t *to_cc_heartbeat_bnap_sendperiod;

const uint32_t *to_dm_heartbeat_sendperiod;
const uint32_t *to_dm_gps_raw_int_sendperiod;
const uint32_t *to_dm_global_position_int_sendperiod;
const uint32_t *to_dm_system_time_sendperiod;
const uint32_t *to_dm_mpiovd_sensors_sendperiod;
const uint32_t *to_dm_sys_status_sendperiod;
const uint32_t *to_dm_statustext_sendperiod;
const uint32_t *to_dm_param_value_sendperiod;
const uint32_t *to_dm_heartbeat_mpiovd_sendperiod;
const uint32_t *to_dm_heartbeat_cc_sendperiod;
const uint32_t *to_dm_heartbeat_bnap_sendperiod;

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

/*
 *******************************************************************************
 * EXPORTED FUNCTIONS
 *******************************************************************************
 */

/**
 *
 */
void LinkInit(void){

  to_cc_heartbeat_mpiovd_sendperiod    = ValueSearch("T_heartbeat");
  to_cc_heartbeat_dm_sendperiod        = ValueSearch("T_heartbeat");
  to_cc_heartbeat_bnap_sendperiod      = ValueSearch("T_heartbeat");

  to_cc_gps_raw_int_sendperiod         = ValueSearch("T_gps_raw");
  to_cc_global_position_int_sendperiod = ValueSearch("T_gps_int");
  to_cc_system_time_sendperiod         = ValueSearch("T_sys_time");
  to_cc_mpiovd_sensors_sendperiod      = ValueSearch("T_mpiovd_data");
  to_cc_sys_status_sendperiod          = ValueSearch("T_sys_status");
  to_cc_statustext_sendperiod          = ValueSearch("T_text");
  to_cc_param_value_sendperiod         = NULL;

  to_dm_heartbeat_mpiovd_sendperiod    = NULL;
  to_dm_heartbeat_cc_sendperiod        = NULL;
  to_dm_heartbeat_bnap_sendperiod      = NULL;

  to_dm_gps_raw_int_sendperiod         = NULL;
  to_dm_global_position_int_sendperiod = NULL;
  to_dm_system_time_sendperiod         = NULL;
  to_dm_mpiovd_sensors_sendperiod      = NULL;
  to_dm_sys_status_sendperiod          = NULL;
  to_dm_statustext_sendperiod          = NULL;
  to_dm_param_value_sendperiod         = NULL;

  link_dm_up(&SDDM);
  link_mpiovd_up(&SDMPIOVD);
  link_cc_up(&SDGSM);
}

/**
 * @brief             Traffic shaper.
 * @details           Limits sending frequency. Created to use in with driven
 *                    senders that send packet only if sending allowed
 *                    by period AND there is event about fresh data.
 *
 * @param[in] last    pointer to variable containing timestamp of last sent event
 * @param[in] perid   pointer to period of sending. Zero value denotes switching off.
 *                    NULL value denotes no limit.
 *
 * return TRUE if sending of packet allowed.
 */
bool_t traffic_limiter(systime_t *last, const systime_t *period){

  if (period == NULL)
    return TRUE;

  if (*period == 0)
    return FALSE;

  if ((chTimeNow() - *last) >= MS2ST(*period)){
    *last = chTimeNow();
    return TRUE;
  }
  else
    return FALSE;
}




