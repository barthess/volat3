//============================================================================
// Драйвер LCD панели                                      
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
// Отправляет сообщение GSM модему
//---------------------------------------------------------------------------
U32 C_LCD::SendMsgToLCD(C_Msg * pMsg){
  pMsg->AddRef();

  C_Msg *pResponse;
  U32 uiError;

  // Отправляю сообщение
  uiError = os_mbx_send(usart->MailBox, pMsg, TIMEOUT_MAILBUX_REDY);
  // Если истекло время ожидания, то уничтожаю сообщение
  if(OS_R_OK != uiError){
    pMsg->Release();
    ErrorHandler.ErrorToList((ID_LCD << 24) + ERROR_LCD_1);
    uiError = ERROR_MAILBUX_REDY;
  }
  // Ожидаю подтверждение от интерфейса
  else{
    uiError = os_mbx_wait(MailBoxForResponse, (void **)&(pResponse), TIMEOUT_RESPONSE_REDY);
    // Подтверждение не получено
    if(OS_R_TMO == uiError) {    
      ErrorHandler.ErrorToList((ID_LCD << 24) + ERROR_LCD_2);
      uiError = ERROR_RESPONSE_REDY;
    }
    // Подтверждение получено. Отправляю подтверждение источнику команды
    else {
      U32 *pMailBoxSource = pResponse->GetPointerMailbox();

      if(NULL != pMailBoxSource){
        uiError = os_mbx_send(pResponse->GetPointerMailbox(), pResponse, TIMEOUT_MAILBUX_REDY);
        // Если истекло время ожидания, то уничтожаю сообщение
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
// Формирует подтверждение
//---------------------------------------------------------------------------
void C_LCD::SendMsgResponse(U32 *pMailboxSource, U8  ucIDSource, U16 usTypeMSG)
{
  pcInputBuf[usCountChar++] = 0;  // Завершаю принятую строку 0
  // Создаю сообщение c принятым подтверждением. 
  C_Msg * pMsg; 
  pMsg = NewMsg(ucIDSource, ID_LCD, pMailboxSource, usTypeMSG, usCountChar);
  memcpy(pMsg->PointerStr(), pcInputBuf, usCountChar); 
  // Диагностика
  if(IsDiagnosticsRx()) print_text(pMsg->PointerStr());
  // Отправляю сообщение
  OS_RESULT uiError = os_mbx_send(MailBoxForResponse, pMsg, TIMEOUT_MAILBUX_REDY);
  // Если истекло время ожидания, то уничтожаю сообщение
  if(OS_R_OK != uiError){
    ErrorHandler.ErrorToList((ID_LCD << 24) + ERROR_LCD_4);
    pMsg->Release();
  }
  usCountChar = 0;
}
//----------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Функция вызывается обработчиком прерываний приемника последовательного 
// интерфейса к которому подключена LCD панель. 
//---------------------------------------------------------------------------
bool C_LCD::GetData(U8 ch)
{
  // Ожидаю начала команды
  if(START_DELIMITER == ch) usCountChar = 0;
  if((0 == usCountChar) & (START_DELIMITER != ch)) return false;
  pcInputBuf[usCountChar] = ch;
  ++usCountChar;
  // Получив true обработчик прерываний приемника прекратит прием поступающих символов
  if(END_DELIMITER == ch){
    // Подтверждение всего один символ. Преобразую подтверждение в HEX формат
    char ch =  pcInputBuf[1];
    CharToHex(pcInputBuf + 1, &ch, 1);
    pcInputBuf[3] = END_DELIMITER;
    pcInputBuf[4] = 0;
    usCountChar = 5;
    return true;
  }
  // Максимальная длина строки LCD_INPUT_BUF_SIZE - 1 ( Одно место оставляю для завершающего 0 )
  if(LCD_INPUT_BUF_SIZE - 1 == usCountChar) return true;
  return false;
}
//----------------------------------------------------------------------------
