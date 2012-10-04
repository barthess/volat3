#include "ch.h"
#include "hal.h"

#include "main.h"
#include "cli.h"
#include "usb_local.h"

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
extern uint32_t GlobalFlags;

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
/**
 *
 */
static Thread* fork_usb_tread(void){
  return CliConnect(UsbInitLocal());
}

/**
 *
 */
static Thread* fork_can_tread(void){
  return NULL;
}

/**
 *
 */
static void term_usb_tread(Thread* tp){
  if (tp != NULL){
    chThdTerminate(tp);
    chThdWait(tp);
  }
  UsbStopLocal();
}

/**
 *
 */
static void term_can_tread(Thread* tp){
  if (tp != NULL){
    chThdTerminate(tp);
    chThdWait(tp);
  }
}



/**
 * Check presence of USB plug reading inputs
 */
static bool_t is_usb_present(void){
  return TRUE;
}

/**
 * Track presence of USB plug and fork appropriate threads
 */
static WORKING_AREA(UsbCanMgrThreadWA, 128);
static msg_t UsbCanMgrThread(void *arg){
  (void)arg;
  Thread* curr_tp = NULL;

  chRegSetThreadName("UsbCanManager");

  /* use CAN as starting point by default */
  setGlobalFlag(CAN_ACTIVE_FLAG);

  /* now track changes of flag and fork appropriate threads */
  while (TRUE) {
    chThdSleepMilliseconds(100);

    if(is_usb_present() && (GlobalFlags & CAN_ACTIVE_FLAG)){
      term_can_tread(curr_tp);
      clearGlobalFlag(CAN_ACTIVE_FLAG);
      curr_tp = fork_usb_tread();
    }
    else if(!is_usb_present() && !(GlobalFlags & CAN_ACTIVE_FLAG)){
      term_usb_tread(curr_tp);
      curr_tp = NULL;
      setGlobalFlag(CAN_ACTIVE_FLAG);
      curr_tp = fork_can_tread();
    }
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
void CanUsbMgrInit(void){
  chThdCreateStatic(UsbCanMgrThreadWA,
          sizeof(UsbCanMgrThreadWA),
          LINK_THREADS_PRIO,
          UsbCanMgrThread,
          NULL);
}

/**
 * @brief   USB high priority interrupt handler -- STM32_USB1_HP_HANDLER.
 *          CAN1 TX interrupt handler -- STM32_CAN1_TX_HANDLER.
 *          Vector8C
 * @isr
 */
CH_IRQ_HANDLER(STM32_USB1_HP_HANDLER) {
  if (GlobalFlags & CAN_ACTIVE_FLAG)
    volat_can1_tx_handler();
  else
    volat_usb1_hp_handler();
}

/**
 * @brief   USB low priority interrupt handler -- STM32_USB1_LP_HANDLER.
 *          CAN1 RX0 interrupt handler -- STM32_CAN1_RX0_HANDLER.
 *          Vector90
 * @isr
 */
CH_IRQ_HANDLER(STM32_USB1_LP_HANDLER) {
  if (GlobalFlags & CAN_ACTIVE_FLAG)
    volat_can1_rx0_handler();
  else
    volat_usb1_lp_handler();
}



