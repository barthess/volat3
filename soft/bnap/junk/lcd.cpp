//============================================================================
// ������� LCD ������                                      
//============================================================================
#include <RTL.h>
#include <string.h>
#include "management.h"
#include "errorhandler.h"
#include "statehandler.h"
#include "lcd.h"
//----------------------------------------------------------------------------

void CharToHex(char *pOut, const char *pIn, U16 usInSize);
//----------------------------------------------------------------------------

C_LCD::C_LCD(C_UART *usart_) : usart(usart_), usCountChar(0) {
  os_mbx_init(MailBoxForResponse, sizeof(MailBoxForResponse));
}

//---------------------------------------------------------------------------
// ���������� ��������� GSM ������
//---------------------------------------------------------------------------
U32 C_LCD::SendMsgToLCD(C_Msg * pMsg){
  pMsg->AddRef();

  C_Msg *pResponse;
  U32 uiError;

  // ��������� ���������
  uiError = os_mbx_send(usart->MailBox, pMsg, TIMEOUT_MAILBUX_REDY);
  // ���� ������� ����� ��������, �� ��������� ���������
  if(OS_R_OK != uiError){
    pMsg->Release();
    ErrorHandler.ErrorToList((ID_LCD << 24) + ERROR_LCD_1);
    uiError = ERROR_MAILBUX_REDY;
  }
  // ������ ������������� �� ����������
  else{
    uiError = os_mbx_wait(MailBoxForResponse, (void **)&(pResponse), TIMEOUT_RESPONSE_REDY);
    // ������������� �� ��������
    if(OS_R_TMO == uiError) {    
      ErrorHandler.ErrorToList((ID_LCD << 24) + ERROR_LCD_2);
      uiError = ERROR_RESPONSE_REDY;
    }
    // ������������� ��������. ��������� ������������� ��������� �������
    else {
      U32 *pMailBoxSource = pResponse->GetPointerMailbox();

      if(NULL != pMailBoxSource){
        uiError = os_mbx_send(pResponse->GetPointerMailbox(), pResponse, TIMEOUT_MAILBUX_REDY);
        // ���� ������� ����� ��������, �� ��������� ���������
        if(OS_R_OK != uiError){
          pResponse->Release();
          ErrorHandler.ErrorToList((ID_LCD << 24) + ERROR_LCD_3);
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
void C_LCD::SendMsgResponse(U32 *pMailboxSource, U8  ucIDSource, U16 usTypeMSG)
{
  pcInputBuf[usCountChar++] = 0;  // �������� �������� ������ 0
  // ������ ��������� c �������� ��������������. 
  C_Msg * pMsg; 
  pMsg = NewMsg(ucIDSource, ID_LCD, pMailboxSource, usTypeMSG, usCountChar);
  memcpy(pMsg->PointerStr(), pcInputBuf, usCountChar); 
  // �����������
  if(IsDiagnosticsRx()) print_text(pMsg->PointerStr());
  // ��������� ���������
  OS_RESULT uiError = os_mbx_send(MailBoxForResponse, pMsg, TIMEOUT_MAILBUX_REDY);
  // ���� ������� ����� ��������, �� ��������� ���������
  if(OS_R_OK != uiError){
    ErrorHandler.ErrorToList((ID_LCD << 24) + ERROR_LCD_4);
    pMsg->Release();
  }
  usCountChar = 0;
}
//----------------------------------------------------------------------------

//---------------------------------------------------------------------------
// ������� ���������� ������������ ���������� ��������� ����������������� 
// ���������� � �������� ���������� LCD ������. 
//---------------------------------------------------------------------------
bool C_LCD::GetData(U8 ch)
{
  // ������ ������ �������
  if(START_DELIMITER == ch) usCountChar = 0;
  if((0 == usCountChar) & (START_DELIMITER != ch)) return false;
  pcInputBuf[usCountChar] = ch;
  ++usCountChar;
  // ������� true ���������� ���������� ��������� ��������� ����� ����������� ��������
  if(END_DELIMITER == ch){
    // ������������� ����� ���� ������. ���������� ������������� � HEX ������
    char ch =  pcInputBuf[1];
    CharToHex(pcInputBuf + 1, &ch, 1);
    pcInputBuf[3] = END_DELIMITER;
    pcInputBuf[4] = 0;
    usCountChar = 5;
    return true;
  }
  // ������������ ����� ������ LCD_INPUT_BUF_SIZE - 1 ( ���� ����� �������� ��� ������������ 0 )
  if(LCD_INPUT_BUF_SIZE - 1 == usCountChar) return true;
  return false;
}
//----------------------------------------------------------------------------
