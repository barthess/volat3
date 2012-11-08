//============================================================================
//                                        
//============================================================================

///---------------------------------------------------------------------------
/// C_UART - базовый класс 
///----------------------------------------------------------------------------

#include "usart.h"
//----------------------------------------------------------------------------

bool print_text(const char *pstrz);
//----------------------------------------------------------------------------

void C_UART::Configure(unsigned int mode, unsigned int baudrate, unsigned int masterClock)
{
  // Reset and disable receiver & transmitter
  usart->US_CR = AT91C_US_RSTRX | AT91C_US_RSTTX | AT91C_US_RXDIS | AT91C_US_TXDIS;
  // Configure mode
  usart->US_MR = mode;

  // Configure baudrate 
  // Asynchronous, no oversampling
  if (((mode & AT91C_US_SYNC) == 0) && ((mode & AT91C_US_OVER) == 0))
    usart->US_BRGR = (masterClock / baudrate) / 16;
}
//---------------------------------------------------------------------------- 

void C_UART::TaskTxRx(I_UpLevel* UpLevel)
{
  if(!boMaster) ReceiverStart();

  for(;;){
    // Интерфейс  занят. Прием подтверждения
    if(boMaster) {
      if(IsWaitResponse()){
        OS_RESULT result = os_evt_wait_or(RX_REDY, TIMEOUT_USART2);
  
        // Отправляю сообщение с принятым подтверждением
        if(OS_R_EVT == result){
          UpLevel->SendMsgResponse(pMailboxSource, ucIDSource, MSG_RESPONSE);
        } 
  
        // Истекло время ожидания. Отправляю сообщение об ошибке.
        else {
          ReceiverStop();
          UpLevel->SendMsgResponse(pMailboxSource, ucIDSource, MSG_TIMEOUT);
        }
        // Освобождаю интерфейс
        SetStateWait();
      }
    }

    // Ожидаю начало очередного цикла передачи
    os_mbx_wait(MailBox, (void **)&(pOutputMsg), 0xFFFF); 
    // Диагностика
    if(boDiagnosticsTx) print_text(pOutputMsg->PointerStr()); 
    SetStateWaitTransmitEnd(pOutputMsg->GetIDSource(), pOutputMsg->GetPointerMailbox());
    usOutputSimbolCount = 0;   // Очищаю счетчик переданных байтов      
    IRQTransmitterEnabled();   // Разрешаю прерывания от передатчика

    for(;;){
      os_evt_wait_or (TX_REDY, 0xffff);
      // Прерывания от передатчика
      if(0 != (AT91C_US_TXRDY & usart->US_CSR)){
        // Передаю очередной байт данных
        if(usOutputSimbolCount < pOutputMsg->GetLength()){
          usart->US_THR = pOutputMsg->PointerStr()[usOutputSimbolCount++]; 
          IRQTransmitterEnabled();   // Разрешаю прерывания от передатчика
        }
        else {
          // Все передано. Запрещаю прерывания от передатчика
          IRQTransmitterDisable();       
             
          if(boMaster){
            // Если команда с подтверждением, то разрешаю работу приемника
            if(MSG_CMDWITHRESPONSE == pOutputMsg->GetFlags()){
              SetStateWaitResponse();
              ReceiverStart();
            }     
            // В противном случае освобождаю интерфейс 
            else SetStateWait();
          }

          // Освобождаю память, занимаемую переданным сообщением
          pOutputMsg->Release();  
          break;
        }
      }
    }
  }   
}
//---------------------------------------------------------------------------
