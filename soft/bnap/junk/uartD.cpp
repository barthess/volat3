//============================================================================
// ��������� UARTD. ����������� GPS ���������.  
//============================================================================

#include "errorhandler.h"
#include "pio/pio.h"
#include "aic/aic.h"
#include "gps.h"
#include "uartD.h"
//----------------------------------------------------------------------------

__irq void UARTD_IRQHandler(void);
bool UARTD_InputDataHandler(int iInput);
bool print_text(const char *pstrz);
//----------------------------------------------------------------------------

C_UARTD::C_UARTD(bool boMaster) : C_USART( boMaster)
{ 
  pInputDataHandler = UARTD_InputDataHandler; 
  usart = (AT91PS_USART)AT91C_BASE_DBGU; 
  const Pin Pins[] = {PINS_UARTD};  
  PIO_Configure(Pins, PIO_LISTSIZE(Pins)); // ��������� ������ � UARTD  
  // ������������ ��������� �� UARTD
  AIC_ConfigureIT(AT91C_ID_SYS, AT91C_AIC_SRCTYPE_INT_HIGH_LEVEL, UARTD_IRQHandler);
  AIC_EnableIT(AT91C_ID_SYS);
}
//----------------------------------------------------------------------------

//---------------------------------------------------------------------------
// ���������� ���������� 
//---------------------------------------------------------------------------
__irq void UARTD_IRQHandler() {
  C_UARTD* UART = &UARTD; 

  // ���������� �� ���������
  if(0 != (AT91C_US_RXRDY & UART->usart->US_IMR)){
    if(0 != (AT91C_US_RXRDY & UART->usart->US_CSR)){
      // ���� ����� ��������, �� �������� ��������
      if(UARTD_InputDataHandler(UART->usart->US_RHR)){
        if(UART->boMaster) UART->ReceiverStop();
        isr_evt_set(RX_REDY, idtGPS_Rx);    
      }
    }
  }
  AT91C_BASE_AIC->AIC_EOICR = 0;
}
//---------------------------------------------------------------------------

///---------------------------------------------------------------------------
/// ������������ ��������� �� ��� �������. ������� ��� ��������� 
/// �� ��� ����� ������, � �� ��� ��������� �������.
/// ������������ ��������: ������� ���������� true ���� ���������� ��������� 
/// �����,  ��������� �������� � ���������� ���������
///---------------------------------------------------------------------------
bool UARTD_InputDataHandler(int iInput)
{
  return GPS.GetData(0xFF & iInput);
}
//----------------------------------------------------------------------------


