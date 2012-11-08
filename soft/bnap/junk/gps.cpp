//============================================================================
// Драйвер GPS приемника                                     
//============================================================================

#include <RTL.h>
#include <string.h>
#include "management.h"
#include "errorhandler.h"
#include "rtc1338.h"
#include "statehandler.h"
#include "gps.h"
//----------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Принимает только два пакета: GPGGA и GPRMC
// Функция вызывается обработчиком прерываний приемника последовательного 
// интерфейса к которому подключен GPS. 
//---------------------------------------------------------------------------
bool C_GPS::GetData(char ch)
{
  // Ожидаю начала пакета
  if((0 == usCount) & ('$' != ch)) return false;
  pcInputBuf[usCount] = ch;
  if(usCount == 5){
    pcInputBuf[6] = 0;
    int iGGA = strcmp(pcInputBuf, "$GPGGA");
    int iRMC = strcmp(pcInputBuf, "$GPRMC");
    // Если "чужой" пакет то завершаю прием
    if((0 != iGGA) && (0 != iRMC)){ usCount = 0; return false;}
  }

  // Если пакет принят полностью, то устанавливаю 
  // флаг готовности для задачи GPS_Rx 
  // Пакет должен содержать как минимум 6 символов(пустой пакет)
  if(5 >= usCount) goto GetData_1;
  if(0xD != pcInputBuf[usCount]) goto GetData_1;
  ++usCount;

  // Устанавливаю указатель на свободный буфер
  if(pcInputBuf == pcDataBuf_0){ pcInputBuf = pcDataBuf_1; pcBufOut = pcDataBuf_0;}
  else                         { pcInputBuf = pcDataBuf_0; pcBufOut = pcDataBuf_1;}
  usCountChar = usCount;                               
  usCount = 0;              // Завершаю приём сообщения
  return true; 
   
GetData_1:
  ++usCount;  
  // Буфер переполнен
  if((GPS_DATA_BUF_SIZE - 1) == usCount)  usCount = 0;  
  return false;
}
//----------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Ожидает поступления подтверждения команды и отправляет его РС 
//---------------------------------------------------------------------------
__task void Task_GPS_Rx(void) 
{
  // Разрешаю работу приемника
  GPS.usCountChar = 0;
  GPS.usart->ReceiverStart();

  for(;;){
    os_evt_wait_or(RX_REDY, 0xFFFF);
    // Сохраняю полученные данные
    GPS.pcBufOut[GPS.usCountChar] = 0;  
    StateHandler->GPSDataToState(GPS.pcBufOut);

    // Если данные не валидны, то использую время от RTC
    if(!StateHandler->GetGPSStatus()){
      float fTime; U32 uiDate;
      // Получаю время от RTC
      RTC1338.TimeToUTC(&fTime,&uiDate);
      // Сохраняю время в состоянии системы.
      StateHandler->SetTimeUTC(fTime, uiDate);
    }

    // Диагностика
    if(GPS.IsDiagnosticsRx()) print_text(GPS.pcBufOut);   
  }
}
//----------------------------------------------------------------------------
