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
// ������� ������  ���� C_Parameters � ���������� ���������� �� ���� 
//------------------------------------------------------------------------------
C_Parameters  * CreatParameters(void){
  static const S_Parameters pstParameters = {1, 0, 0, 2, 0, {'1', '2', '3', '4', '5', '6', 0}, {'1', '3', '4', 0}};  /// ��������� ������� 

  static C_Parameters Parameters(&pstParameters); 
  return  &Parameters;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool C_Parameters::SetWPTimeout(U16 usData){
  // �������� � �������� � ���
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
  /// ������������ ����� ������ GPSPASSWORDSIZE 
  if(GPSPASSWORDSIZE < ucSize) return false;
  WriteToFlash(reinterpret_cast<const char *>(&(pstParameters->strzGPSPassowrd)), 
               reinterpret_cast<const char *>(pcInput), ucSize + 1);
  return true;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool C_Parameters::SetTerminalNumber(char * pcInput){
  U8 ucSize = strlen(pcInput);  
  /// ������������ ����� ������ ��������� TERMINALNUMBERSIZE 
  if(TERMINALNUMBERSIZE < ucSize) return false;
  WriteToFlash(reinterpret_cast<const char *>(&(pstParameters->strzTerminalNumber)), 
               reinterpret_cast<const char *>(pcInput), ucSize + 1); // ������� � ���������� 0
  return true;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool C_Parameters::SetTelephoneNumber(U8 ucItem, char * pcInput) {
  /// ������������ ����� ���������� ������� TELNUMBERS
  if(TELNUMBERS <= ucItem) return false;
  /// ��������� ������� � ����� �������� � ������������ ��������� �� 1-� ������ 
  /// ������ ���������� ����� ��������
  pcInput = FindNextArg(pcInput, ARG_DELIMITER, 2);  
  /// �� ����� ��������� ������������ ��������� 0 
  U8 ucSize = strlen(pcInput);  pcInput[ucSize] = 0;
  /// ������������ ����� ������ �������� TELNUMBERSIZE 
  if(TERMINALNUMBERSIZE < ucSize) return false;
  WriteToFlash(reinterpret_cast<const char *>(pstParameters->strzTelephoneNumbers[ucItem]), 
               reinterpret_cast<const char *>(pcInput), ucSize);
  return true;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
