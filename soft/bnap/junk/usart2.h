#ifndef USART2_H
#define USART2_H
//============================================================================
//                                        
//============================================================================

#include "usart.h"
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// C_USART2 - класс для USART2 
//----------------------------------------------------------------------------
class C_USART2 : public C_USART
{
  friend  __irq  void USART2_IRQHandler(void);
  friend bool USART2_InputDataHandler(int iInput);
  friend  __task void Task_USART2(void);
private:
public:
  C_USART2(bool boMaster);
  void Task_USART2(void);
};
//----------------------------------------------------------------------------

extern C_USART2 USART2;
extern OS_TID idtUSART2;
__task void Task_USART2(void);
//----------------------------------------------------------------------------

#endif //#ifndef USART_H
