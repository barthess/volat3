//============================================================================
//                                        
//============================================================================

#include <string.h>
#include <stdio.h>
#include "parameters.h"
//----------------------------------------------------------------------------

char * FindNextArg(char *str, char cDelim, U8 ucCount = 1);
//----------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Создает объект  типа C_Parameters и возвращает укказатель на него 
//------------------------------------------------------------------------------
C_Parameters  * CreatParameters(void){
  static const S_Parameters pstParameters = {1, 0, 0, 2, 0, {'1', '2', '3', '4', '5', '6', 0}, {'1', '3', '4', 0}};  /// Параметры системы 

  static C_Parameters Parameters(&pstParameters); 
  return  &Parameters;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool C_Parameters::SetWPTimeout(U16 usData){
  // Принимаю и сохраняю в сек
  if(0 == usData) usData = 0xFFFF; 
  WriteToFlash(reinterpret_cast<const char *>(&(pstParameters->usWPTimeout)), 
               reinterpret_cast<const char *>(&usData), sizeof(usData));
  return true;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool C_Parameters::SetWPDistanse(U16 usData){
  WriteToFlash(reinterpret_cast<const char *>(&(pstParameters->usWPDistanse)), 
               reinterpret_cast<const char *>(&usData), sizeof(usData));
  return true;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool C_Parameters::SetWPAzimut(U8 ucData){
  WriteToFlash(reinterpret_cast<const char *>(&(pstParameters->ucWPAzimut)), 
               reinterpret_cast<const char *>(&ucData), sizeof(ucData));
  return true;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool C_Parameters::SetTimeZone(U8 ucData){
  WriteToFlash(reinterpret_cast<const char *>(&(pstParameters->ucTimeZone)), 
               reinterpret_cast<const char *>(&ucData), sizeof(ucData));
  return true;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool C_Parameters::SetOutputState(U8 ucData){
  WriteToFlash(reinterpret_cast<const char *>(&(pstParameters->ucOutputState)), 
               reinterpret_cast<const char *>(&ucData), sizeof(ucData));
  return true;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool C_Parameters::SetGPSPassowrd(char * pcInput){
  U8 ucSize = strlen(pcInput);
  /// Максимальная длина пароля GPSPASSWORDSIZE 
  if(GPSPASSWORDSIZE < ucSize) return false;
  WriteToFlash(reinterpret_cast<const char *>(&(pstParameters->strzGPSPassowrd)), 
               reinterpret_cast<const char *>(pcInput), ucSize + 1);
  return true;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool C_Parameters::SetTerminalNumber(char * pcInput){
  U8 ucSize = strlen(pcInput);  
  /// Максимальная длина номера терминала TERMINALNUMBERSIZE 
  if(TERMINALNUMBERSIZE < ucSize) return false;
  WriteToFlash(reinterpret_cast<const char *>(&(pstParameters->strzTerminalNumber)), 
               reinterpret_cast<const char *>(pcInput), ucSize + 1); // Копирую и завешающий 0
  return true;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool C_Parameters::SetTelephoneNumber(U8 ucItem, char * pcInput) {
  /// Максимальное число телефонных номеров TELNUMBERS
  if(TELNUMBERS <= ucItem) return false;
  /// Пропускаю команду и номер элемента и устанавливаю указатель на 1-й символ 
  /// строки содержащей номер телефона
  pcInput = FindNextArg(pcInput, ARG_DELIMITER, 2);  
  /// На место конечного ограничителя запичываю 0 
  U8 ucSize = strlen(pcInput);  pcInput[ucSize] = 0;
  /// Максимальная длина номера телефона TELNUMBERSIZE 
  if(TERMINALNUMBERSIZE < ucSize) return false;
  WriteToFlash(reinterpret_cast<const char *>(pstParameters->strzTelephoneNumbers[ucItem]), 
               reinterpret_cast<const char *>(pcInput), ucSize);
  return true;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
