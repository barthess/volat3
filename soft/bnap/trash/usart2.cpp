//===========================================================================
// ��������� USART2. ����� � �������� �����������                                       
//===========================================================================

#include "pio/pio.h"
#include "aic/aic.h"
#include "lcd.h"
#include "usart2.h"
//----------------------------------------------------------------------------

__irq void USART2_IRQHandler(void);
bool USART2_InputDataHandler(int iInput);
//----------------------------------------------------------------------------

C_USART2::C_USART2(bool boMaster) : C_USART( boMaster)
{ 
  pInputDataHandler = USART2_InputDataHandler; 
  usart = AT91C_BASE_US2; 
  *AT91C_PMC_PCER = 0x1 << AT91C_ID_US2;   // ������� CLK ��� USARTx
  const Pin Pins[] = {PINS_USART2};  
  PIO_Configure(Pins, PIO_LISTSIZE(Pins)); // ��������� ������ � USARTx
  // ������������  ��������� �� USARTx
  AIC_ConfigureIT(AT91C_ID_US2, AT91C_AIC_SRCTYPE_INT_HIGH_LEVEL, USART2_IRQHandler);
  AIC_EnableIT(AT91C_ID_US2);
}
//----------------------------------------------------------------------------

//---------------------------------------------------------------------------
//  
//---------------------------------------------------------------------------
__task void Task_USART2(void) 
{
  USART2.TaskTxRx(&LCD);
}
//----------------------------------------------------------------------------

//---------------------------------------------------------------------------
// ���������� ���������� 
//---------------------------------------------------------------------------
__irq void USART2_IRQHandler(void) { 
  C_USART2* UART = &USART2; 

  // ���������� �� ���������
  if(0 != (AT91C_US_RXRDY & UART->usart->US_IMR)){
    if(0 != (AT91C_US_RXRDY & UART->usart->US_CSR)){
      // ���� ����� ��������, �� �������� ��������
      if(USART2_InputDataHandler(UART->usart->US_RHR)){
        if(UART->boMaster) UART->ReceiverStop();
        isr_evt_set (RX_REDY, idtUSART2);    
      }
    }
  }

  // ���������� �� �����������
  if(0 != (AT91C_US_TXRDY & UART->usart->US_IMR)){
    if(0 != (AT91C_US_TXRDY & UART->usart->US_CSR)){
      UART->IRQTransmitterDisable();       
      isr_evt_set (TX_REDY, idtUSART2);
    }
  }
  
  AT91C_BASE_AIC->AIC_EOICR = 0;
}
//----------------------------------------------------------------------------

//---------------------------------------------------------------------------
// ������������ ��������� �� ��� �������. ������� ��� ��������� 
// �� ��� ����� ������, � �� ��� ��������� �������.
// ������������ ��������: ������� ���������� true ���� ���������� ��������� 
// �����,  ��������� �������� � ���������� ���������
//---------------------------------------------------------------------------
bool USART2_InputDataHandler(int iInput)
{
  return LCD.GetData(iInput & 0xFF);
}
//----------------------------------------------------------------------------

