//============================================================================
// Модуль связи с РС
//============================================================================

#include <string.h>
#include "errorhandler.h"
#include "management.h"
#include "linkpc.h"
//----------------------------------------------------------------------------

void Restart(void);

//----------------------------------------------------------------------------
U32 C_LinkPC::SendMsgToPC(C_Msg * pMsg){
   if(NULL != pMsg){ 
     pMsg->AddRef();
     if(OS_R_OK == os_mbx_send(usart->MailBox, pMsg, TIMEOUT_MAILBUX_REDY)) return NO_ERROR;
     else return ERROR_MSGTOPC_1;
   }
   else return ERROR_MSGTOPC_2; 
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Функция вызывается обработчиком прерываний приемника последовательного 
// интерфейса к которому подключен PC. 
//---------------------------------------------------------------------------
bool C_LinkPC::GetData(U8 ch)
{
  // Ожидаю начала команды
  if(START_DELIMITER == ch) usCountChar = 0;
  if((0 == usCountChar) & (START_DELIMITER != ch)) return false;
  pcInputBuf[usCountChar] = ch;
  ++usCountChar;
  // Получив true обработчик прерываний приемника прекратит прием поступающих символов
  if(END_DELIMITER == ch) return true;   
  // Максимальная длина строки GSM_INPUT_BUF_SIZE - 1. (Одно место оставляю для завершающего 0)
  if((LINKPC_INPUT_BUF_SIZE - 1) == usCountChar) return true;     
  return false;
}
//----------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Ожидает поступления команды от РС
//---------------------------------------------------------------------------
__task void Task_LinkPC_Rx(void) 
{
  // Разрешаю работу приемника
  LinkPC.usCountChar = 0;
  LinkPC.usart->ReceiverStart();

  for(;;){
    os_evt_wait_or(RX_REDY, 0xFFFF);
    LinkPC.pcInputBuf[LinkPC.usCountChar] = 0;  // Завершаю принятую строку 0
    ++LinkPC.usCountChar;
    C_Msg * pMsg; 
    pMsg = NewMsg(ID_MANAGEMENT, ID_LINKPC, LinkPC.GetPtrMailbox(), MSG_CMDWITHRESPONSE, LinkPC.usCountChar);
    memcpy(pMsg->PointerStr(), LinkPC.pcInputBuf, LinkPC.usCountChar); 
    m_os_mbx_send(Management.MailBox, pMsg, TIMEOUT_MAILBUX_REDY, ID_LINKPC);

    LinkPC.usCountChar = 0;
    LinkPC.usart->ReceiverStart();
  }
}
//----------------------------------------------------------------------------
