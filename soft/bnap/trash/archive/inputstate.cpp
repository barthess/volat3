///===========================================================================
/// Через заданный в тиках интервал времени
/// - инкрементирует переменную uiTime, которая используется  для определения 
///   интервалов времени между событиями,
/// - сохраняет состояние аналоговых и цифровых входов,
/// - генерирует сигнал, по которому формируется пакет с путевой информацией
/// Интервал времени задается константой SYSTIME_PERIOD.
//============================================================================
// Семенчик В.Г. 22.01.2010
//============================================================================

#include <AT91SAM7A3.H>
#include <RTL.h>
#include "errorhandler.h"
#include "msg.h"
#include "gsm.h"
#include "management.h"
#include "statehandler.h"
#include "inputstate.h"
//----------------------------------------------------------------------------

OS_TID idtInputState;
C_ADC  ADC(AT91C_BASE_ADC0);
C_SysTime SysTime(SYSTIME_PERIOD);
void MsgToGSM(C_Msg * pMsg);
//----------------------------------------------------------------------------

__task void Task_InputState(void)
{
  os_itv_set (SYSTIME_PERIOD);
  for(;;){
    os_itv_wait ();
    SysTime.IncrementTime();

    // Сохраняю состояние состояния цифровых входов и клавиатуры
    // С каждой кнопкой связано два бита в переменной   State.ucDigitalInputState
    // Младший бит отображает текущее состояние кнопки, старший - предыдущее.
    U32 uiInput = *AT91C_PIOB_PDSR;
    U16  usState = 0;
    if (0 != (D_IN0 & uiInput))      usState = usState | KEY_ALARM; 
    if (0 != (D_IN1 & uiInput))      usState = usState | KEY_ENTER; 
    if (0 != (KBD_ALARM & uiInput))  usState = usState | KEY_ESC; 
    if (0 != (KBD_ESC & uiInput))    usState = usState | KEY_UP; 
    if (0 != (KBD_ENTER & uiInput))  usState = usState | KEY_DOWN; 
    if (0 != (KBD_DOWN & uiInput))   usState = usState | INPUT0; 
    if (0 != (KBD_UP & uiInput))     usState = usState | INPUT1; 
    StateHandler.DigitalInputToState(usState);

    // Сохраняю состояние аналоговых входов
    StateHandler.AnalogInputToState(ADC.GetData(ANALOG_0), ADC.GetData(ANALOG_1), ADC.GetData(ANALOG_2));
    ADC.Start();
  }   
}
//----------------------------------------------------------------------------

