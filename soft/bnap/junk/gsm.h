#ifndef GSM_H
#define GSM_H
//============================================================================
// Драйвер GSM модема                                     
//============================================================================

#include <RTL.h>
#include "pio/pio.h"
#include "SysConfig.h"
#include "usart.h"
//----------------------------------------------------------------------------

class C_GSM : public I_UpLevel  
{
protected:
  C_UART *usart;     // Указатель на интерфейс для связи с GSM модемом
  U16 usCountChar;   // Число принятых символов 
  char pcInputBuf[GSM_INPUT_BUF_SIZE];
public:
  os_mbx_declare(MailBoxForResponse, 4); 
  bool boEnableRequest;

  C_GSM(C_UART *usart_);
  
  void HardReset(void);
  void HardOff(void);
  void HardOn(void);

  void DiagnosticsTxDisable(void) {usart->boDiagnosticsTx = false;}
  void DiagnosticsRxDisable(void) {usart->boDiagnosticsRx = false;}
  void DiagnosticsTxEnabled(void) {usart->boDiagnosticsTx = true;}
  void DiagnosticsRxEnabled(void) {usart->boDiagnosticsRx = true;}
  bool IsDiagnosticsRx(void) {return usart->boDiagnosticsRx;}

  U8   GetIDSource(void){return usart->GetIDSource();}
  U32* GetPtrMailboxSource(void){return usart->GetPtrMailboxSource();}
  bool GetData(U8 ch);
  U32  SendMsgToGSM(C_Msg * pMsg);
  virtual void SendMsgResponse(U32 *pMailboxSource, U8  ucIDSource, U16 usTypeMSG);

};
//----------------------------------------------------------------------------

extern C_GSM GSM;
//----------------------------------------------------------------------------

#endif //#ifndef GSM_H
