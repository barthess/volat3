//============================================================================
// ������� I2C ����������                                     
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
  *AT91C_PMC_PCER = 0x1 << AT91C_ID_TWI;   // ������� CLK ��� TWI
  const Pin Pins[] = {PINS_TWI};  
  PIO_Configure(Pins, PIO_LISTSIZE(Pins)); // ��������� ������ � TWI
  /// ������������ ���������
  AIC_ConfigureIT(AT91C_ID_TWI, AT91C_AIC_SRCTYPE_INT_HIGH_LEVEL, TWI_IRQHandler);
  AIC_EnableIT(AT91C_ID_TWI);
}
//----------------------------------------------------------------------------

//---------------------------------------------------------------------------
// � ������� ������ ��������� ����������  ���� ����������, �������  
// ���������� ����� ������ Twi ���������� � ����������� ��������
//---------------------------------------------------------------------------
void C_TWI::Init(void){
  // � ������� ������ ��������� ����������  ���� ����������, �������  
  // ���������� ����� ������ TWI ���������� � ����������� ��������
  AT91C_BASE_TWI->TWI_CR = AT91C_TWI_SWRST;
  S_TWIMsg * pTWIMsg = (S_TWIMsg *)pMsg->PointerStr();

  // ������������ �������
  // Tlow = (CLDIV * 2^CKDIV + 3)* Tmck  Thigh = (CHDIV * 2^CKDIV + 3)* Tmck
  // ��� CKDIV = 0, CLDIV = CHDIV = CDIV,  usSCLFrequency � ���.
  // Tlow = Thigh = 1/(2 * usSCLFrequency * 1000) = (CLDIV + 3)/BOARD_MCK
  // ==> CLDIV = BOARD_MCK/2/usSCLFrequency - 3
  U8 ucCDIV = BOARD_MCK/2000/pTWIMsg->usSCLFrequency - 3;
//  AT91C_BASE_TWI->TWI_CWGR = (1 << 17) + (ucCDIV << 8) + ucCDIV;
  AT91C_BASE_TWI->TWI_CWGR = (ucCDIV << 8) + ucCDIV;

  // �������� ����� ��������
  AT91C_BASE_TWI->TWI_CR = AT91C_TWI_MSDIS;
  AT91C_BASE_TWI->TWI_CR = AT91C_TWI_MSEN;

  // ������������ slave �����, ������ ����������� ������ � ����������� ��������
  AT91C_BASE_TWI->TWI_MMR = (pTWIMsg->ucAddresSlave << 16) +  (pTWIMsg->ucAddressSizeAndOper << 8);
  // ������������ ���������� ������
  AT91C_BASE_TWI->TWI_IADR = pTWIMsg->uiInternalAddess;

  // ��������� ������ ������
  usLength = pMsg->GetLength() - sizeof(S_TWIMsg);
  // ������ ������� ��������/���������� ������
  usCount = 0; 
  // ������������ ��������� �� ������ ������
  pcBuf = pMsg->PointerStr() + sizeof(S_TWIMsg);
}
//----------------------------------------------------------------------------


//---------------------------------------------------------------------------
// �������� ��������� �� ��������� �����, ������������� TWI � ��������� ���������� 
//---------------------------------------------------------------------------
static volatile int i;
__task void Task_TWI(void) 
{
  for(;;){
    // ��������� �����. 
    if(TWI.IsBusy()) {
      U16 usResult = os_evt_wait_or(TX_REDY | RX_REDY | TWI_ERR, TIMEOUT_TWI);
      if(OS_R_EVT == usResult) {
        U16 usFlags = os_evt_get();

        //  ������� ��������
        if(TX_REDY == usFlags) { 
          TWI.pMsg->Release(); 
          TWI.pMsg = NULL;
        } 

        //  �������� �����
        else if(RX_REDY == usFlags) { 
                TWI.pMsg->SetFlags(MSG_RESPONSE); // ����� ��� ���������
                os_mbx_send (TWI.pMsg->GetPointerMailbox(), TWI.pMsg, TIMEOUT_TWI);
              }
              else goto ERROR;  // TWI_ERR
      }

      // ������. TimeOut
      else {
ERROR:
        TWI.IRQDisable();

        // ����������� ������. ��������� ���������� ���������
        if(0 == (AT91C_BASE_TWI->TWI_MMR & AT91C_TWI_MREAD)) {       
          TWI.pMsg->Release(); 
          TWI.pMsg = NULL;
        }

        // ����������� ������. ��������� ��������� �� ������
        else {
          TWI.pMsg->SetFlags(MSG_ERROR);  // ����� ��� ���������
          os_mbx_send (TWI.pMsg->GetPointerMailbox(), TWI.pMsg, TIMEOUT_TWI);
        }
      } 

      // � ����� ������ ������ ����� � ���������� ���������
      os_evt_wait_or(0xFFFF, 0);
      TWI.SetFree();
    }   

    // ������ ������ ��������� �������� �������� ������
    else {
      os_mbx_wait(TWI.MailBox, (void **)&(TWI.pMsg), 0xFFFF); 
      TWI.SetBusy();
      TWI.Init();
      // ���� ���������� ��� ����������� ���� ����, �� ������������ ������������ ��� �����
      if(1 == TWI.usLength) AT91C_BASE_TWI->TWI_CR =  AT91C_TWI_START | AT91C_TWI_STOP;
      else AT91C_BASE_TWI->TWI_CR = AT91C_TWI_START;  
        
      // �������� ������.
      if(0 == (AT91C_BASE_TWI->TWI_MMR & AT91C_TWI_MREAD)){      
        AT91C_BASE_TWI->TWI_THR = TWI.pcBuf[0];
        TWI.usCount = 1;
        AT91C_BASE_TWI->TWI_IER = AT91C_TWI_TXRDY;
      }

      // �������� ������.
      else { 
        i = AT91C_BASE_TWI->TWI_RHR;
        AT91C_BASE_TWI->TWI_IER = AT91C_TWI_RXRDY;
      }
    }
  }
}
//----------------------------------------------------------------------------

//---------------------------------------------------------------------------
// ���������� ���������� 
//---------------------------------------------------------------------------
__irq void TWI_IRQHandler(void) { 
  U32 uiIMR = AT91C_BASE_TWI->TWI_IMR;
  U32 uiSR  = AT91C_BASE_TWI->TWI_SR;

  // ���������� ��������
  if(0 != (AT91C_TWI_TXCOMP & uiIMR & uiSR)){
    AT91C_BASE_TWI->TWI_IDR = AT91C_TWI_TXCOMP;
    // ���������� �������� ������
    if(0 == (AT91C_BASE_TWI->TWI_MMR & AT91C_TWI_MREAD))  isr_evt_set(TX_REDY, idtTWI);  
    // ���������� �������� ������
    else  isr_evt_set(RX_REDY, idtTWI);  
  }

  // ������
  if(0 != (AT91C_TWI_RXRDY & uiIMR & uiSR)){ 
    // ��� ������ ������ ������� � ��������� � ������� ��������� �������
    TWI.pcBuf[TWI.usCount++] = AT91C_BASE_TWI->TWI_RHR;
    if(TWI.usCount == TWI.usLength){
      if(1 != TWI.usLength) AT91C_BASE_TWI->TWI_CR = AT91C_TWI_STOP;
      AT91C_BASE_TWI->TWI_IDR = AT91C_TWI_RXRDY;
      AT91C_BASE_TWI->TWI_IER = AT91C_TWI_TXCOMP;
    }
  }

  // ������
  if(0 != (AT91C_TWI_TXRDY & uiIMR & uiSR)){ 
    if(TWI.usCount == TWI.usLength){
      // ��� �������� 
      if(1 != TWI.usLength) AT91C_BASE_TWI->TWI_CR = AT91C_TWI_STOP;
      AT91C_BASE_TWI->TWI_IDR = AT91C_TWI_TXRDY;
      AT91C_BASE_TWI->TWI_IER = AT91C_TWI_TXCOMP;
    }
    else  
      AT91C_BASE_TWI->TWI_THR = TWI.pcBuf[TWI.usCount++];
  }

  /// ������ TWI
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
