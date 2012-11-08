#ifndef TWI_H
#define TWI_H
//============================================================================
// Драйвер I2C интерфейса                                     
//============================================================================

#include <RTL.h>
#include "SysConfig.h"
#include "msg.h"
//----------------------------------------------------------------------------

#pragma pack(1)
struct S_TWIMsg
{
  U16 usSCLFrequency;   /// частота SCL
  U8  ucAddresSlave;  
  U8  ucAddressSizeAndOper;  /// [1 - 0] - Размер, [4] - выполняемая операция
  U32 uiInternalAddess;      /// внутренние адреса  [31-24] - не используются
                             /// [23-16] - Addr2, [15-8] - Addr1, [7-0] - Addr0                     
};
#pragma pack(8)
//----------------------------------------------------------------------------

class C_TWI  
{		 
  friend __task void Task_TWI(void);
  friend __irq void TWI_IRQHandler(void);
protected:
  C_Msg * pMsg;   /// Указатель на сообщение которое необходимо передать/принять
  U16 usLength;   /// Число байтов которые необходимо принять/передать 
  U16 usCount;    /// Число принятых/переданных байтов
  U32 *pMailboxSource;  /// Указатель на почтовый ящик источника команды. Используется при формировании подтверждения
  U8  ucIDSource;       /// Идентификатор источника команды. Используется при формировании подтверждения
  bool boBusy;
  char * pcBuf;   /// Указатель на буфер для данных

  void Init(void);
  void IRQEnabled(void){AT91C_BASE_TWI->TWI_IER = AT91C_TWI_TXCOMP | AT91C_TWI_RXRDY;}
  void IRQDisable(void){AT91C_BASE_TWI->TWI_IDR = AT91C_TWI_TXCOMP | AT91C_TWI_RXRDY | AT91C_TWI_TXRDY;}
  bool IsBusy(void) {return boBusy;}
  void SetBusy(void) {boBusy = true;}
  void SetFree(void) {boBusy = false;}
public:
  os_mbx_declare (MailBox, TWI_MAILBOX_SIZE);
  C_TWI(void);
};
//----------------------------------------------------------------------------

extern C_TWI TWI;
extern OS_TID idtTWI;
__task void Task_TWI(void);
__irq void TWI_IRQHandler(void);
//----------------------------------------------------------------------------

#endif //#ifndef TWI_H
