//============================================================================
// Драйвер RTC DC1338                                     
//============================================================================

#include <string.h>
#include <stdio.h>
#include "rtc1338.h"
#include "profiler.h"
#include "errorhandler.h"
//----------------------------------------------------------------------------

// --------------------  class C_RTC1338 -------------------------------------

C_RTC1338::C_RTC1338() : usSCLFrequency(100), ucAddresSlave(0x68), ucAddressSizeAndOper(0x01), uiTimeZone(0)
{ 
  os_mbx_init(MailBox, sizeof(MailBox));
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void  C_RTC1338::SetTimeZone(U8 ucTime){
  uiTimeZone = ucTime*3600;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

time_t C_RTC1338::GetTime(void)
{
  U8 pc[7];
  tm stTime;
  time_t Time = 0xFFFFFFFF;
  U16 usError = Read(0, pc, 7);

  if(NO_ERROR == usError){
    stTime.tm_sec =  ((pc[0] >> 4) & 0x07) * 10 + (pc[0] & 0x0F);
    stTime.tm_min =  ((pc[1] >> 4) & 0x07) * 10 + (pc[1] & 0x0F);
    stTime.tm_hour = ((pc[2] >> 4) & 0x03) * 10 + (pc[2] & 0x0F);
    stTime.tm_mday = ((pc[4] >> 4) & 0x03) * 10 + (pc[4] & 0x0F);
    stTime.tm_mon =  ((pc[5] >> 4) & 0x01) * 10 + (pc[5] & 0x0F); 
    stTime.tm_year = ((pc[6] >> 4) & 0x0F) * 10 + (pc[6] & 0x0F);
    stTime.tm_isdst= 0;
  if(22 != stTime.tm_mday)
    Time = 0;  
    // Согласование форматов  RTC1338 и структуры tm
    stTime.tm_mon = stTime.tm_mon - 1;
    stTime.tm_year = stTime.tm_year + 100;
    Time = mktime(&stTime);
    Time += uiTimeZone;                                 
  }
  else ErrorHandler.ErrorToList((ID_RTC1338 << 16) + ERROR_GETTIME);

  return Time;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

U16 C_RTC1338::SetTime(time_t t){
  U8 pc[7];
  t = t - uiTimeZone; 
  tm * pstTime = localtime(&t);
  pc[0]  =  pstTime->tm_sec  % 10;         /// Единицы секунд
  pc[0] += (pstTime->tm_sec  / 10) << 4;   /// Десятки секунд
  pc[1]  =  pstTime->tm_min  % 10;         /// Единицы минут
  pc[1] += (pstTime->tm_min  / 10) << 4;   /// Десятки минут
  pc[2]  =  pstTime->tm_hour % 10;         /// Единицы часов
  pc[2] += (pstTime->tm_hour / 10) << 4;   /// Десятки часов
  pc[3]  =  pstTime->tm_wday + 1;
  pc[4]  =  pstTime->tm_mday % 10;         /// Единицы дней
  pc[4] += (pstTime->tm_mday / 10) << 4;   /// Десятки дней
  pstTime->tm_mon = pstTime->tm_mon + 1;   /// Согласование форматов  RTC1338 и структуры tm
  pc[5]  =  pstTime->tm_mon  % 10;         /// Единицы месяцев
  pc[5] += (pstTime->tm_mon  / 10) << 4;   /// Десятки месяцев
  pstTime->tm_year = pstTime->tm_year - 100;
  pc[6]  =  pstTime->tm_year % 10;         /// Единицы лет
  pc[6] += (pstTime->tm_year / 10) << 4;   /// Десятки лет
  U16 usError = Write(0, pc, 7);
  return usError;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
// Формат строки: DD.MM.YY  hh.mm.ss
//---------------------------------------------------------------------------
U16 C_RTC1338::SetTimeFromStr(char *pstrzTime){
  char pch[3];
  tm stTime;
  while(' ' == *pstrzTime) ++pstrzTime;   /// Убираю лидирующе пробелы

  /// Выделяю день
  pch[0] = *pstrzTime;   ++pstrzTime;
  pch[1] = *pstrzTime;   ++pstrzTime;
  pch[2] = 0;
  if( 1 != sscanf(pch, "%u", &stTime.tm_mday)) return ERROR_DS1338_SSCANF; /// Ошибка в аргументе
  /// Выделяю месяц
  ++pstrzTime;            /// Пропускаю '.'
  pch[0] = *pstrzTime;   ++pstrzTime;
  pch[1] = *pstrzTime;   ++pstrzTime;
  pch[2] = 0;
  if( 1 != sscanf(pch, "%u", &stTime.tm_mon)) return ERROR_DS1338_SSCANF; /// Ошибка в аргументе
  stTime.tm_mon = stTime.tm_mon - 1;   /// Согласование форматов ППри вводе месяц нумеруются с 1 
  /// Выделяю год
  ++pstrzTime;            /// Пропускаю '.'
  pch[0] = *pstrzTime;   ++pstrzTime;
  pch[1] = *pstrzTime;   ++pstrzTime;
  pch[2] = 0;
  if( 1 != sscanf(pch, "%u", &stTime.tm_year)) return ERROR_DS1338_SSCANF; /// Ошибка в аргументе
  stTime.tm_year += 100;  /// Отсчет от 1900 года. Минимальное значение для tm_year 70

  while(' ' == *pstrzTime) ++pstrzTime;   /// Убираю пробелы разделяющие дату и время

  /// Выделяю час
  pch[0] = *pstrzTime;   ++pstrzTime;
  pch[1] = *pstrzTime;   ++pstrzTime;
  pch[2] = 0;
  if( 1 != sscanf(pch, "%u", &stTime.tm_hour)) return ERROR_DS1338_SSCANF; /// Ошибка в аргументе
  /// Выделяю минуты
  ++pstrzTime;            /// Пропускаю ':'
  pch[0] = *pstrzTime;   ++pstrzTime;
  pch[1] = *pstrzTime;   ++pstrzTime;
  pch[2] = 0;
  if( 1 != sscanf(pch, "%u", &stTime.tm_min)) return ERROR_DS1338_SSCANF; /// Ошибка в аргументе
   /// Выделяю секунды
  pstrzTime++;            /// Пропускаю ':'
  pch[0] = *pstrzTime;   ++pstrzTime;
  pch[1] = *pstrzTime;   ++pstrzTime;
  pch[2] = 0;
  if( 1 != sscanf(pch, "%u", &stTime.tm_sec)) return ERROR_DS1338_SSCANF; /// Ошибка в аргументе

  time_t t = mktime(&stTime);
  U16 usError = SetTime(t);
  return usError;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

///---------------------------------------------------------------------------
/// Формат даты:     U32   DDMMYY
/// Формат времени:  float hhmmss.sss
///---------------------------------------------------------------------------
U16 C_RTC1338::SetTimeFromUTC(float fTime, U32 uiDate){
  tm stTime;

  /// Выделяю дату
  stTime.tm_year = uiDate % 100;  uiDate = uiDate/100;
  stTime.tm_year += 100;  /// Отсчет от 1900 года. Минимальное значение для tm_year 70
  stTime.tm_mon  = uiDate % 100; 
  stTime.tm_mday = uiDate/100;

  /// Выделяю время
  uiDate = fTime;  // Отбрасываю микросекунды
  stTime.tm_sec  = uiDate % 100;  uiDate = uiDate/100;
  stTime.tm_min  = uiDate % 100; 
  stTime.tm_hour = uiDate/100;

  time_t t = mktime(&stTime);
  U16 usError = SetTime(t);
  return usError;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

U16 C_RTC1338::TimeToStr(char *pstrz, U16 usSize){
  time_t t = GetTime();
  if(0xFFFFFFFF == t){
    pstrz[0] = 0; 
    return ERROR_DS1338_GETTIME;
  }
  strftime(pstrz, usSize,"%d.%m.%y  %H:%M:%S", localtime(&t));
  return NO_ERROR;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

U16 C_RTC1338::TimeToUTC(float *pfTime, U32 *puiDate){
  time_t t = GetTime();
  if(0xFFFFFFFF == t){
    *pfTime = 0;
    *puiDate = 0; 
    return ERROR_DS1338_GETTIME;
  }
  t = t - uiTimeZone;    // Процедура GetTime() возвращает скоректированное время
  tm * stTime = localtime(&t);
  *pfTime  = stTime->tm_hour * 10000 + stTime->tm_min * 100 + stTime->tm_sec; 
  *puiDate = stTime->tm_mday * 10000 + (stTime->tm_mon + 1) * 100 + (stTime->tm_year - 100);
  return NO_ERROR;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

U32 C_RTC1338::Write(U8 ucAddress, const U8 * pcData, U8 ucSizeData){
  const U8 ADDRESS_MAX  = 63;
  C_Msg * pMsg;
  S_TWIMsg *TWIMsg;
  U32 uiError = NO_ERROR;

  if(ADDRESS_MAX < ucAddress){ 
    uiError = ERROR_DS1338_WRITE_1;
    goto END;
  }
  if(ADDRESS_MAX < ucAddress + ucSizeData - 1){ 
    uiError = ERROR_DS1338_WRITE_2;
    goto END;
  }
  // В структуре S_TWIMsg передаю параметры работы I2C интерфейса
  pMsg = NewMsg(ID_TWI, ID_RTC1338, MailBox, MSG_CMDWITHRESPONSE, sizeof(S_TWIMsg) + ucSizeData);

  TWIMsg = (S_TWIMsg *)pMsg->PointerStr();
  TWIMsg->usSCLFrequency = usSCLFrequency;
  TWIMsg->ucAddresSlave  = ucAddresSlave;
  TWIMsg->ucAddressSizeAndOper  = ucAddressSizeAndOper;
  TWIMsg->uiInternalAddess = ucAddress; 
  memcpy(pMsg->PointerStr() + sizeof(S_TWIMsg), pcData, ucSizeData);  

  // Отправляю команду записи данных данных
  if(!m_os_mbx_send(TWI.MailBox, pMsg, TIMEOUT_MAILBUX_REDY, ID_TWI)){ 
    uiError = ERROR_DS1338_WRITE_4;
    goto END;
  }
END:
  return uiError; 
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//----------------------------------------------------------------------------
// По результатам измерений время выполнения операции чтение присмерно 2 мс.
// Приоритет модуля TWI = 2, остальных модулей 1
//----------------------------------------------------------------------------
U32 C_RTC1338::Read(U8 ucAddress, U8 * pcData, U8 ucSizeData)
{
  const U8 ADDRESS_MAX = 63;
  const U8 OPER_READ = 0x10;
  C_Msg * pMsg;
  S_TWIMsg *TWIMsg;
  C_Msg * pMsgResponse;
  U32 uiError = NO_ERROR;

  if(ADDRESS_MAX < ucAddress){
    uiError = ERROR_DS1338_READ_1;
    goto END;
  }
  if(ADDRESS_MAX < ucAddress + ucSizeData - 1){
    uiError = ERROR_DS1338_READ_2;
    goto END;
  }

  // В структуре S_TWIMsg передаю параметры работы I2C интерфейса
  pMsg = NewMsg(ID_TWI, ID_RTC1338, MailBox, MSG_CMDWITHOUTRESPONSE, sizeof(S_TWIMsg) + ucSizeData);

  TWIMsg = (S_TWIMsg *)pMsg->PointerStr();
  TWIMsg->usSCLFrequency = usSCLFrequency;
  TWIMsg->ucAddresSlave  = ucAddresSlave;
  TWIMsg->ucAddressSizeAndOper  = ucAddressSizeAndOper | OPER_READ;
  TWIMsg->uiInternalAddess = ucAddress; 

  // Отправляю команду чтения данных
  if(!m_os_mbx_send(TWI.MailBox, pMsg, TIMEOUT_MAILBUX_REDY, ID_TWI)){
    uiError = ERROR_DS1338_READ_4;
    goto END;
  }

  // Ожидаю подтверждение
  pMsgResponse = NULL;
  if(OS_R_TMO == os_mbx_wait(MailBox, (void **)&(pMsgResponse), TIMEOUT_RTC_RESPONSE)) {
    uiError = ERROR_DS1338_READ_5;     
    goto END;
  }

  // Структура подтверждения полностью аналогична структуре команды
  if(MSG_RESPONSE == pMsgResponse->GetFlags())
    memcpy(pcData, pMsgResponse->PointerStr() + sizeof(S_TWIMsg), ucSizeData); 
  else uiError = ERROR_DS1338_READ_6;

  pMsgResponse->Release();

END:
  return uiError; 
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
