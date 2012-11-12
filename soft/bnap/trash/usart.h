#ifndef USART_H
#define USART_H
//============================================================================
//                                        
//============================================================================

#include "SysConfig.h"
#include "msg.h"
//----------------------------------------------------------------------------

class I_UpLevel
{
public:
  virtual void SendMsgResponse(U32 *pMailboxSource, U8  ucIDSource, U16 usTypeMSG) = 0;
};
//----------------------------------------------------------------------------

// ��������� �� ������� ��������� �������� ����������� �� UART
// ���������� ��� �� ����������� ���������� ��� ������� ������������ �������.
// ��� ��� ���������� �� ����������� ����������, �� ������ ���� ����������� ��������.
typedef bool (* pfuncUSARTInputDataHandler)(int iInput);
#define TX_PERMIT 0x1
//----------------------------------------------------------------------------

///---------------------------------------------------------------------------
/// C_UART - ������� ����� 
// �������� ������ � ������, ����� ��� ����������� UART � USART
///---------------------------------------------------------------------------
class C_UART
{
protected:
  AT91S_USART *usart;       // ��������� �� ���� ��������� ������������� ���������� UART
  C_Msg * pOutputMsg;       // ��������� �� ��������� ������� ���������� ��������
  pfuncUSARTInputDataHandler pInputDataHandler;
  U32 *pMailboxSource;      // ��������� �� �������� ���� ��������� �������. ������������ ��� ������������ �������������
  U8  ucIDSource;           // ������������� ��������� �������. ������������ ��� ������������ �������������
  U8  ucState;              // 1 - �������� ������, 2 - �������� �������������
  U16 usOutputSimbolCount;  // ������� ���������� ��������
  bool boMaster;
  C_UART(bool boMaster_) : usart(NULL), pOutputMsg(NULL), pInputDataHandler(NULL), pMailboxSource(NULL), 
                 ucIDSource(ID_NULL), ucState(0), boDiagnosticsTx(false), boDiagnosticsRx(false)
                 {boMaster = boMaster_; os_mbx_init(MailBox, sizeof(MailBox));}
public:
  bool boDiagnosticsTx;
  bool boDiagnosticsRx;
  os_mbx_declare(MailBox, USART_MAILBOX_SIZE); 
  void Configure(unsigned int mode, unsigned int baudrate, unsigned int masterClock = BOARD_MCK);
  void TransmitterEnabled(void) {usart->US_CR = AT91C_US_TXEN;} 
//  void TransmitterDisable(void) {usart->US_CR = AT91C_US_TXDIS;} 
  void ReceiverEnabled(void) {usart->US_CR = AT91C_US_RXEN;} 
  void ReceiverDisable(void) {usart->US_CR = AT91C_US_RXDIS;}
  void ReceiverReset(void) {usart->US_CR = AT91C_US_RSTRX;}
  void IRQTransmitterEnabled(void) {ucState = ucState | 1;    usart->US_IER = AT91C_US_TXRDY;} 
  void IRQTransmitterDisable(void) {ucState = ucState & 0xFE; usart->US_IDR = AT91C_US_TXRDY;} 
  void IRQReceiverEnabled(void) {usart->US_IER = AT91C_US_RXRDY;} 
  void IRQReceiverDisable(void) {usart->US_IDR = AT91C_US_RXRDY;}
  void ReceiverStart(void) {ReceiverReset(); ReceiverEnabled(); IRQReceiverEnabled();} 
  void ReceiverStop(void)  {IRQReceiverDisable(); ReceiverDisable(); ucState = ucState & 0xFD;} 
  void SetStateWait(void){pMailboxSource = NULL; ucState = ucState = 0;}
  void SetStateWaitTransmitEnd(U8  ucIDSource_, U32 *pMailbox){ 
           ucIDSource = ucIDSource_; pMailboxSource = pMailbox; ucState = 1;}
  void SetStateWaitResponse(void){ucState = 2;}
  bool IsWaitResponse(void){ return (0 != (ucState & 0x2));}
  bool IsTransmit(void){ return (0 != (ucState & 0x1));}
  U8   GetIDSource(void){ return ucIDSource;}
  U32* GetPtrMailboxSource(void) { return pMailboxSource; }
  void TaskTxRx(I_UpLevel* UpLevel);
};
//----------------------------------------------------------------------------

///---------------------------------------------------------------------------
/// C_USART - ������� ����� ��� USART 
/// �������� ������ � ������, ����� ��� ����������� USART � �� �������� � ����� UART
///---------------------------------------------------------------------------
class C_USART : public C_UART
{
private:
public:
  C_USART(bool boMaster) : C_UART(boMaster) {}
  void SetTimeOut(U16 uiTime){usart->US_RTOR = uiTime;}  
  void IRQTimeOutEnabled(void) {usart->US_CR = AT91C_US_RETTO; usart->US_IER = AT91C_US_TIMEOUT;} 
  void IRQTimeOutDisable(void) {usart->US_IDR = AT91C_US_TIMEOUT;}
};
//----------------------------------------------------------------------------

#endif //#ifndef USART_H
