#ifndef MESSAGE_H_
#define MESSAGE_H_

#include "mavlink.h"

/* defines for autogenerated code */
typedef mavlink_heartbeat_t     mavlink_heartbeat_mpiovd_t;
typedef mavlink_heartbeat_t     mavlink_heartbeat_cc_t;
typedef mavlink_heartbeat_t     mavlink_heartbeat_dm_t;
typedef mavlink_heartbeat_t     mavlink_heartbeat_bnap_t;

/* defines for autogenerated code */
#define mavlink_msg_heartbeat_cc_encode(a,b,c,d)      mavlink_msg_heartbeat_encode((a),(b),(c),(d))
#define mavlink_msg_heartbeat_dm_encode(a,b,c,d)      mavlink_msg_heartbeat_encode((a),(b),(c),(d))
#define mavlink_msg_heartbeat_mpiovd_encode(a,b,c,d)  mavlink_msg_heartbeat_encode((a),(b),(c),(d))
#define mavlink_msg_heartbeat_bnap_encode(a,b,c,d)    mavlink_msg_heartbeat_encode((a),(b),(c),(d))

/* event masks */
#define EVMSK_MAVLINK_HEARTBEAT_BNAP        (1UL << 0)
#define EVMSK_MAVLINK_GPS_RAW_INT           (1UL << 1)
#define EVMSK_MAVLINK_GLOBAL_POSITION_INT   (1UL << 2)
#define EVMSK_GPS_TIME_GOT                  (1UL << 3)
#define EVMSK_MAVLINK_SYSTEM_TIME           (1UL << 4)
#define EVMSK_MAVLINK_SYS_STATUS            (1UL << 5)
#define EVMSK_MAVLINK_MPIOVD_SENSORS        (1UL << 6)
#define EVMSK_MAVLINK_STATUSTEXT            (1UL << 7)
#define EVMSK_MAVLINK_PARAM_VALUE           (1UL << 8)
#define EVMSK_MAVLINK_PARAM_SET             (1UL << 9)
#define EVMSK_MAVLINK_PARAM_REQUEST_LIST    (1UL << 10)
#define EVMSK_MAVLINK_PARAM_REQUEST_READ    (1UL << 11)
#define EVMSK_MAVLINK_COMMAND_LONG          (1UL << 12)
#define EVMSK_MAVLINK_COMMAND_ACK           (1UL << 13)
#define EVMSK_MAVLINK_HEARTBEAT_CC          (1UL << 14)
#define EVMSK_MAVLINK_HEARTBEAT_DM          (1UL << 15)
#define EVMSK_MAVLINK_HEARTBEAT_MPIOVD      (1UL << 16)
#define EVMSK_MAVLINK_OBLIQUE_AGPS          (1UL << 17)
#define EVMSK_MAVLINK_OBLIQUE_RSSI          (1UL << 18)

/**
 * Structure for data exchange with confimation capability.
 */
typedef struct Mail Mail;
struct Mail{
  /**
   * @brief   pointer to external buffer.
   * @details When receiver got data it must be set this pointer to NULL
   *          as a ready flag.
   */
  void *payload;
  /**
   * Content is on program responsibility. Can be contain anything.
   */
  msg_t invoice;
  /**
   * Protection semaphore.
   * Set to NULL if unused.
   */
  BinarySemaphore *semp;
};

void ReleaseMail(Mail* mailp);
void MsgInit(void);
void MavInit(void);


#endif /* MESSAGE_H_ */

