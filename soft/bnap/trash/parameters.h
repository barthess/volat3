#ifndef PARAMETERS_H
#define PARAMETERS_H
//==============================================================================
//                                        
//==============================================================================

#include "SysConfig.h"
#include "packets_dc.h"
//------------------------------------------------------------------------------

void WriteToFlash(const char * pFL_Data, const char *pData, U16 usSize);
//----------------------------------------------------------------------------

// Таблица параметров 
const U8 TELNUMBERS = 10;
const U8 TELNUMBERSIZE = 15;

struct S_Parameters
{
  U16  usWPTimeout;         // Интервал выдачи навигационных данных
  U16  usWPDistanse;        // Расстояние выдачи навигационных данных
  U8   ucWPAzimut;          // Азимут передачи навигационных данных
  U8   ucTimeZone;          //
  U8   ucOutputState;       // Состоянме выхода
  char strzGPSPassowrd[GPSPASSWORDSIZE + 1];       // Пароль (пароль по умолчанию "123456"). 
  char strzTerminalNumber[TERMINALNUMBERSIZE +1];  // Номер терминала (например "0001")
  char strzTelephoneNumbers[TELNUMBERS][TELNUMBERSIZE + 1];
};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class C_Parameters
{
  friend C_Parameters  * CreatParameters(void);
private:
  const S_Parameters * pstParameters;
  C_Parameters(void) {} 
  C_Parameters( const S_Parameters * pstParameters_) {pstParameters = pstParameters_;} 
public:
  bool SetWPTimeout(U16 usData);
  bool SetWPDistanse(U16 usData);
  bool SetWPAzimut(U8 ucData);
  bool SetTimeZone(U8 ucData);
  bool SetOutputState(U8 ucData);
  bool SetGPSPassowrd(char * pstrz);
  bool SetTerminalNumber(char * pstrz);
  bool SetTelephoneNumber(U8 ucItem, char * pstrz);

  U16  GetWPTimeout(void) { return pstParameters->usWPTimeout; }
  U16  GetWPDistanse(void) { return pstParameters->usWPDistanse; }
  U8   GetWPAzimut(void) { return pstParameters->ucWPAzimut; }
  U8   GetTimeZone(void) { return pstParameters->ucTimeZone; }
  U8   GetOutputState(void) { return pstParameters->ucOutputState; }
  const char * GetGPSPassowrd(void) { return pstParameters->strzGPSPassowrd; }
  const char * GetTerminalNumber(void) { return pstParameters->strzTerminalNumber; }
  const char * GetTelephoneNumber(U8 ucItem) { return pstParameters->strzTelephoneNumbers[ucItem]; }
};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template <typename T>
void WriteToFlash(T& rFL_Data, T& rData) {
  WriteToFlash(reinterpret_cast<const char *>(&rFL_Data), reinterpret_cast<const char *>(rData), sizeof(T));
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

extern C_Parameters  * Parameters;
C_Parameters  * CreatParameters(void);
//----------------------------------------------------------------------------
#endif //#ifndef PARAMETERS
