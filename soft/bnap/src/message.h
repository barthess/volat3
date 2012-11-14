#ifndef MESSAGE_H_
#define MESSAGE_H_

/* event masks */
#define EVMSK_HEARTBEAT             (1UL << 0)
#define EVMSK_GPS_RAW_INT           (1UL << 1)
#define EVMSK_GLOBAL_POSITION_INT   (1UL << 2)
#define EVMSK_GPS_TIME_GOT          (1UL << 3)
#define EVMSK_SYSTEM_TIME           (1UL << 4)
#define EVMSK_SYS_STATUS            (1UL << 5)
#define EVMSK_MPIOVD_SENSORS        (1UL << 6)
#define EVMSK_STATUSTEXT            (1UL << 7)

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

