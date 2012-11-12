//============================================================================
// TINS5                                    
//============================================================================

#include <RTL.h>
#include <stdio.h>
#include <string.h>

#include "sysconfig.h"
#include "pio/pio.h"
#include "aic/aic.h"
#include "errorhandler.h"
#include "flashbuf.h"
#include "gps.h"
#include "gsm.h"
#include "linkpc.h"
#include "management.h"
#include "msg.h"
#include "statehandler.h"
#include "usart0.h"
#include "usart1.h"
#include "usart2.h"
#include "uartD.h"
#include "twi.h"
#include "dc.h"
#include "rtc1338.h"
#include "mmc_spi.h"
#include "lcd.h"
#include "terminal.h"
#include "parameters.h"
#include "profiler.h"
//----------------------------------------------------------------------------

const bool SLAVE = false;
const bool MASTER = true;

char pBuffer[MMC_BLOCKSIZE];
OS_TID idtLinkPC_Rx;
OS_TID idtLinkPC_Tx;
OS_TID idtGPS_Rx;
OS_TID idtUSART0;
OS_TID idtUSART1;
OS_TID idtUSART2;
OS_TID idtUARTD;
OS_TID idtDC;
OS_TID idtTerminal;

C_UARTD       UARTD(SLAVE);       // GPS
C_USART0      USART0(SLAVE);      // PC
C_USART1      USART1(MASTER);     // GSM
C_USART2      USART2(MASTER);     // LCD

C_GSM         GSM(&USART1);
C_LCD         LCD(&USART2);
C_Terminal    Terminal;

C_StateHandler StateHandler_(AT91C_BASE_ADC0);
C_GPS         GPS(&UARTD);
C_LinkPC      LinkPC(&USART0);

C_RTC1338     RTC1338;
C_MMCBuffer   MMCBuffer(pBuffer, &RTC1338, RTC1338_FLASHPOINTER, MMC_STARTBLOCK);
C_PacketQueue PacketQueue(&MMCBuffer, &DC);
C_DC          DC;

C_StateHandler * StateHandler;
C_Parameters   * Parameters;
//----------------------------------------------------------------------------

extern char strz80[80];
//----------------------------------------------------------------------------

void Sleep(U32 uiTime);
void EFCInit(void);
void Restart(void);
//----------------------------------------------------------------------------


#define D_PROFILER
#ifdef D_PROFILER
  C_Profiler   Profiler;
#endif
//----------------------------------------------------------------------------

#if STDOUT == USART0_OUT
  FILE * USART0out =  stdout;
#else 
  FILE * USART0out = (FILE *)USART0_OUT;
#endif

#if STDOUT == USART1_OUT
  FILE * USART1out =  stdout;
#else 
  FILE * USART1out = (FILE *)USART1_OUT;
#endif

#if STDOUT == USART2_OUT
  FILE * USART2out =  stdout;
#else 
  FILE * USART2out = (FILE *)USART0_OUT;
#endif

#if STDOUT == UARTD_OUT
  FILE * UARTDout =  stdout;
#else 
  FILE * UARTDout = (FILE *)UARTD_OUT;
#endif

//----------------------------------------------------------------------------

//const Pin pinStrob = PIN_STROB;  
bool boMMC_REDY = false;
//----------------------------------------------------------------------------

const int UPSIZE = 128;
const int UPSIZE_1 = 512;
// Стеки задач
static U64 Stek_ErrorHandler[(80 + UPSIZE)/8];    
static U64 Stek_TWI[(56 + UPSIZE_1)/8];
static U64 Stek_LinkPC_Rx[(224 + UPSIZE)/8];
static U64 Stek_USART0[(72 + UPSIZE)/8];
static U64 Stek_USART2[(272 + UPSIZE)/8];
static U64 Stek_Terminal[(408 + UPSIZE_1)/8];         // ?
static U64 Stek_GPS_Rx[(528 + UPSIZE_1)/8];
static U64 Stek_Management[(616 + UPSIZE_1)/8];      // ?
static U64 Stek_USART1[(272 + UPSIZE)/8];
static U64 Stek_InputState[(28 + UPSIZE)/8];
static U64 Stek_DC[(408 + UPSIZE)/8];          // ?
//----------------------------------------------------------------------------

__task void InitTasks(void)
{
  os_dly_wait(100); 
  idtErrorHandler = os_tsk_create_user (Task_ErrorHandler, 2, &Stek_ErrorHandler, sizeof(Stek_ErrorHandler));
  idtTWI          = os_tsk_create_user (Task_TWI,    2, &Stek_TWI, sizeof(Stek_TWI));
  idtLinkPC_Rx    = os_tsk_create_user (Task_LinkPC_Rx, 2, &Stek_LinkPC_Rx, sizeof(Stek_LinkPC_Rx));
  idtUSART0       = os_tsk_create_user (Task_USART0, 2, &Stek_USART0, sizeof(Stek_USART0));
  idtUSART2       = os_tsk_create_user (Task_USART2, 2, &Stek_USART2, sizeof(Stek_USART2));
  idtTerminal     = os_tsk_create_user (Task_Terminal, 2, &Stek_Terminal, sizeof(Stek_Terminal));
  idtGPS_Rx       = os_tsk_create_user (Task_GPS_Rx, 2, &Stek_GPS_Rx, sizeof(Stek_GPS_Rx));  
  idtManagement   = os_tsk_create_user (Task_Management, 2, &Stek_Management, sizeof(Stek_Management));
  idtUSART1       = os_tsk_create_user (Task_USART1, 2, &Stek_USART1, sizeof(Stek_USART1));
  idtInputState   = os_tsk_create_user (Task_InputState,2, &Stek_InputState, sizeof(Stek_InputState));
  idtDC           = os_tsk_create_user (Task_DC, 2, &Stek_DC, sizeof(Stek_DC));

  switch(AT91C_BASE_RSTC->RSTC_RSR & AT91C_RSTC_RSTTYP)
  {
    case AT91C_RSTC_RSTTYP_GENERAL:
      print_text("\rЗапуск. [GENERAL]   ");
      break;
    case AT91C_RSTC_RSTTYP_WAKEUP:
      print_text("\rЗапуск. [WAKEUP]   ");
      break;
    case AT91C_RSTC_RSTTYP_WATCHDOG:   
      print_text("\rЗапуск. [WATCHDOG]   ");
      break;
    case AT91C_RSTC_RSTTYP_SOFTWARE:
      print_text("\rЗапуск. [SOFTWARE]   ");
      break;
    case AT91C_RSTC_RSTTYP_USER:
      print_text("\rЗапуск. [USER]   ");
      break;
    default:
      print_text("\rЗапуск. [??]   ");
      break;
  }

  RTC1338.TimeToStr(strz80, sizeof(strz80)); 
  print_text(strz80);  print_text("\r");


  // Инициализации ММС карты
  U16 usError;
  usError = MMC_Init();
  if(NO_ERROR == usError) boMMC_REDY = true;
  else {
    boMMC_REDY = false; 
    print_text("\rОшибка инициализации ММС карты\r");
  }

//  if(boMMC_REDY){
//    usError = MMCBuffer.Init();
//    if(NO_ERROR != usError){ 
//      boMMC_REDY = false;
//      print_text("\rОшибка инициализации ММС буфера\r");
//    }
//    else print_text("\rИнициализация завершена\r");
//  }

  print_text(">\r");
  os_tsk_delete_self(); 
}
#include "time.h"
//----------------------------------------------------------------------------

U8 InReport;      // HID Input Report 
U8 OutReport;     // HID Out Report 
unsigned char pucTxBuf[16];
unsigned char ucTxCont = 16;
//----------------------------------------------------------------------------

extern  "C" void GetInReport (void) {
  int i;
  static unsigned short usCount = 0;


  InReport = usCount++;
  for(i = 0; i < sizeof(pucTxBuf); ++i)
    pucTxBuf[i] = i + 1;
  *((unsigned short *)pucTxBuf) = usCount;
  *((unsigned short *)(pucTxBuf + 2)) = usCount;
}
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
//  Set HID Output Report <- OutReport
//----------------------------------------------------------------------------
extern  "C" void SetOutReport (void) {

//  if (OutReport & 0x01) pPIOA->PIO_CODR = LED1; else pPIOA->PIO_SODR = LED1;
//  if (OutReport & 0x02) pPIOA->PIO_CODR = LED2; else pPIOA->PIO_SODR = LED2;
//  if (OutReport & 0x04) pPIOA->PIO_CODR = LED3; else pPIOA->PIO_SODR = LED3;
//  if (OutReport & 0x08) pPIOA->PIO_CODR = LED4; else pPIOA->PIO_SODR = LED4;
}
//----------------------------------------------------------------------------


//extern  "C" void  USB_Init(void);
//extern  "C" void  USB_Connect    (bool  con);

//volatile long lll;
int main(void)
{  
//  lll = 1;
  // Включаю CLK для PIOA и PIOB
  *AT91C_PMC_PCER = (1 << AT91C_ID_PIOA) | (1 << AT91C_ID_PIOB); 

  // Инициализирую таймер. Используется для формирования системного времени.
  InitPIT();

  // Инициализирую контроллер FLASH памяти 
  EFCInit();

  // Создаю таблицу параметров
  Parameters = CreatParameters();

  // Инициализирую обработчик состояния системы
  StateHandler = &StateHandler_;

  // Инициализирую USART0 (Связь с РС)
  USART0.Configure(AT91C_US_CHRL_8_BITS | AT91C_US_PAR_NONE, 115200);
  USART0.TransmitterEnabled();

  // Инициализирую USART1 (GSM модем)
  USART1.Configure(AT91C_US_CHRL_8_BITS | AT91C_US_PAR_NONE, 115200);
  USART1.TransmitterEnabled();

  //  Конфигурирую CS_LCD
  const Pin Pins[] = {PIN_CS_LCD};  
  PIO_Configure(Pins, PIO_LISTSIZE(Pins)); 
  // Инициализирую USART2 (LCD)
  USART2.Configure(AT91C_US_CHRL_8_BITS | AT91C_US_PAR_NONE, 9600);
  USART2.TransmitterEnabled();

  // Конфигурирую GPS_RESET
  const Pin Pins1[] = {PIN_GPS_RESET};  
  PIO_Configure(Pins1, PIO_LISTSIZE(Pins1)); 
  // Инициализирую UARTD (GPS)
  UARTD.Configure(AT91C_US_CHRL_8_BITS | AT91C_US_PAR_NONE, 9600);
  UARTD.TransmitterEnabled();

 
//  // Конфигурирую вывод для строба 
//  const Pin Pins[] = {pinStrob};  
//  PIO_Configure(Pins, PIO_LISTSIZE(Pins)); 

    
//  // Разрешаю внешний сброс и устанавливаю длительность сброса 
//  // 32 "медленных" такта. Примерно 1 мс.
//  *AT91C_RSTC_RMR = 0xA5000000 + (4 << 8) + AT91C_RSTC_URSTEN;
  *AT91C_RSTC_RMR = 0xA5000000 + AT91C_RSTC_URSTEN;

//  memset(pucTxBuf, 0, sizeof(pucTxBuf));
//  USB_Init();                               /* USB Initialization */
//  USB_Connect(true);                        /* USB Connect */

  // Запускаю задачи
  os_sys_init_prio (InitTasks, 10);

  return 0;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

extern "C" int sendchar (int ch) {
  return ch;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

extern "C" int fputc(int ch, FILE *f) {
  C_Msg *pMsg = NewMsg(0, ID_NULL, NULL, 0, 1);
  if(NULL == pMsg ) return ch;
  pMsg->PointerStr()[0] = ch;
  if(f == USART0out){ 
  os_mbx_send(USART0.MailBox, pMsg, 0xFFFF); return ch;}
  if(f == USART1out){ 
  os_mbx_send(USART1.MailBox, pMsg, 0xFFFF); return ch;}
  if(f == USART2out){ 
  os_mbx_send(USART2.MailBox, pMsg, 0xFFFF); return ch;}
  if(f == UARTDout) { 
  os_mbx_send(UARTD.MailBox,  pMsg, 0xFFFF); return ch;}
  pMsg->Release();
  return ch;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


 //  // Тестирование TWI
//  const U8 TWI_SIZE =56;
//  for(U16 usCount = 0; usCount < 10000; ++usCount){
//
//    for(U8 uc = 0; uc < TWI_SIZE; ++uc)
//      pucWriteForTWI[uc] = uc + usCount;
//
//    Profiler.SaveTime(1000);
//    usError = RTC1338.Write(8, pucWriteForTWI, TWI_SIZE);
//    Profiler.SaveTime(2000 + usError);
//
//    usError = RTC1338.Read(8, pucReadFromTWI, TWI_SIZE);
//    Profiler.SaveTime(3000 + usError);
//
//    for(usError = 0; usError < TWI_SIZE; ++usError){
//      if(pucWriteForTWI[usError] != pucReadFromTWI[usError]){
//        break;
//      }
//    }
//    Profiler.SaveTime(4000 + usError);
//  }

// usError = 0;
// for(;;){
//   print_text("\rОшибка инициализации ММС карты\r");
//   ++usError;
//   if(10000 == usError)
//     usError = 0;
// }
