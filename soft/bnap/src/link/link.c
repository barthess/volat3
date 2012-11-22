#include "ch.h"
#include "hal.h"
#include "mavlink.h"

#include "link.h"
#include "link_dm.h"
#include "link_cc.h"
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

void LinkInit(void){
  link_dm_up(&SDDM);
  link_cc_up(&SDGSM);
}

/**
 * @brief             Traffic shaper.
 * @details           Limits sending frequency. Created to use in with driven
 *                    senders that send packet only if sending allowed
 *                    by period AND there is event about fresh data.
 *
 * @param[in] last    pointer to variable containing timestamp of last sent event
 * @param[in] perid   period of sending. Zero value denotes switching off.
 *
 * return TRUE if sending of packet allowed.
 */
bool_t traffic_limiter(systime_t *last, systime_t period){
  if (period == 0)
    return FALSE;

  if ((chTimeNow() - *last) >= period){
    *last = chTimeNow();
    return TRUE;
  }
  else
    return FALSE;
}

