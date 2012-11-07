//============================================================================
//                                        
//============================================================================

///---------------------------------------------------------------------------
/// C_UART - ������� ����� 
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
    // ���������  �����. ����� �������������
    if(boMaster) {
      if(IsWaitResponse()){
        OS_RESULT result = os_evt_wait_or(RX_REDY, TIMEOUT_USART2);
  
        // ��������� ��������� � �������� ��������������
        if(OS_R_EVT == result){
          UpLevel->SendMsgResponse(pMailboxSource, ucIDSource, MSG_RESPONSE);
        } 
  
        // ������� ����� ��������. ��������� ��������� �� ������.
        else {
          ReceiverStop();
          UpLevel->SendMsgResponse(pMailboxSource, ucIDSource, MSG_TIMEOUT);
        }
        // ���������� ���������
        SetStateWait();
      }
    }

    // ������ ������ ���������� ����� ��������
    os_mbx_wait(MailBox, (void **)&(pOutputMsg), 0xFFFF); 
    // �����������
    if(boDiagnosticsTx) print_text(pOutputMsg->PointerStr()); 
    SetStateWaitTransmitEnd(pOutputMsg->GetIDSource(), pOutputMsg->GetPointerMailbox());
    usOutputSimbolCount = 0;   // ������ ������� ���������� ������      
    IRQTransmitterEnabled();   // �������� ���������� �� �����������

    for(;;){
      os_evt_wait_or (TX_REDY, 0xffff);
      // ���������� �� �����������
      if(0 != (AT91C_US_TXRDY & usart->US_CSR)){
        // ������� ��������� ���� ������
        if(usOutputSimbolCount < pOutputMsg->GetLength()){
          usart->US_THR = pOutputMsg->PointerStr()[usOutputSimbolCount++]; 
          IRQTransmitterEnabled();   // �������� ���������� �� �����������
        }
        else {
          // ��� ��������. �������� ���������� �� �����������
          IRQTransmitterDisable();       
             
          if(boMaster){
            // ���� ������� � ��������������, �� �������� ������ ���������
            if(MSG_CMDWITHRESPONSE == pOutputMsg->GetFlags()){
              SetStateWaitResponse();
              ReceiverStart();
            }     
            // � ��������� ������ ���������� ��������� 
            else SetStateWait();
          }

          // ���������� ������, ���������� ���������� ����������
          pOutputMsg->Release();  
          break;
        }
      }
    }
  }   
}
//---------------------------------------------------------------------------
