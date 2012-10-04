#include "ch.h"
#include "hal.h"

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
extern MemoryHeap ThdHeap;

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
 * Internal loopback mode, 500KBaud, automatic wakeup, automatic recover
 * from abort mode.
 * See section 22.7.7 on the STM32 reference manual.
 */
static const CANConfig cancfg = {
  CAN_MCR_ABOM | CAN_MCR_AWUM | CAN_MCR_TXFP,
  CAN_BTR_LBKM | CAN_BTR_SJW(0) | CAN_BTR_TS2(1) |
  CAN_BTR_TS1(8) | CAN_BTR_BRP(6),
  0,
  NULL
};

static Thread *can_tp;

/*
 ******************************************************************************
 ******************************************************************************
 * LOCAL FUNCTIONS
 ******************************************************************************
 ******************************************************************************
 */
/*
 * Transmitter thread.
 */
static WORKING_AREA(CanTxThreadWA, 256);
static msg_t CanTxThread(void * arg) {
  (void)arg;
  chRegSetThreadName("CanTx");

  CANTxFrame txmsg;

  txmsg.IDE = CAN_IDE_EXT;
  txmsg.EID = 0x01234567;
  txmsg.RTR = CAN_RTR_DATA;
  txmsg.DLC = 8;
  txmsg.data32[0] = 0x55AA55AA;
  txmsg.data32[1] = 0x00FF00FF;

  while (!chThdShouldTerminate()) {
    canTransmit(&CAND1, &txmsg, MS2ST(100));
    chThdSleepMilliseconds(250);

    if (chThdShouldTerminate())
      chThdExit(0);
  }
  return 0;
}

/*
 ******************************************************************************
 * EXPORTED FUNCTIONS
 ******************************************************************************
 */

/**
 *
 */
Thread* CanInitLocal(void){
  canInit();
  canStart(&CAND1, &cancfg);

  can_tp = chThdCreateFromHeap(
      &ThdHeap,
      sizeof(CanTxThreadWA),
      LINK_THREADS_PRIO - 2,
      CanTxThread,
      NULL);

  if (can_tp == NULL)
    chDbgPanic("Can not allocate memory");

  return can_tp;
}

/**
 *
 */
void CanStopLocal(void){
  canStop(&CAND1);
}

