#ifndef STATEHANDLER_H
#define STATEHANDLER_H
//============================================================================
//
//============================================================================

#include <RTL.h>
#include <string.h>
#include "SysConfig.h"
#include "packets_dc.h"
//----------------------------------------------------------------------------


//-------------------  Класс   C_ADC  ----------------------------------------
//----------------------------------------------------------------------------
// Класс для работы с АЦП
//----------------------------------------------------------------------------
class C_ADC  
{
protected:
  AT91S_ADC *pADC;  // Базовай адрес АЦП
  C_ADC() : pADC(NULL) {}
public:
  void ADCStart(void) { pADC->ADC_CR = AT91C_ADC_START; }
  void ADCReset(void){	pADC->ADC_CR = AT91C_ADC_SWRST; }
  void ADCChannelEnable(U8 ucChannel){ pADC->ADC_CHER = ucChannel;}
  void ADCChannelDisable(U8 ucChannel){ pADC->ADC_CHER = ucChannel;}
  U32  ADCGetData(U8 ucChannel){return ((U32 *)&(pADC->ADC_CDR0))[ucChannel];}
  void ADCInit(U32 uiMode){pADC->ADC_MR = uiMode;}
};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//-------------------  Класс   C_StateHandler  -------------------------------
//----------------------------------------------------------------------------
// Используется для хранения состояния системы.
//----------------------------------------------------------------------------
const U8 LENGH_NUMBER_BUS = 9;
const U8 LENGH_SHIFT = 6;
const U8 LENGH_NAME_BUS_STOP = 30;

struct S_State
{
  U32   uiDate;              // Дата 
  float fTime;               // Время  
  float fLatitude;           // Широта
  float fLongitude;          // Долгота
  float fAltitude;
  float fGeoid;
  float fSpeed;              // Скорость
  float fCourse;             // Курс    
  U16   usDigitalInputState; // Состояние цифровых входов
  U16   usSatellites;
  U16   usAnalogInput0;
  U16   usAnalogInput1;
  U16   usAnalogInput2;
  char  cNSIndicator;
  char  cEWIndicator;
  char  cPFIndicator;
  char  cGPSStatus;           // Состояние GPS соединения
  bool  boVoiseStatus;        // Состояние голосовой связи
  bool  boTCPStatus;          // Состояние TCP соединения
  bool  boUpdateStatus;       // Обновить экран
  char strzBusStop[LENGH_NAME_BUS_STOP + 1];
  char strzNextBusStop[LENGH_NAME_BUS_STOP + 1];
  char strzNumberBus[LENGH_NUMBER_BUS + 1];
  char strzShift[LENGH_SHIFT + 1];

  S_State(void);
};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//----------------------------------------------------------------------------
// Сохраняет и обрабатывает состояние системы                                    
//----------------------------------------------------------------------------
class C_StateHandler : public C_ADC
{
friend C_StateHandler * CreateStateHandler(void);
friend __task void Task_InputState(void);
private:
  S_State stState;
  U8 ucActiveKey;
  char pcGGAString[GPS_DATA_BUF_SIZE];                                           
  char pcRMCString[GPS_DATA_BUF_SIZE];   
  C_StateHandler();                                        
public:
  C_StateHandler(AT91S_ADC *pADC_);
//  void  GPSDataToState(C_Msg * pMsg);
  void  GPSDataToState(const char* szString);
  void  AnalogInputToState(U16 usAnalog0, U16 usAnalog1, U16 usAnalog2){stState.usAnalogInput0 = usAnalog0; 
                          stState.usAnalogInput1 = usAnalog1; stState.usAnalogInput2 = usAnalog2;} 
  void  DigitalInputToState(U16 usInput);
  void  SetTimeUTC(float fTime, U32 uiDate){stState.fTime = fTime; stState.uiDate = uiDate;}
  void  VocesToState(bool  boStatus) {stState.boVoiseStatus = boStatus;}
  void  SetTCPStatus(void){if(stState.boTCPStatus) return; stState.boTCPStatus = true; stState.boUpdateStatus = true; }
  void  ResetTCPStatus(void){ if(!stState.boTCPStatus) return; stState.boTCPStatus = false; stState.boUpdateStatus = true;}
  bool  GetTCPStatus(void){return stState.boTCPStatus;}
  void  SetBusStop(const char *strz) {strncpy(stState.strzBusStop, strz, LENGH_NAME_BUS_STOP); stState.boUpdateStatus = true;}
  void  SetNextBusStop(const char *strz) {strncpy(stState.strzNextBusStop, strz, LENGH_NAME_BUS_STOP); stState.boUpdateStatus = true;}
  void  SetNumberBus(const char *strz) {strncpy(stState.strzNumberBus, strz, LENGH_NUMBER_BUS); stState.boUpdateStatus = true;}
  void  SetShift(const char *strz) {strncpy(stState.strzShift, strz, LENGH_SHIFT); stState.boUpdateStatus = true;}
  void  ResetUpdateStatus(void){ stState.boUpdateStatus = false;}
                                                                   
  U16   GetDigitalInputState(void){ return stState.usDigitalInputState;}
  bool  GetGPSStatus(void){return 'A' == stState.cGPSStatus;}
  bool  GetVoiseStatus(void){return stState.boVoiseStatus;}
  bool  GetUpdateStatus(void){return stState.boUpdateStatus;}
  float GetTime(void) {return stState.fTime;}
  U32   GetDate(void) {return stState.uiDate;}
  const char * GetBusStop(void) {return stState.strzBusStop;}
  const char * GetNextBusStop(void) {return stState.strzNextBusStop;}
  const char * GetNumberBus(void) {return stState.strzNumberBus;}
  const char * GetShift(void) {return stState.strzShift;}

  U8    GetActiveKey(void){return ucActiveKey;}
  void  ClearActiveKey(void){ucActiveKey = ACTIVE_NO;}

  void  BuildWayPointPacket(S_WayPointPacket& rPacket, U16 usNumberPoint_);
  char  DigitalStateToChar(U16 us);
  C_Msg * GPSStateToText(void);
  bool StateKey(U16 usKey, U8 ucMask);
  C_Msg * InputStateToText(void);
};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__task void Task_InputState(void);
extern OS_TID idtInputState;
extern void InitPIT(void);
extern U32  DiffTime(U32 uiOldTime);
extern C_StateHandler * StateHandler;
C_StateHandler * CreateStateHandler(void);
//----------------------------------------------------------------------------

#endif //#ifndef STATEHANDLER_H
