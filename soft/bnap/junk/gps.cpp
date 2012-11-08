//============================================================================
// ������� GPS ���������                                     
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
// ��������� ������ ��� ������: GPGGA � GPRMC
// ������� ���������� ������������ ���������� ��������� ����������������� 
// ���������� � �������� ��������� GPS. 
//---------------------------------------------------------------------------
bool C_GPS::GetData(char ch)
{
  // ������ ������ ������
  if((0 == usCount) & ('$' != ch)) return false;
  pcInputBuf[usCount] = ch;
  if(usCount == 5){
    pcInputBuf[6] = 0;
    int iGGA = strcmp(pcInputBuf, "$GPGGA");
    int iRMC = strcmp(pcInputBuf, "$GPRMC");
    // ���� "�����" ����� �� �������� �����
    if((0 != iGGA) && (0 != iRMC)){ usCount = 0; return false;}
  }

  // ���� ����� ������ ���������, �� ������������ 
  // ���� ���������� ��� ������ GPS_Rx 
  // ����� ������ ��������� ��� ������� 6 ��������(������ �����)
  if(5 >= usCount) goto GetData_1;
  if(0xD != pcInputBuf[usCount]) goto GetData_1;
  ++usCount;

  // ������������ ��������� �� ��������� �����
  if(pcInputBuf == pcDataBuf_0){ pcInputBuf = pcDataBuf_1; pcBufOut = pcDataBuf_0;}
  else                         { pcInputBuf = pcDataBuf_0; pcBufOut = pcDataBuf_1;}
  usCountChar = usCount;                               
  usCount = 0;              // �������� ���� ���������
  return true; 
   
GetData_1:
  ++usCount;  
  // ����� ����������
  if((GPS_DATA_BUF_SIZE - 1) == usCount)  usCount = 0;  
  return false;
}
//----------------------------------------------------------------------------


//---------------------------------------------------------------------------
// ������� ����������� ������������� ������� � ���������� ��� �� 
//---------------------------------------------------------------------------
__task void Task_GPS_Rx(void) 
{
  // �������� ������ ���������
  GPS.usCountChar = 0;
  GPS.usart->ReceiverStart();

  for(;;){
    os_evt_wait_or(RX_REDY, 0xFFFF);
    // �������� ���������� ������
    GPS.pcBufOut[GPS.usCountChar] = 0;  
    StateHandler->GPSDataToState(GPS.pcBufOut);

    // ���� ������ �� �������, �� ��������� ����� �� RTC
    if(!StateHandler->GetGPSStatus()){
      float fTime; U32 uiDate;
      // ������� ����� �� RTC
      RTC1338.TimeToUTC(&fTime,&uiDate);
      // �������� ����� � ��������� �������.
      StateHandler->SetTimeUTC(fTime, uiDate);
    }

    // �����������
    if(GPS.IsDiagnosticsRx()) print_text(GPS.pcBufOut);   
  }
}
//----------------------------------------------------------------------------
