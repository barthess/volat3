#include "ch.h"
#include "hal.h"

#include "main.h"
#include "utils.h"
#include "message.h"
#include "sensors.h"

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
extern const RawData raw_data;
extern const mavlink_mpiovd_sensors_raw_t     mpiovd_sensors_raw_struct;

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
  //CAN_BTR_LBKM | CAN_BTR_SJW(1) | CAN_BTR_TS2(2) | CAN_BTR_TS1(11) | CAN_BTR_BRP(5),
  CAN_BTR_SJW(1) | CAN_BTR_TS2(2) | CAN_BTR_TS1(11) | CAN_BTR_BRP(5),
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
  uint32_t i;

  txmsg.IDE = CAN_IDE_EXT;
  txmsg.EID = 0x01234567;
  txmsg.RTR = CAN_RTR_DATA;
  txmsg.DLC = 8;
  for (i=0; i<8; i++)
    txmsg.data8[i] = 0;
//  txmsg.data32[0] = 0x55AA55AA;
//  txmsg.data32[1] = 0x00FF00FF;

  while (!chThdShouldTerminate()) {
    i = mpiovd_sensors_raw_struct.analog01;
    putinrange(i, 0, 255);
    txmsg.data8[0] = i;
    i = mpiovd_sensors_raw_struct.analog02;
    putinrange(i, 0, 255);
    txmsg.data8[1] = i;
    i = mpiovd_sensors_raw_struct.analog03;
    putinrange(i, 0, 255);
    txmsg.data8[2] = i;
    i = mpiovd_sensors_raw_struct.analog04;
    putinrange(i, 0, 255);
    txmsg.data8[3] = i;

    /* (>> 1) to exclude "dead" bit from SPI*/
    txmsg.data8[4] = (raw_data.discrete >> 1) & 0xFF;

    canTransmit(&CAND1, &txmsg, MS2ST(100));
    chThdSleepMilliseconds(250);
  }
  chThdExit(0);
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

