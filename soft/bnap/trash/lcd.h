#ifndef LCD_H
#define LCD_H
//============================================================================
// Драйвер LCD панели                                      
//============================================================================

#include <RTL.h>
#include "SysConfig.h"
#include "usart.h"
#include "msg.h"
//----------------------------------------------------------------------------

const U8 LCD_INPUT_BUF_SIZE = 5;
//----------------------------------------------------------------------------

class C_LCD : public I_UpLevel
{
private:
  C_UART *usart;     // Указатель на интерфейс для связи с РС
  U16 usCountChar;   // Число принятых символов команды
  char pcInputBuf[LCD_INPUT_BUF_SIZE];
public:
  os_mbx_declare(MailBoxForResponse, LCD_MAILBOX_SIZE); 

  C_LCD(C_UART *usart_);

  bool GetData(U8 ch);
  U8   GetIDSource(void){return usart->GetIDSource();}
  U32* GetPtrMailboxSource(void){return usart->GetPtrMailboxSource();}
  U32  SendMsgToLCD(C_Msg * pMsg); 
  void SetUART(C_UART *usart_){usart = usart_;} 
  virtual void SendMsgResponse(U32 *pMailboxSource, U8  ucIDSource, U16 usTypeMSG);

  void DiagnosticsTxDisable(void) {usart->boDiagnosticsTx = false;}
  void DiagnosticsRxDisable(void) {usart->boDiagnosticsRx = false;}
  void DiagnosticsTxEnabled(void) {usart->boDiagnosticsTx = true;}
  void DiagnosticsRxEnabled(void) {usart->boDiagnosticsRx = true;}
  bool IsDiagnosticsRx(void) {return usart->boDiagnosticsRx;}
};
//----------------------------------------------------------------------------

extern C_LCD LCD;
//extern OS_TID idtLCD_Rx;
//extern OS_TID idtLCD_Tx;
//__task void Task_LCD_Rx(void);
//__task void Task_LCD_Tx(void);
//----------------------------------------------------------------------------

#endif //#ifndef LCD_H
