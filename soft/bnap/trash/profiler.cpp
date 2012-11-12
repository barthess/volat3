#include "sysconfig.h"
#include "profiler.h"
//------------------------------------------------------------------------------

void C_Profiler::SaveTime(U16 ucID){
  // Отключаю прерывания
//  AT91C_BASE_AIC->AIC_IDCR = uiInterruptList;
  if(SIZE > usCount){
    puiList[usCount++] = DiffTime(uiStart) * 10000 + ucID;
  }
  else
   __nop();
       // Включаю прерывания
//  AT91C_BASE_AIC->AIC_ICCR = uiInterruptList;
}
//------------------------------------------------------------------------------
