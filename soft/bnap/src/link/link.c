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
const uint32_t *cc_heartbeat_sendperiod;
const uint32_t *cc_gps_raw_int_sendperiod;
const uint32_t *cc_global_position_int_sendperiod;
const uint32_t *cc_system_time_sendperiod;
const uint32_t *cc_mpiovd_sensors_sendperiod;
const uint32_t *cc_sys_status_sendperiod;
const uint32_t *cc_statustext_sendperiod;
const uint32_t *cc_param_value_sendperiod;

const uint32_t *dm_heartbeat_sendperiod;
const uint32_t *dm_gps_raw_int_sendperiod;
const uint32_t *dm_global_position_int_sendperiod;
const uint32_t *dm_system_time_sendperiod;
const uint32_t *dm_mpiovd_sensors_sendperiod;
const uint32_t *dm_sys_status_sendperiod;
const uint32_t *dm_statustext_sendperiod;
const uint32_t *dm_param_value_sendperiod;

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

  cc_heartbeat_sendperiod           = ValueSearch("T_heartbeat");
  cc_gps_raw_int_sendperiod         = ValueSearch("T_gps_raw");
  cc_global_position_int_sendperiod = ValueSearch("T_gps_int");
  cc_system_time_sendperiod         = ValueSearch("T_sys_time");
  cc_mpiovd_sensors_sendperiod      = ValueSearch("T_mpiovd_data");
  cc_sys_status_sendperiod          = ValueSearch("T_sys_status");
  cc_statustext_sendperiod          = ValueSearch("T_text");
  cc_param_value_sendperiod         = NULL;

  dm_heartbeat_sendperiod           = NULL;
  dm_gps_raw_int_sendperiod         = NULL;
  dm_global_position_int_sendperiod = NULL;
  dm_system_time_sendperiod         = NULL;
  dm_mpiovd_sensors_sendperiod      = NULL;
  dm_sys_status_sendperiod          = NULL;
  dm_statustext_sendperiod          = NULL;
  dm_param_value_sendperiod         = NULL;

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




