//============================================================================
// ������� RTC DC1338                                     
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
    // ������������ ��������  RTC1338 � ��������� tm
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
  pc[0]  =  pstTime->tm_sec  % 10;         /// ������� ������
  pc[0] += (pstTime->tm_sec  / 10) << 4;   /// ������� ������
  pc[1]  =  pstTime->tm_min  % 10;         /// ������� �����
  pc[1] += (pstTime->tm_min  / 10) << 4;   /// ������� �����
  pc[2]  =  pstTime->tm_hour % 10;         /// ������� �����
  pc[2] += (pstTime->tm_hour / 10) << 4;   /// ������� �����
  pc[3]  =  pstTime->tm_wday + 1;
  pc[4]  =  pstTime->tm_mday % 10;         /// ������� ����
  pc[4] += (pstTime->tm_mday / 10) << 4;   /// ������� ����
  pstTime->tm_mon = pstTime->tm_mon + 1;   /// ������������ ��������  RTC1338 � ��������� tm
  pc[5]  =  pstTime->tm_mon  % 10;         /// ������� �������
  pc[5] += (pstTime->tm_mon  / 10) << 4;   /// ������� �������
  pstTime->tm_year = pstTime->tm_year - 100;
  pc[6]  =  pstTime->tm_year % 10;         /// ������� ���
  pc[6] += (pstTime->tm_year / 10) << 4;   /// ������� ���
  U16 usError = Write(0, pc, 7);
  return usError;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
// ������ ������: DD.MM.YY  hh.mm.ss
//---------------------------------------------------------------------------
U16 C_RTC1338::SetTimeFromStr(char *pstrzTime){
  char pch[3];
  tm stTime;
  while(' ' == *pstrzTime) ++pstrzTime;   /// ������ ��������� �������

  /// ������� ����
  pch[0] = *pstrzTime;   ++pstrzTime;
  pch[1] = *pstrzTime;   ++pstrzTime;
  pch[2] = 0;
  if( 1 != sscanf(pch, "%u", &stTime.tm_mday)) return ERROR_DS1338_SSCANF; /// ������ � ���������
  /// ������� �����
  ++pstrzTime;            /// ��������� '.'
  pch[0] = *pstrzTime;   ++pstrzTime;
  pch[1] = *pstrzTime;   ++pstrzTime;
  pch[2] = 0;
  if( 1 != sscanf(pch, "%u", &stTime.tm_mon)) return ERROR_DS1338_SSCANF; /// ������ � ���������
  stTime.tm_mon = stTime.tm_mon - 1;   /// ������������ �������� ���� ����� ����� ���������� � 1 
  /// ������� ���
  ++pstrzTime;            /// ��������� '.'
  pch[0] = *pstrzTime;   ++pstrzTime;
  pch[1] = *pstrzTime;   ++pstrzTime;
  pch[2] = 0;
  if( 1 != sscanf(pch, "%u", &stTime.tm_year)) return ERROR_DS1338_SSCANF; /// ������ � ���������
  stTime.tm_year += 100;  /// ������ �� 1900 ����. ����������� �������� ��� tm_year 70

  while(' ' == *pstrzTime) ++pstrzTime;   /// ������ ������� ����������� ���� � �����

  /// ������� ���
  pch[0] = *pstrzTime;   ++pstrzTime;
  pch[1] = *pstrzTime;   ++pstrzTime;
  pch[2] = 0;
  if( 1 != sscanf(pch, "%u", &stTime.tm_hour)) return ERROR_DS1338_SSCANF; /// ������ � ���������
  /// ������� ������
  ++pstrzTime;            /// ��������� ':'
  pch[0] = *pstrzTime;   ++pstrzTime;
  pch[1] = *pstrzTime;   ++pstrzTime;
  pch[2] = 0;
  if( 1 != sscanf(pch, "%u", &stTime.tm_min)) return ERROR_DS1338_SSCANF; /// ������ � ���������
   /// ������� �������
  pstrzTime++;            /// ��������� ':'
  pch[0] = *pstrzTime;   ++pstrzTime;
  pch[1] = *pstrzTime;   ++pstrzTime;
  pch[2] = 0;
  if( 1 != sscanf(pch, "%u", &stTime.tm_sec)) return ERROR_DS1338_SSCANF; /// ������ � ���������

  time_t t = mktime(&stTime);
  U16 usError = SetTime(t);
  return usError;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

///---------------------------------------------------------------------------
/// ������ ����:     U32   DDMMYY
/// ������ �������:  float hhmmss.sss
///---------------------------------------------------------------------------
U16 C_RTC1338::SetTimeFromUTC(float fTime, U32 uiDate){
  tm stTime;

  /// ������� ����
  stTime.tm_year = uiDate % 100;  uiDate = uiDate/100;
  stTime.tm_year += 100;  /// ������ �� 1900 ����. ����������� �������� ��� tm_year 70
  stTime.tm_mon  = uiDate % 100; 
  stTime.tm_mday = uiDate/100;

  /// ������� �����
  uiDate = fTime;  // ���������� ������������
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
  t = t - uiTimeZone;    // ��������� GetTime() ���������� ���������������� �����
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
  // � ��������� S_TWIMsg ������� ��������� ������ I2C ����������
  pMsg = NewMsg(ID_TWI, ID_RTC1338, MailBox, MSG_CMDWITHRESPONSE, sizeof(S_TWIMsg) + ucSizeData);

  TWIMsg = (S_TWIMsg *)pMsg->PointerStr();
  TWIMsg->usSCLFrequency = usSCLFrequency;
  TWIMsg->ucAddresSlave  = ucAddresSlave;
  TWIMsg->ucAddressSizeAndOper  = ucAddressSizeAndOper;
  TWIMsg->uiInternalAddess = ucAddress; 
  memcpy(pMsg->PointerStr() + sizeof(S_TWIMsg), pcData, ucSizeData);  

  // ��������� ������� ������ ������ ������
  if(!m_os_mbx_send(TWI.MailBox, pMsg, TIMEOUT_MAILBUX_REDY, ID_TWI)){ 
    uiError = ERROR_DS1338_WRITE_4;
    goto END;
  }
END:
  return uiError; 
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//----------------------------------------------------------------------------
// �� ����������� ��������� ����� ���������� �������� ������ ��������� 2 ��.
// ��������� ������ TWI = 2, ��������� ������� 1
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

  // � ��������� S_TWIMsg ������� ��������� ������ I2C ����������
  pMsg = NewMsg(ID_TWI, ID_RTC1338, MailBox, MSG_CMDWITHOUTRESPONSE, sizeof(S_TWIMsg) + ucSizeData);

  TWIMsg = (S_TWIMsg *)pMsg->PointerStr();
  TWIMsg->usSCLFrequency = usSCLFrequency;
  TWIMsg->ucAddresSlave  = ucAddresSlave;
  TWIMsg->ucAddressSizeAndOper  = ucAddressSizeAndOper | OPER_READ;
  TWIMsg->uiInternalAddess = ucAddress; 

  // ��������� ������� ������ ������
  if(!m_os_mbx_send(TWI.MailBox, pMsg, TIMEOUT_MAILBUX_REDY, ID_TWI)){
    uiError = ERROR_DS1338_READ_4;
    goto END;
  }

  // ������ �������������
  pMsgResponse = NULL;
  if(OS_R_TMO == os_mbx_wait(MailBox, (void **)&(pMsgResponse), TIMEOUT_RTC_RESPONSE)) {
    uiError = ERROR_DS1338_READ_5;     
    goto END;
  }

  // ��������� ������������� ��������� ���������� ��������� �������
  if(MSG_RESPONSE == pMsgResponse->GetFlags())
    memcpy(pcData, pMsgResponse->PointerStr() + sizeof(S_TWIMsg), ucSizeData); 
  else uiError = ERROR_DS1338_READ_6;

  pMsgResponse->Release();

END:
  return uiError; 
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
