//===========================================================================
// Интерфейс USART1. Связь с GSM-модемом.
//===========================================================================

#include "pio/pio.h"
#include "aic/aic.h"
#include "GSM.h"
#include "usart1.h"
//----------------------------------------------------------------------------

__irq void USART1_IRQHandler(void);
bool USART1_InputDataHandler(int iInput);
//----------------------------------------------------------------------------

C_USART1::C_USART1(bool boMaster) : C_USART( boMaster)
{ 
  pInputDataHandler = USART1_InputDataHandler; 
  usart = AT91C_BASE_US1; 
  *AT91C_PMC_PCER = 0x1 << AT91C_ID_US1;   // Включаю CLK для USARTx
  const Pin Pins[] = {PINS_USART1};  
  PIO_Configure(Pins, PIO_LISTSIZE(Pins)); // Подключаю выводы к USARTx
  // Конфигурирую  прерывния от USARTx
  AIC_ConfigureIT(AT91C_ID_US1, AT91C_AIC_SRCTYPE_INT_HIGH_LEVEL, USART1_IRQHandler);
  AIC_EnableIT(AT91C_ID_US1);
}
//----------------------------------------------------------------------------

//---------------------------------------------------------------------------
//  
//---------------------------------------------------------------------------
__task void Task_USART1(void) 
{
  USART1.TaskTxRx(&GSM);
}
//----------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Обработчик прерываний 
//---------------------------------------------------------------------------
__irq void USART1_IRQHandler(void) { 

  C_USART1* UART = &USART1; 

  // Прерывания от приемника
  if(0 != (AT91C_US_RXRDY & UART->usart->US_IMR)){
    if(0 != (AT91C_US_RXRDY & UART->usart->US_CSR)){
      // Если прием закончен, то выключаю приемник
      if(USART1_InputDataHandler(UART->usart->US_RHR)){
        if(UART->boMaster) UART->ReceiverStop();
        isr_evt_set (RX_REDY, idtUSART1);    
      }
    }
  }

  // Прерывания от передатчика
  if(0 != (AT91C_US_TXRDY & UART->usart->US_IMR)){
    if(0 != (AT91C_US_TXRDY & UART->usart->US_CSR)){
      UART->IRQTransmitterDisable();       
      isr_evt_set (TX_REDY, idtUSART1);
    }
  }
  
  AT91C_BASE_AIC->AIC_EOICR = 0;
}
//----------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Используется указатель на эту функцию. Поэтому она оформлена 
// не как метод класса, а не как отдельная функция.
// Возвращаемое значение: функция возвращает true если необходимо закончить 
// прием,  выключить приемник и освободить интерфейс
//---------------------------------------------------------------------------
bool USART1_InputDataHandler(int iInput)
{
  return GSM.GetData(iInput & 0xFF);
}
//----------------------------------------------------------------------------

