/*----------------------------------------------------------------------------
 *      R T L  -  K e r n e l
 *----------------------------------------------------------------------------
 *      Name:    RTX_CONFIG.C
 *      Purpose: Configuration of RTX Kernel for Atmel AT91SAM7A3
 *      Rev.:    V3.40
 *----------------------------------------------------------------------------
 *      This code is part of the RealView Run-Time Library.
 *      Copyright (c) 2004-2008 KEIL - An ARM Company. All rights reserved.
 *---------------------------------------------------------------------------*/

#include <RTL.h>
#include <AT91SAM7A3.H>

/*----------------------------------------------------------------------------
 *      RTX User configuration part BEGIN
 *---------------------------------------------------------------------------*/

//-------- <<< Use Configuration Wizard in Context Menu >>> -----------------
//
// <h>Task Definitions
// ===================
//
//   <o>Number of concurrent running tasks <0-250>
//   <i> Define max. number of tasks that will run at the same time.
//   <i> Default: 6
#ifndef OS_TASKCNT
 #define OS_TASKCNT     14
#endif

//   <o>Number of tasks with user-provided stack <0-250>
//   <i> Define the number of tasks that will use a bigger stack.
//   <i> The memory space for the stack is provided by the user.
//   <i> Default: 0
#ifndef OS_PRIVCNT
 #define OS_PRIVCNT     13
#endif

//   <o>Task stack size [bytes] <20-4096:8><#/4>
//   <i> Set the stack size for tasks which is assigned by the system.
//   <i> Default: 200
#ifndef OS_STKSIZE
 #define OS_STKSIZE     125
#endif

// <q>Check for the stack overflow
// ===============================
// <i> Include the stack checking code for a stack overflow.
// <i> Note that additional code reduces the Kernel performance.
#ifndef OS_STKCHECK
 #define OS_STKCHECK    1
#endif

//   <o>Number of user timers <0-250>
//   <i> Define max. number of user timers that will run at the same time.
//   <i> Default: 0  (User timers disabled)
#ifndef OS_TIMERCNT
 #define OS_TIMERCNT    0
#endif

// </h>
// <h>System Timer Configuration
// =============================
//   <o>RTX Kernel timer <0=> Timer 0 <1=> Timer 1 <2=> Timer 2 <3=> PIT
//   <i> Define the ARM timer used as a system tick timer.
//   <i> Default: PIT
#ifndef OS_TIMER
 #define OS_TIMER       0
#endif

//   <q>Common IRQ System Handler for PIT timer
//   <i> Include a code for Common System Interrupt Handler
//   <i> when a PIT timer is used
#ifndef OS_SYSIRQ
 #define OS_SYSIRQ      0
#endif

//   <o>Timer clock value [Hz] <1-1000000000>
//   <i> Set the timer clock value for selected timer.
//   <i> Default PIT: 3000000  (3 MHz at 48MHz MCLK and prescaler by 16)
//   <I> Default TCx: 1500000  (1.5MHz at 48MHz MCLK and prescaler by 32)
#ifndef OS_CLOCK
 #define OS_CLOCK       1500000
#endif

//   <o>Timer tick value [us] <1-1000000>
//   <i> Set the timer tick value for selected timer.
//   <i> Default: 10000  (10ms)
#ifndef OS_TICK
 #define OS_TICK        10000
#endif

// </h>
// <e>Round-Robin Task switching
// =============================
// <i> Enable Round-Robin Task switching.
#ifndef OS_ROBIN
 #define OS_ROBIN       0
#endif

//   <o>Round-Robin Timeout [ticks] <1-1000>
//   <i> Define how long a task will execute before a task switch.
//   <i> Default: 5
#ifndef OS_ROBINTOUT
 #define OS_ROBINTOUT   5
#endif

// </e>

//------------- <<< end of configuration section >>> -----------------------

// Standard library system mutexes
// ===============================
//  Define max. number system mutexes that are used to protect 
//  the arm standard runtime library. For microlib they are not used.
#ifndef OS_MUTEXCNT
 #define OS_MUTEXCNT    8
#endif

/*----------------------------------------------------------------------------
 *      RTX User configuration part END
 *---------------------------------------------------------------------------*/

#define AIC             AT91C_BASE_AIC                  /* Abbreviations    */
#define SYS             AT91C_BASE_SYS

#if   (OS_TIMER == 0)                                   /* Timer/Counter 0  */
 #define ID_TC          AT91C_ID_TC0
 #define TCx            AT91C_BASE_TC0
#elif (OS_TIMER == 1)                                   /* Timer/Counter 1  */
 #define ID_TC          AT91C_ID_TC1
 #define TCx            AT91C_BASE_TC1
#elif (OS_TIMER == 2)                                   /* Timer/Counter 2  */
 #define ID_TC          AT91C_ID_TC2
 #define TCx            AT91C_BASE_TC2
#elif (OS_TIMER == 3)                                   /* PIT              */
 #define ID_TC          AT91C_ID_SYS
#else
 #error OS_TIMER invalid
#endif

#define OS_TIM_         (1 << ID_TC)                    /*  Interrupt Mask  */
#define OS_TRV          ((U32)(((double)OS_CLOCK*(double)OS_TICK)/1E6)-1)

#if (OS_TIMER < 3) 
 #define OS_TVAL        (TCx->TC_CV & 0x0000FFFF)       /*  Timer Value     */
 #define OS_TOVF        ((TCx->TC_SR >> 4) & 1)         /*  Reload Flag     */
 #define OS_TFIRQ()     AIC->AIC_ISCR  = OS_TIM_;       /*  Force Interrupt */
 #define OS_TIACK()     AIC->AIC_ICCR  = OS_TIM_;       /*  Interrupt Ack   */ \
                        AIC->AIC_EOICR = TCx->TC_SR;
 #define OS_TINIT()     SYS->PMC_PCER  = OS_TIM_;       /*  Initialization  */ \
                        TCx->TC_CCR    = AT91C_TC_CLKEN | AT91C_TC_SWTRG;      \
                        TCx->TC_CMR    = 2 | AT91C_TC_CPCTRG;                  \
                        TCx->TC_RC     = OS_TRV;                               \
                        TCx->TC_IER    = AT91C_TC_CPCS;                        \
                        AIC->AIC_SPU   = (U32)os_def_interrupt;                \
                        AIC->AIC_SVR[ID_TC] = (U32)os_clock_interrupt;         \
                        AIC->AIC_SMR[ID_TC] =                                  \
                          AT91C_AIC_SRCTYPE_INT_POSITIVE_EDGE | 0;
#else
 #define OS_TVAL        (SYS->PITC_PIIR & 0x000FFFFF)   /*  Timer Value     */
 #define OS_TOVF        (SYS->PITC_PISR & 1)            /*  Overflow Flag   */
 #define OS_TFIRQ()     SET_IRQFLAG;                    /*  Force Interrupt */ \
                        AIC->AIC_ISCR  = OS_TIM_;
 #define OS_TIACK()     CLR_IRQFLAG;                    /*  Interrupt Ack   */ \
                        AIC->AIC_ICCR  = OS_TIM_;                              \
                        AIC->AIC_EOICR = SYS->PITC_PIVR;
 #define OS_TINIT()     SYS->PITC_PIMR = OS_TRV |       /*  Initialization  */ \
                          AT91C_PITC_PITIEN | AT91C_PITC_PITEN;                \
                        AIC->AIC_SPU = (U32)os_def_interrupt;                  \
                        AIC->AIC_SVR[ID_TC] = (U32)sys_ctrl_interrupt;         \
                        AIC->AIC_SMR[ID_TC] =                                  \
                          AT91C_AIC_SRCTYPE_INT_POSITIVE_EDGE | 0;
#endif

#define OS_IACK()       AIC->AIC_EOICR = 0;             /* Interrupt Ack    */

#define OS_LOCK()       AIC->AIC_IDCR  = OS_TIM_;       /* Lock             */
#define OS_UNLOCK()     AIC->AIC_IECR  = OS_TIM_;       /* Unlock           */

/* WARNING: Using IDLE mode might cause you troubles while debugging. */
#define _idle_()        SYS->PMC_SCDR = 1;

#if (OS_TIMER == 3 && OS_SYSIRQ == 1)
 BIT force_irq;
 #define SET_IRQFLAG    force_irq = __TRUE
 #define CLR_IRQFLAG    force_irq = __FALSE
#else
 #define SET_IRQFLAG
 #define CLR_IRQFLAG
#endif

/*----------------------------------------------------------------------------
 *      Global Functions
 *---------------------------------------------------------------------------*/

extern void os_clock_interrupt (void);

#if (OS_TIMER == 3 && OS_SYSIRQ == 1)
/*--------------------------- sys_irq_handler -------------------------------*/

__irq void irq_sys_handler (void) {
   /* Common System Interrupt Handler for: DBGU, RSTC, RTT, WDT and PMC  */
   /* system peripheral interrupts.                                      */

   for(;;);
}


/*--------------------------- sys_ctrl_interrupt ----------------------------*/

__asm void sys_ctrl_interrupt (void) {
   /* Common System Interrupt Handler entry. */
        PRESERVE8
        ARM

        STMDB   SP!,{R0}                        ; Save Work Register
        LDR     R0,=__cpp((U32)AT91C_PITC_PISR) ; PIT Status Register
        LDR     R0,[R0]                         ; Read PIT ISR

        TST     R0,#__cpp(AT91C_PITC_PITS)      ; Check for PIT interrupt
        LDMIANE SP!,{R0}                        ; Restore Work Register
        LDRNE   PC,=__cpp(os_clock_interrupt)   ; Jump to RTOS Clock IRQ

        LDR     R0,=__cpp(&force_irq)           ; Check for forced interrupt
        LDRB    R0,[R0]                         ; Read os_psh_flag

        CMP     R0,#__cpp(__TRUE)               ; Check if __TRUE
        LDMIA   SP!,{R0}                        ; Restore Work Register
        LDREQ   PC,=__cpp(os_clock_interrupt)   ; Jump to RTOS Clock IRQ

        LDR     PC,=__cpp(irq_sys_handler)      ; Jump to SYS IRQ Handler
}
#else
 #define sys_ctrl_interrupt    	os_clock_interrupt
#endif


/*--------------------------- os_idle_demon ---------------------------------*/

__task void os_idle_demon (void) {
   /* The idle demon is a system task, running when no other task is ready */
   /* to run. The 'os_xxx' function calls are not allowed from this task.  */

   for (;;) {
   /* HERE: include optional user code to be executed when no task runs.*/
   }
}


/*--------------------------- os_tmr_call -----------------------------------*/

void os_tmr_call (U16 info) {
   /* This function is called when the user timer has expired. Parameter   */
   /* 'info' holds the value, defined when the timer was created.          */

   /* HERE: include optional user code to be executed on timeout. */
}


/*--------------------------- os_stk_overflow -------------------------------*/

void os_stk_overflow (OS_TID task_id) {
   /* This function is called when a stack overflow is detected. Parameter */
   /* 'task_id' holds the id of this task. You can use 'RTX Kernel' dialog,*/
   /* page 'Active Tasks' to check, which task needs a bigger stack.       */

   /* HERE: include optional code to be executed on stack overflow. */
   for (;;);
}


/*----------------------------------------------------------------------------
 *      RTX Configuration Functions
 *---------------------------------------------------------------------------*/

static void os_def_interrupt (void) __irq  {
   /* Default Interrupt Function: may be called when timer ISR is disabled */
   OS_IACK();
}

#include <RTX_lib.c>

/*----------------------------------------------------------------------------
 * end of file
 *---------------------------------------------------------------------------*/

