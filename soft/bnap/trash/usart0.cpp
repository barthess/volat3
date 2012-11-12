///===========================================================================
// Интерфейс USART0. Используется для подключение Crypto-карты.
//===========================================================================

#include <string.h>
#include "pio/pio.h"
#include "aic/aic.h"
#include "linkpc.h"
#include "usart0.h"
//----------------------------------------------------------------------------

bool USART0_InputDataHandler(int iInput);
//----------------------------------------------------------------------------

C_USART0::C_USART0(bool boMaster) : C_USART( boMaster)
{ 
  pInputDataHandler = USART0_InputDataHandler; 
  usart = AT91C_BASE_US0; 
  *AT91C_PMC_PCER = 0x1 << AT91C_ID_US0;   // Включаю CLK для USART0
  const Pin Pins[] = {PINS_USART0};  
  PIO_Configure(Pins, PIO_LISTSIZE(Pins)); // Подключаю выводы к USART0
  // Конфигурирую прерывния от USART0
  AIC_ConfigureIT(AT91C_ID_US0, AT91C_AIC_SRCTYPE_INT_HIGH_LEVEL, USART0_IRQHandler);
  AIC_EnableIT(AT91C_ID_US0);
}
//----------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Ожидает поступления подтверждения команды и отправляет его РС 
//---------------------------------------------------------------------------
__task void Task_USART0(void) 
{
    USART0.TaskTxRx(&LinkPC);
}
//----------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Обработчик прерываний 
//---------------------------------------------------------------------------
__irq void USART0_IRQHandler() {
  C_USART0* UART = &USART0; 
  OS_TID idtRx = idtLinkPC_Rx;

  // Прерывания от приемника
  if(0 != (AT91C_US_RXRDY & UART->usart->US_IMR)){
    if(0 != (AT91C_US_RXRDY & UART->usart->US_CSR)){
      // Если прием закончен, то выключаю приемник
      if(USART0_InputDataHandler(UART->usart->US_RHR)){
        if(UART->boMaster) UART->ReceiverStop();
        isr_evt_set (RX_REDY, idtRx);    
      }
    }
  }

  // Прерывания от передатчика
  if(0 != (AT91C_US_TXRDY & UART->usart->US_IMR)){
    if(0 != (AT91C_US_TXRDY & UART->usart->US_CSR)){
      UART->IRQTransmitterDisable();       
      isr_evt_set (TX_REDY, idtUSART0);
    }
  }
  
  AT91C_BASE_AIC->AIC_EOICR = 0;
}
//----------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Используется указатель на эту функцию. Поэтому она оформлена 
// не как метод класса, а как отдельная функция.
// Возвращаемое значение: функция возвращает true если необходимо закончить 
// прием,  выключить приемник и освободить интерфейс
//----------------------------------------------------------------------------
bool USART0_InputDataHandler(int iInput)
{
  return LinkPC.GetData(iInput & 0xFF);
}
//----------------------------------------------------------------------------
