//============================================================================
// ������� GSM ������                                     
//============================================================================

#include <RTL.h>
#include <string.h>
#include <stdio.h>
#include "errorhandler.h"
#include "gsm.h"
//----------------------------------------------------------------------------

char HexToChar(U8 *pOut, const char *pIn, U16 usOutSize);
void Restart(void);
bool print_text(const char *pstrz);
//----------------------------------------------------------------------------

C_GSM::C_GSM(C_UART* usart_) : usart(usart_), usCountChar(0), boEnableRequest(false){
  os_mbx_init(MailBoxForResponse, sizeof(MailBoxForResponse));
  // ��������� ������ ��� ���������� �������. (���������� ����� � On/Off )
  const Pin Pins[] = {PIN_GSM_RESET, PIN_GSM_ON_OFF};  
  PIO_Configure(Pins, PIO_LISTSIZE(Pins)); 
}
//----------------------------------------------------------------------------

void C_GSM::HardReset(void){
  const Pin Pins[] = {PIN_GSM_RESET};  
  PIO_Clear(Pins);
  os_dly_wait(1);
  PIO_Set(Pins);
} 
//----------------------------------------------------------------------------

void C_GSM::HardOn(void){
  const Pin Pins[] = {PIN_GSM_ON_OFF};  
  PIO_Set(Pins);
} 
//----------------------------------------------------------------------------

void C_GSM::HardOff(void){
  const Pin Pins[] = {PIN_GSM_ON_OFF};  
  PIO_Clear(Pins);
} 
//----------------------------------------------------------------------------

//---------------------------------------------------------------------------
// ������� ���������� ������������ ���������� ��������� ����������������� 
// ���������� � �������� ��������� GSM �����. 
//---------------------------------------------------------------------------
bool C_GSM::GetData(U8 ch)
{
  // ������ ������ �������
  if(START_DELIMITER == ch) usCountChar = 0;
  if((0 == usCountChar) & (START_DELIMITER != ch)) return false;
  pcInputBuf[usCountChar] = ch;
  ++usCountChar;
  // ������� true ���������� ���������� ��������� ��������� ����� ����������� ��������
  if(END_DELIMITER == ch) return true;   
  // ������������ ����� ������ GSM_INPUT_BUF_SIZE - 1. (���� ����� �������� ��� ������������ 0)
  if((GSM_INPUT_BUF_SIZE - 1) == usCountChar) return true;     
  return false;
}
//----------------------------------------------------------------------------


//---------------------------------------------------------------------------
// ���������� ��������� GSM ������
//---------------------------------------------------------------------------
U32 C_GSM::SendMsgToGSM(C_Msg * pMsg){
  pMsg->AddRef();

  C_Msg *pResponse;
  U32 uiError;

  // ��������� ���������
  uiError = os_mbx_send(usart->MailBox, pMsg, TIMEOUT_MAILBUX_REDY);
  // ���� ������� ����� ��������, �� ��������� ���������
  if(OS_R_OK != uiError){
    pMsg->Release();
    ErrorHandler.ErrorToList((ID_GSM << 24) + ERROR_GSM_1);
    uiError = ERROR_MAILBUX_REDY;
  }
  // ������ ������������� �� ����������
  else{
    uiError = os_mbx_wait(MailBoxForResponse, (void **)&(pResponse), TIMEOUT_RESPONSE_REDY);
    // ������������� �� ��������
    if(OS_R_TMO == uiError) {    
      ErrorHandler.ErrorToList((ID_GSM << 24) + ERROR_GSM_2);
      uiError = ERROR_RESPONSE_REDY;
    }
    // ������������� ��������. ��������� ������������� ��������� �������
    else {
      U32 *pMailBoxSource = pResponse->GetPointerMailbox();
      if(NULL != pMailBoxSource){
        uiError = os_mbx_send(pMailBoxSource, pResponse, TIMEOUT_MAILBUX_REDY);
        // ���� ������� ����� ��������, �� ��������� ���������
        if(OS_R_OK != uiError){
          pResponse->Release();
          ErrorHandler.ErrorToList((ID_GSM << 24) + ERROR_GSM_3);
          uiError = ERROR_MAILBUX_REDY;
        }
      }
      else pResponse->Release();

    }
  }
  return uiError;
}
//----------------------------------------------------------------------------

//---------------------------------------------------------------------------
// ��������� �������������
//---------------------------------------------------------------------------
void C_GSM::SendMsgResponse(U32 *pMailboxSource, U8  ucIDSource, U16 usTypeMSG)
{
  pcInputBuf[usCountChar++] = 0;  // �������� �������� ������ 0
  // ������ ��������� c �������� ��������������. 
  C_Msg * pMsg; 
  pMsg = NewMsg(ucIDSource, ID_GSMR, pMailboxSource, usTypeMSG, usCountChar);
  memcpy(pMsg->PointerStr(), pcInputBuf, usCountChar); 
  // �����������
  if(IsDiagnosticsRx()) print_text(pMsg->PointerStr());
  // ��������� ���������
  OS_RESULT uiError = os_mbx_send(MailBoxForResponse, pMsg, TIMEOUT_MAILBUX_REDY);
  // ���� ������� ����� ��������, �� ��������� ���������
  if(OS_R_OK != uiError){
    ErrorHandler.ErrorToList((ID_GSMR << 24) + ERROR_GSM_4);
    pMsg->Release();
  }
  usCountChar = 0;
}
//----------------------------------------------------------------------------

