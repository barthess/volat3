#ifndef LINKPC_H
#define LINKPC_H
///===========================================================================
/// Модуль конфигурирования и мониторинга системы посредством РС
//============================================================================

#include <RTL.h>
#include "usart.h"
#include "msg.h"
#include "SysConfig.h"
//----------------------------------------------------------------------------

class C_LinkPC : public I_UpLevel
{
  friend void Task_LinkPC_Rx(void);
private:
  C_UART *usart;     // Указатель на интерфейс для связи с РС
  U16 usCountChar;   // Число принятых символов команды
  char pcInputBuf[LINKPC_INPUT_BUF_SIZE];
public:
  C_LinkPC(C_UART* usart_) : usart(usart_), usCountChar(0) {}

  bool GetData(U8 ch);
//  U8   GetIDSource(void){return usart->GetIDSource();}
  U32* GetPtrMailbox(void){return usart->MailBox;}
  U32  SendMsgToPC(C_Msg * pMsg);
  void SetUART(C_UART *usart_){usart = usart_;}    
  virtual void SendMsgResponse(U32 *pMailboxSource, U8  ucIDSource, U16 usTypeMSG){};
};
//----------------------------------------------------------------------------

extern C_LinkPC LinkPC;
extern OS_TID idtLinkPC_Rx;
extern OS_TID idtLinkPC_Tx;
__task void Task_LinkPC_Rx(void);
__task void Task_LinkPC_Tx(void);
//----------------------------------------------------------------------------

#endif //#ifndef LINKPC_H
