//============================================================================
// Драйвер I2C интерфейса                                     
//============================================================================

#include <RTL.h>
#include "pio/pio.h"
#include "aic/aic.h"
#include "errorhandler.h"
#include "twi.h"
#include "profiler.h"
//----------------------------------------------------------------------------

#define TWI_FREE 0x1
//----------------------------------------------------------------------------

C_TWI TWI;
OS_TID idtTWI;
//----------------------------------------------------------------------------

C_TWI::C_TWI() : pMsg(NULL), usLength(0), usCount(0), boBusy(false), pcBuf(NULL) 
{ 
  os_mbx_init(MailBox, sizeof(MailBox));
  *AT91C_PMC_PCER = 0x1 << AT91C_ID_TWI;   // Включаю CLK для TWI
  const Pin Pins[] = {PINS_TWI};  
  PIO_Configure(Pins, PIO_LISTSIZE(Pins)); // Подключаю выводы к TWI
  /// Конфигурирую прерывния
  AIC_ConfigureIT(AT91C_ID_TWI, AT91C_AIC_SRCTYPE_INT_HIGH_LEVEL, TWI_IRQHandler);
  AIC_EnableIT(AT91C_ID_TWI);
}
//----------------------------------------------------------------------------

//---------------------------------------------------------------------------
// В области данных сообщения содержится  блок параметров, который  
// Определяют Режим Работы Twi интерфейса и выполняемую операцию
//---------------------------------------------------------------------------
void C_TWI::Init(void){
  // В области данных сообщения содержится  блок параметров, который  
  // определяют режим работы TWI интерфейса и выполняемую операцию
  AT91C_BASE_TWI->TWI_CR = AT91C_TWI_SWRST;
  S_TWIMsg * pTWIMsg = (S_TWIMsg *)pMsg->PointerStr();

  // Устанавливаю частоту
  // Tlow = (CLDIV * 2^CKDIV + 3)* Tmck  Thigh = (CHDIV * 2^CKDIV + 3)* Tmck
  // Для CKDIV = 0, CLDIV = CHDIV = CDIV,  usSCLFrequency в КГц.
  // Tlow = Thigh = 1/(2 * usSCLFrequency * 1000) = (CLDIV + 3)/BOARD_MCK
  // ==> CLDIV = BOARD_MCK/2/usSCLFrequency - 3
  U8 ucCDIV = BOARD_MCK/2000/pTWIMsg->usSCLFrequency - 3;
//  AT91C_BASE_TWI->TWI_CWGR = (1 << 17) + (ucCDIV << 8) + ucCDIV;
  AT91C_BASE_TWI->TWI_CWGR = (ucCDIV << 8) + ucCDIV;

  // Разрешаю режим ведущего
  AT91C_BASE_TWI->TWI_CR = AT91C_TWI_MSDIS;
  AT91C_BASE_TWI->TWI_CR = AT91C_TWI_MSEN;

  // Устанавливаю slave адрес, размер внутреннего адреса и направление передачи
  AT91C_BASE_TWI->TWI_MMR = (pTWIMsg->ucAddresSlave << 16) +  (pTWIMsg->ucAddressSizeAndOper << 8);
  // Устанавливаю внутренние адреса
  AT91C_BASE_TWI->TWI_IADR = pTWIMsg->uiInternalAddess;

  // Определяю размер данных
  usLength = pMsg->GetLength() - sizeof(S_TWIMsg);
  // Очищаю счетчик принятых/переданных байтов
  usCount = 0; 
  // Устанавливаю указатель на начало данных
  pcBuf = pMsg->PointerStr() + sizeof(S_TWIMsg);
}
//----------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Забирает сообщение из почтового ящика, конфигурирует TWI и разрешает прерывания 
//---------------------------------------------------------------------------
static volatile int i;
__task void Task_TWI(void) 
{
  for(;;){
    // Интерфейс занят. 
    if(TWI.IsBusy()) {
      U16 usResult = os_evt_wait_or(TX_REDY | RX_REDY | TWI_ERR, TIMEOUT_TWI);
      if(OS_R_EVT == usResult) {
        U16 usFlags = os_evt_get();

        //  Завешаю передачу
        if(TX_REDY == usFlags) { 
          TWI.pMsg->Release(); 
          TWI.pMsg = NULL;
        } 

        //  Завершаю прием
        else if(RX_REDY == usFlags) { 
                TWI.pMsg->SetFlags(MSG_RESPONSE); // Меняю тип сообщения
                os_mbx_send (TWI.pMsg->GetPointerMailbox(), TWI.pMsg, TIMEOUT_TWI);
              }
              else goto ERROR;  // TWI_ERR
      }

      // Ошибка. TimeOut
      else {
ERROR:
        TWI.IRQDisable();

        // Выполнялась запись. Уничтожаю полученное сообщение
        if(0 == (AT91C_BASE_TWI->TWI_MMR & AT91C_TWI_MREAD)) {       
          TWI.pMsg->Release(); 
          TWI.pMsg = NULL;
        }

        // Выполнялась чтение. Отправляю сообщение об ошибке
        else {
          TWI.pMsg->SetFlags(MSG_ERROR);  // Меняю тип сообщения
          os_mbx_send (TWI.pMsg->GetPointerMailbox(), TWI.pMsg, TIMEOUT_TWI);
        }
      } 

      // В любом случае очищаю флаги и освобождаю интерфейс
      os_evt_wait_or(0xFFFF, 0);
      TWI.SetFree();
    }   

    // Ожидаю начало очередной операции передачи данных
    else {
      os_mbx_wait(TWI.MailBox, (void **)&(TWI.pMsg), 0xFFFF); 
      TWI.SetBusy();
      TWI.Init();
      // Если передается или принимается один байт, то устанавливаю одновременно два флага
      if(1 == TWI.usLength) AT91C_BASE_TWI->TWI_CR =  AT91C_TWI_START | AT91C_TWI_STOP;
      else AT91C_BASE_TWI->TWI_CR = AT91C_TWI_START;  
        
      // Операция запись.
      if(0 == (AT91C_BASE_TWI->TWI_MMR & AT91C_TWI_MREAD)){      
        AT91C_BASE_TWI->TWI_THR = TWI.pcBuf[0];
        TWI.usCount = 1;
        AT91C_BASE_TWI->TWI_IER = AT91C_TWI_TXRDY;
      }

      // Операция чтение.
      else { 
        i = AT91C_BASE_TWI->TWI_RHR;
        AT91C_BASE_TWI->TWI_IER = AT91C_TWI_RXRDY;
      }
    }
  }
}
//----------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Обработчик прерываний 
//---------------------------------------------------------------------------
__irq void TWI_IRQHandler(void) { 
  U32 uiIMR = AT91C_BASE_TWI->TWI_IMR;
  U32 uiSR  = AT91C_BASE_TWI->TWI_SR;

  // Завершение операций
  if(0 != (AT91C_TWI_TXCOMP & uiIMR & uiSR)){
    AT91C_BASE_TWI->TWI_IDR = AT91C_TWI_TXCOMP;
    // Завершение операции записи
    if(0 == (AT91C_BASE_TWI->TWI_MMR & AT91C_TWI_MREAD))  isr_evt_set(TX_REDY, idtTWI);  
    // Завершение операции чтения
    else  isr_evt_set(RX_REDY, idtTWI);  
  }

  // Чтение
  if(0 != (AT91C_TWI_RXRDY & uiIMR & uiSR)){ 
    // При чтении данные помещаю в сообщении в котором поступила команда
    TWI.pcBuf[TWI.usCount++] = AT91C_BASE_TWI->TWI_RHR;
    if(TWI.usCount == TWI.usLength){
      if(1 != TWI.usLength) AT91C_BASE_TWI->TWI_CR = AT91C_TWI_STOP;
      AT91C_BASE_TWI->TWI_IDR = AT91C_TWI_RXRDY;
      AT91C_BASE_TWI->TWI_IER = AT91C_TWI_TXCOMP;
    }
  }

  // Запись
  if(0 != (AT91C_TWI_TXRDY & uiIMR & uiSR)){ 
    if(TWI.usCount == TWI.usLength){
      // Все передано 
      if(1 != TWI.usLength) AT91C_BASE_TWI->TWI_CR = AT91C_TWI_STOP;
      AT91C_BASE_TWI->TWI_IDR = AT91C_TWI_TXRDY;
      AT91C_BASE_TWI->TWI_IER = AT91C_TWI_TXCOMP;
    }
    else  
      AT91C_BASE_TWI->TWI_THR = TWI.pcBuf[TWI.usCount++];
  }

  /// Ошибки TWI
  if( (0 != (AT91C_TWI_OVRE & uiIMR & uiSR)) || 
      (0 != (AT91C_TWI_UNRE & uiIMR & uiSR)) ||
      (0 != (AT91C_TWI_NACK & uiIMR & uiSR)) ) 
  {
    TWI.IRQDisable();
    isr_evt_set(TWI_ERR, idtTWI);  
   }

  AT91C_BASE_AIC->AIC_EOICR = 0;
}
//----------------------------------------------------------------------------
