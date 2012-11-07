#ifndef INPUTSTATE_H
#define INPUTSTATE_H
//============================================================================
//                                      
//============================================================================
// Семенчик В.Г. 22.01.2010
//============================================================================

#include <RTL.h>
#include "board.h"
//----------------------------------------------------------------------------

/// Состояние каждой клавиши описывается двумя битами
#define KEY_OFF        0
#define KEY_OFF_TO_ON  1
#define KEY_ON         3
#define KEY_ON_TO_OFF  2
//----------------------------------------------------------------------------

class C_ADC  
{
private:
  AT91S_ADC *pADC;  // Базовай адрес АЦП
public:
  C_ADC(AT91S_ADC *pADC_){ pADC = pADC_;}
  U32 DiffTime(U32 uiOldTime);
  void Start(void) { pADC->ADC_CR = AT91C_ADC_START; }
  void Reset(void){	pADC->ADC_CR = AT91C_ADC_SWRST; }
  void ChannelEnable(U8 ucChannel){ pADC->ADC_CHER = ucChannel;}
  void ChannelDisable(U8 ucChannel){ pADC->ADC_CHER = ucChannel;}
  U32  GetData(U8 ucChannel){return ((U32 *)&(pADC->ADC_CDR0))[ucChannel];}
  void Init(U32 uiMode){pADC->ADC_MR = uiMode;}
};
//----------------------------------------------------------------------------

class C_SysTime
{
private:
  U32 uiTime;
  U32 uiT;
public:
  C_SysTime(U32 uiT_) : uiTime(0) {uiT = uiT_;}
  U32 DiffTime(U32 uiOldTime){
    if(uiTime >= uiOldTime) return uiTime - uiOldTime;
    return (0xFFFFFFFF - uiOldTime) + uiTime + 1;
  }
  void IncrementTime(void){uiTime += uiT;} 
  friend  __task void Task_InputState(void);

};
//----------------------------------------------------------------------------

extern OS_TID idtInputState;
extern C_ADC  ADC;
extern C_SysTime SysTime;
__task void Task_InputState(void);
//----------------------------------------------------------------------------

#endif //#ifndef INPUTSTATE_H
