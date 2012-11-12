//============================================================================
// ������ ��� ������������ � ���������� ��������� �������                                    
//============================================================================

#include <RTL.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "msg.h"
#include "terminal.h"
#include "statehandler.h"
#include "profiler.h"
//----------------------------------------------------------------------------

char * m_strtok(char * str, const char * pcDelim);
//----------------------------------------------------------------------------

//----------------  ������������ ���������� ������� --------------------------
//----------------------------------------------------------------------------
// ������ ������������� �� ������ 1 ��. (PIV + 1 = BOARD_MCK/1000/16
// ���������� ���������
//----------------------------------------------------------------------------
void InitPIT(void)
{
  const U32 uiPIV = BOARD_MCK/16000 - 1;                      
  AT91C_BASE_PITC->PITC_PIMR = (uiPIV & 0xFFFFF) | AT91C_PITC_PITEN;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

///---------------------------------------------------------------------------
/// ���������� ����� � ���� ��������� �� ������� uiOldTime
/// ��� ���������� ���������� uiTime � ������� ����������� ������� ����� 
/// ���������� ������������ �������� ������� DiffTime(). ��� ������������ � 
/// ������� ���������� (����������� �������� 1 ����) ������������ �������� ����� 
/// �������� DiffTime() �� ������ ��������� 4 ���. ������ ��������� ������������ 
/// 12-�� ���������� �������� PITC_PIVR
///---------------------------------------------------------------------------
U32 DiffTime(U32 uiOldTime)
{
   static U32 uiTime = 0;
   uiTime = uiTime + ((AT91C_BASE_PITC->PITC_PIVR) >> 20);
   U32 uiTemp;
   if(uiTime >= uiOldTime) uiTemp = uiTime - uiOldTime;
   else uiTemp = (0xFFFFFFFF - uiOldTime) + uiTime + 1;
   return uiTemp;
}
//----------------------------------------------------------------------------

void Sleep(U32 uiTime)
{
   U32 uiTimeBeg = DiffTime(0);
   while(DiffTime(uiTimeBeg) < uiTime) ;
   return;
}
//----------------------------------------------------------------------------

S_State::S_State(void) {
  memset(this, 0, sizeof(S_State));
  strzBusStop[0] = ' ';
  strzNextBusStop[0] = ' ';
  strzNumberBus[0] = ' '; 
  strcpy(strzShift, "-00:00");
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//----------------  �����  C_StateHandler  -----------------------------------

////----------------------------------------------------------------------------
//// ������� ������  ���� C_StateHandler � ���������� ���������� �� ���� 
////----------------------------------------------------------------------------
//C_StateHandler * CreateStateHandler(void){
//  static C_StateHandler StateHandler(AT91C_BASE_ADC0); 
//  // ������������ � �������� ��� 
//  StateHandler.ADCInit(ADC_SHTIM << 24  | ADC_STARTUP << 16 | ADC_PRESCAL << 8 | AT91C_ADC_SLEEP_NORMAL_MODE | AT91C_ADC_LOWRES_10_BIT | AT91C_ADC_TRGEN_DIS);
//  StateHandler.ADCChannelEnable(AT91C_ADC_CH2 | AT91C_ADC_CH1 | AT91C_ADC_CH0);
//  StateHandler.ADCStart();
//  return  &StateHandler;
//}
////++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

C_StateHandler::C_StateHandler(AT91S_ADC *pADC_) : ucActiveKey(ACTIVE_NO) {
  pADC = pADC_;
  // ������������ � �������� ��� 
  ADCInit(ADC_SHTIM << 24  | ADC_STARTUP << 16 | ADC_PRESCAL << 8 | AT91C_ADC_SLEEP_NORMAL_MODE | AT91C_ADC_LOWRES_10_BIT | AT91C_ADC_TRGEN_DIS);
  ADCChannelEnable(AT91C_ADC_CH2 | AT91C_ADC_CH1 | AT91C_ADC_CH0);
  ADCStart();
}                                        
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// ���������� � ��������� �������� ������ � ����������. 
// ��������� ��� � ��������� ��������� ���������� ������
//----------------------------------------------------------------------------
OS_TID idtInputState;
__task void Task_InputState(void)
{
  for(;;){
    os_dly_wait(10);    // 100��.

    // �������� ��������� ��������� �������� ������ � ����������
    // � ������ ������� ������� ��� ���� � ����������   State.ucDigitalInputState
    // ������� ��� ���������� ������� ��������� ������, ������� - ����������.
    U32 uiInput = *AT91C_PIOB_PDSR;
    U16  usState = 0;
    if (0 == (D_IN0 & uiInput))      usState = usState | INPUT0;  
    if (0 == (D_IN1 & uiInput))      usState = usState | INPUT1;  
    if (0 == (KBD_ALARM & uiInput))  usState = usState | KEY_ALARM; 
    if (0 == (KBD_ESC & uiInput))    usState = usState | KEY_ESC; 
    if (0 == (KBD_ENTER & uiInput))  usState = usState | KEY_ENTER; 
    if (0 == (KBD_DOWN & uiInput))   usState = usState | KEY_DOWN; 
    if (0 == (KBD_UP & uiInput))     usState = usState | KEY_UP; 
    StateHandler->DigitalInputToState(usState);

    // �������� ��������� ���������� ������
    StateHandler->AnalogInputToState(StateHandler->ADCGetData(ANALOG_0), StateHandler->ADCGetData(ANALOG_1), StateHandler->ADCGetData(ANALOG_2));
    StateHandler->ADCStart();

    //---------------------------------------------------------------------------
    // ��������� ����� ������� � ��������� OFF_TO_ON
    // ��� ������������� ������� ������ ����� ��������� ����������:
    // KEY_ALARM, KEY_ESC, KEY_ENTER, KEY_UP � ����� ������ KEY_DOWN
    //---------------------------------------------------------------------------
    if(ACTIVE_NO == StateHandler->ucActiveKey){
      U32 uiState = StateHandler->GetDigitalInputState();
      U8  ucActiveKey;
      if(KEY_ALARM == (uiState & (3*KEY_ALARM))) ucActiveKey = ACTIVE_ALARM;
      else if(KEY_ESC == (uiState & (3*KEY_ESC))) ucActiveKey = ACTIVE_ESC;
           else if(KEY_ENTER == (uiState & (3*KEY_ENTER))) ucActiveKey = ACTIVE_ENTER;
                else if(KEY_DOWN == (uiState & (3*KEY_DOWN))) ucActiveKey = ACTIVE_DOWN;
                     else if(KEY_UP == (uiState & (3*KEY_UP))) ucActiveKey = ACTIVE_UP;
                          else ucActiveKey = ACTIVE_NO;
      StateHandler->ucActiveKey = ucActiveKey;
    }
  }   
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//----------------------------------------------------------------------------
// ��������� ��������� �������� ������ � ����� ���������
//----------------------------------------------------------------------------
void  C_StateHandler::DigitalInputToState(U16 usInput){
  stState.usDigitalInputState = stState.usDigitalInputState & 0x5555;
  stState.usDigitalInputState = (stState.usDigitalInputState << 1) | usInput;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//----------------------------------------------------------------------------
// ���������� ��������� ��������� ����� � ������������ � ������������� ������
//----------------------------------------------------------------------------
bool C_StateHandler::StateKey(U16 usKey, U8 ucMask){
  U16 usTemp = stState.usDigitalInputState;
  for(U8 ucCount = 0; ucCount < 8; ++ucCount){
    if(0 != (usKey & 0x0001)) break;
    usTemp = usTemp >> 2; usKey = usKey >> 2;
  }
  if((usTemp & 0x03) == ucMask)  return true;
  else return false;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void C_StateHandler::GPSDataToState(const char* szString)
{
  // �������� ����� GPGGA
  if('G' == szString[3])  strcpy(pcGGAString, szString); 
  // �������� ����� GPRMC
  if('R' == szString[3])  strcpy(pcRMCString, szString);
   
  // ���� ������ ����� ���������� �����, �� �������� ������ � ���������� ���������
  if(0 == memcmp(pcGGAString + 7, pcRMCString + 7, 9)){
    // �������� ����� GGA
    char *pString = m_strtok(pcGGAString, ",");  // $GPGGA - ���������
    pString = m_strtok(NULL, ",");            // �����
    sscanf(pString, "%f", &stState.fTime);
    pString = m_strtok(NULL, ",");         // ������ 
    sscanf(pString, "%f", &stState.fLatitude);
    pString = m_strtok(NULL, ",");         // N/S ���������
    sscanf(pString, "%c", &stState.cNSIndicator);
    pString = m_strtok(NULL, ",");         // �������
    sscanf(pString, "%f", &stState.fLongitude);
    pString = m_strtok(NULL, ",");         // E/W ���������
    sscanf(pString, "%c", &stState.cEWIndicator);
    pString = m_strtok(NULL, ",");         // Position Fix ���������
    sscanf(pString, "%c", &stState.cPFIndicator);
    pString = m_strtok(NULL, ",");         // ����� ���������
    sscanf(pString, "%hu", &stState.usSatellites);
    pString = m_strtok(NULL, ",");         // HDOP - ���������
    pString = m_strtok(NULL, ",");         // MSL Altitude
    sscanf(pString, "%f", &stState.fAltitude);
    pString = m_strtok(NULL, ",");         // Units - ���������
    pString = m_strtok(NULL, ",");         // Geoid Separation
    sscanf(pString, "%f", &stState.fGeoid);

     // �������� ����� RMC
    pString = m_strtok(pcRMCString, ",");  // $GPRMC - ���������
    pString = m_strtok(NULL, ",");         // ����� - ���������
    pString = m_strtok(NULL, ",");         // ������
    sscanf(pString, "%c", &stState.cGPSStatus);
    pString = m_strtok(NULL, ",");         // ������ - ��������� 
    pString = m_strtok(NULL, ",");         // N/S ��������� - ���������
    pString = m_strtok(NULL, ",");         // ������� - ���������
    pString = m_strtok(NULL, ",");         // E/W ��������� - ���������
    pString = m_strtok(NULL, ",");         // ��������
    sscanf(pString, "%f", &stState.fSpeed);
    pString = m_strtok(NULL, ",");         // ����
    sscanf(pString, "%f", &stState.fCourse);
    pString = m_strtok(NULL, ",");         // ����
    sscanf(pString, "%lu", &stState.uiDate);
  }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//void C_StateHandler::GPSDataToState(C_Msg * pMsg){
//  pMsg->AddRef();
//  // �������� ������
//  if('G' == pMsg->PointerStr()[3])  /// ����� GPGGA
//	strcpy(pcGGAString, pMsg->PointerStr()); 
//  if('R' == pMsg->PointerStr()[3])  /// ����� GPRMC
//	strcpy(pcRMCString, pMsg->PointerStr()); 
//  /// ���� ������ ����� ���������� �����, �� �������� ������ � ���������� ���������
//  if(0 == memcmp(pcGGAString + 7, pcRMCString + 7, 9)){
//    // �������� ����� GGA
//    char *pString = m_strtok(pcGGAString, ",");  // $GPGGA - ���������
//    pString = m_strtok(NULL, ",");            // �����
//    sscanf(pString, "%f", &stState.fTime);
//    pString = m_strtok(NULL, ",");         // ������ 
//    sscanf(pString, "%f", &stState.fLatitude);
//    pString = m_strtok(NULL, ",");         // N/S ���������
//    sscanf(pString, "%c", &stState.cNSIndicator);
//    pString = m_strtok(NULL, ",");         // �������
//    sscanf(pString, "%f", &stState.fLongitude);
//    pString = m_strtok(NULL, ",");         // E/W ���������
//    sscanf(pString, "%c", &stState.cEWIndicator);
//    pString = m_strtok(NULL, ",");         // Position Fix ���������
//    sscanf(pString, "%c", &stState.cPFIndicator);
//    pString = m_strtok(NULL, ",");         // ����� ���������
//    sscanf(pString, "%hu", &stState.usSatellites);
//    pString = m_strtok(NULL, ",");         // HDOP - ���������
//    pString = m_strtok(NULL, ",");         // MSL Altitude
//    sscanf(pString, "%f", &stState.fAltitude);
//    pString = m_strtok(NULL, ",");         // Units - ���������
//    pString = m_strtok(NULL, ",");         // Geoid Separation
//    sscanf(pString, "%f", &stState.fGeoid);
//
//     // �������� ����� RMC
//    pString = m_strtok(pcRMCString, ",");  // $GPRMC - ���������
//    pString = m_strtok(NULL, ",");         // ����� - ���������
//    pString = m_strtok(NULL, ",");         // ������
//    sscanf(pString, "%c", &stState.cGPSStatus);
//    pString = m_strtok(NULL, ",");         // ������ - ��������� 
//    pString = m_strtok(NULL, ",");         // N/S ��������� - ���������
//    pString = m_strtok(NULL, ",");         // ������� - ���������
//    pString = m_strtok(NULL, ",");         // E/W ��������� - ���������
//    pString = m_strtok(NULL, ",");         // ��������
//    sscanf(pString, "%f", &stState.fSpeed);
//    pString = m_strtok(NULL, ",");         // ����
//    sscanf(pString, "%f", &stState.fCourse);
//    pString = m_strtok(NULL, ",");         // ����
//    sscanf(pString, "%lu", &stState.uiDate);
//  }
//  pMsg->Release();
//}
////++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void C_StateHandler::BuildWayPointPacket(S_WayPointPacket& rPacket, U16 usNumberPoint_){
  rPacket.uiData = stState.uiDate;                // ���� ddmmyy (�������� 250109 - 25/01/2009)
  rPacket.uiTime = (U32)(stState.fTime * 1000.);  // ����� hhmmsssss (�������� 234859000 - 23:48:59.000)
  rPacket.fLat = stState.fLatitude;               // ������ ddmm.mmmm(�������� 5352.2134 - 53?53.2134')
  if('S' == stState.cNSIndicator) rPacket.fLat = -rPacket.fLat;
  rPacket.fLon = stState.fLongitude;              // ������� ddmm.mmmm(�������� 2752.2134 - 27?53.2134')
  if('W' == stState.cEWIndicator) rPacket.fLon = -rPacket.fLon;
  rPacket.usAlt = (U16)(stState.fAltitude + stState.fGeoid);  // ������ � ������
  rPacket.usCourse = (U16)stState.fCourse;        // ���� � ��������
  rPacket.usSpeed = (U16)stState.fSpeed;          // �������� � ���������� � ���
  rPacket.usNumberPoint = usNumberPoint_;         /// ����� ������� ����� (������������ � ������ �������������) 
  rPacket.uiDistanse = 0;                         // �� ������������
  rPacket.fPower  = stState.usAnalogInput0;       // �������� ������� � �������
  rPacket.pusAi[0] = stState.usAnalogInput1;      // ��������� ���������� ������
  rPacket.pusAi[1] = stState.usAnalogInput2;
  // ������������ ���������� ���������
  rPacket.uiStatus = 0;
  if(StateKey(KEY_ALARM, KEY_ON)) rPacket.uiStatus  = rPacket.uiStatus | STATE_ALARM; /// ������� ��� - ��������� ��������� ������ (0x01) 
  // ������������ ��������� �������� ������
  rPacket.ucDi = 0;
  if(StateKey(INPUT0, KEY_ON)) rPacket.ucDi = rPacket.ucDi | 0x01;
  if(StateKey(INPUT1, KEY_ON)) rPacket.ucDi = rPacket.ucDi | 0x02;

}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

C_Msg * C_StateHandler::InputStateToText(void){
  C_Msg * pMsg  = NewMsg(ID_LINKPC, ID_MANAGEMENT, NOMAILBOX, MSG_RESPONSE, 256);
  if(NULL != pMsg){ 
    char strz[] = "0   0   0   0   0   0   0";
    strz[0]  = DigitalStateToChar(stState.usDigitalInputState);
    strz[4]  = DigitalStateToChar(stState.usDigitalInputState >> 2);
    strz[8]  = DigitalStateToChar(stState.usDigitalInputState >> 4);
    strz[12]  = DigitalStateToChar(stState.usDigitalInputState >> 6);
    strz[16]  = DigitalStateToChar(stState.usDigitalInputState >> 8);
    strz[20] = DigitalStateToChar(stState.usDigitalInputState >> 10);
    strz[24] = DigitalStateToChar(stState.usDigitalInputState >> 12);
    sprintf(pMsg->PointerStr(), "\r���������� �����:  A0 = %4u,   A1 = %4u,   A2 = %4u\r�������� �����:\rAl  En  Es  Up  Dn  I0  I1\r%s\r\r",
    stState.usAnalogInput0, stState.usAnalogInput1, stState.usAnalogInput2, strz);
    pMsg->SetLength(strlen(pMsg->PointerStr()));
  }		 
  return pMsg;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

char C_StateHandler::DigitalStateToChar(U16 us){
  char cX;
  switch(us & 0x3){
    case 0: cX = '0'; break;
    case 1: cX = 'U'; break;
    case 2: cX = 'D'; break;
    case 3: cX = '1'; break;
  }
  return cX;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

C_Msg * C_StateHandler::GPSStateToText(void){
  C_Msg * pMsg  = NewMsg(ID_LINKPC, ID_MANAGEMENT, NOMAILBOX, MSG_RESPONSE, 256);
  if(NULL != pMsg){ 
    sprintf(pMsg->PointerStr(), "\r\
���������� %c   ���� %2u.%2u.%02u    ����� %2u:%2u:%02u\r\
������  %.4f-%c\r\
������� %.4f-%c\r\
������  %.1f   ���������� ������ %.1f\r\
�������� %.3f  ���� %.2f\r\
Position Fix %c   ����� ��������� %2u\r",\
    stState.cGPSStatus, stState.uiDate/10000, (stState.uiDate % 10000)/100, stState.uiDate % 100,\
	  int(stState.fTime)/10000, (int(stState.fTime) % 10000)/100, int(stState.fTime) % 100,\
	  stState.fLatitude, stState.cNSIndicator, stState.fLongitude, stState.cEWIndicator,\
	  stState.fAltitude, stState.fGeoid, stState.fSpeed, stState.fCourse,\
	  stState.cPFIndicator, stState.usSatellites);

   pMsg->SetLength(strlen(pMsg->PointerStr()));
  }		 
  return pMsg;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

