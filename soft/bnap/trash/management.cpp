///============================================================================
/// ������ ����������
/// ������������ ��� ��������� ����������� ���������� ������.                                     
//============================================================================

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "errorhandler.h"
#include "usart1.h"
#include "usart2.h"
#include "uartd.h"
#include "gps.h"
#include "gsm.h"
#include "linkpc.h"
#include "statehandler.h"
#include "dc.h"
#include "management.h"
#include "lcd.h"
#include "terminal.h"
#include "rtc1338.h"
#include "parameters.h"
//----------------------------------------------------------------------------

char szVersion[] = "V0.6/22.09.2011";
char szSerialNumber[8] = "001";
C_Management Management;
OS_TID idtManagement;
char strz80[80];
char strz82[86];
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// ������ ��������
//----------------------------------------------------------------------------
static volatile int iDelay;
void m_delay(U16 i){
  while(0 != i){ iDelay = iDelay * iDelay; --i; }
}
//----------------------------------------------------------------------------

//------------------------------------------------------------------------------
// ���������� ������ usLength �������� ���� ������ �������� ��� ����� ��������
// ������������ ��������: true ���� ������ usLength �������� ���������
//                        false ���� ������ usLength �������� �� ��������� ���
//                        ���� �� ����� ��� ��� ������ ������ usLength 
//------------------------------------------------------------------------------
bool strncmp2lower(const char *strz1, const char *strz2, U16 usLength){
  U16 i;
  for(i = 0; i < usLength; ++i){
    if(tolower(strz1[i]) != tolower(strz2[i])) return false;
    if(0 == strz1[i]) return false;
    if(0 == strz2[i]) return false;
  }
  return true;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//------------------------------------------------------------------------------
// ������� � ������ str ������ ������ �����������. ����� ����� ������������� 
// ��������� ��  ������ ������ �������� �� �����������. 
// ucCount - ����� �������� ������������� ��������. ������������ ��� ����������� 
//           ��������� �� ��������� ����������
// 30.10.2010
//------------------------------------------------------------------------------
char * FindNextArg(char *str, char cDelim, U8 ucCount = 1){
  for(U8 uc = 0; uc < ucCount; ++uc){
    str = strchr(str, cDelim);
    if(NULL != str){
      str = str + 1;
  	while(cDelim == *str) str = str + 1;
      if(0 == *str) return NULL;
    }
  }
  return str;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//------------------------------------------------------------------------------
// ��������� ���� �� �� ������� ������ ��������� �������.
// pcInput ��������� �� ������ ������� 
// ����� ����� ������� ������ ���� ������ ��� ����������� ������ 0
//------------------------------------------------------------------------------
bool IsCMD(const char *strzCMD, char  * pcInput){
  U16 usLength = strlen(strzCMD);
  for(U16 i = 0; i < usLength; ++i)
    if(tolower(strzCMD[i]) != tolower(pcInput[i])) return false;
  
  if(ARG_DELIMITER == pcInput[usLength]) return true;
  if(0 == pcInput[usLength]) return true;
  return false;  // ����� ������� ������� ��� ����������� ��� ������������ 0
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//------------------------------------------------------------------------------
// ��������� ���� �� �� ������� ������ ��������� �������.
// ������� �������� ���� �������� - ����� ����� 
// �������� ���������� �� ������� ucArgumentDelimiter
// pcInput - ��������� �� ������ � ��������
//------------------------------------------------------------------------------
bool IsCMDwithArg(const char *strzCMD, char *pcInput, U32* puiArg){
  if(!IsCMD(strzCMD, pcInput)) return false;            // ������� �� ����������������
  // ������������ ��������� �� 1-� ������ ���������
  pcInput = FindNextArg(pcInput, ARG_DELIMITER);  
  if( 1 != sscanf(const_cast<char *>(pcInput), "%lu", puiArg)) return false; // ������ � ���������
  return true;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//------------------------------------------------------------------------------
// ��������� ���� �� �� ������� ������ ��������� �������.
// ������� �������� ���� �������� - ����� � ������� float
// pcInput[0] �������� ������ ���������� ������������
//------------------------------------------------------------------------------
bool IsCMDwithArg(const char *strzCMD, char *pcInput, float* pfArg){
  if(!IsCMD(strzCMD, pcInput)) return false;            // ������� �� ����������������
  // ������������ ��������� �� 1-� ������ ���������
  pcInput = FindNextArg(pcInput, ARG_DELIMITER);  
  if( 1 != sscanf(const_cast<char *>(pcInput), "%f", pfArg)) return false;  // ������ � ���������
  return true;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

char * m_strtok(char * str, const char * pcDelim){
  static char * pstr = NULL;
  static char * pEnd = NULL;

  if(NULL != str)  pstr = str;
  else {
    if(NULL != pEnd) pstr = pEnd + 1;
    else return NULL;              /// ������ ������ ��������
  }
  pEnd = strchr(pstr, *pcDelim);   /// ������������ ��������� �� 1-� �����������
  if(NULL != pEnd) *pEnd = 0;      /// ����� �������

  return pstr;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//------------------------------------------------------------------------------
// �������� ����� � ���� ����������. 
//------------------------------------------------------------------------------
void PrintResponse(U8 ucError, const char * strz){
  // �������� ��������� ����
  strz82[0] = START_DELIMITER;
  strz82[1] = ucError;
  U8 ucLength =  strlen(strz);
  if(0 == ucLength){
    strz82[2] = END_DELIMITER;
    strz82[3] = '\r';
    strz82[4] = 0;
  }
  else {
    // ucStartDelimiter, ucError, ucArgumentDelimiter, ucEndDelimiter, '\r', 0
    // ����� 6 ��������
    strz82[2] = ARG_DELIMITER;
    strz82[3] = 0;
    strncat(strz82, strz, sizeof(strz82) - 6); 
    ucLength =  strlen(strz82);
    strz82[ucLength] = END_DELIMITER;
    strz82[ucLength + 1] = '\r';
    strz82[ucLength + 2] = 0;
  }
  print_text(strz82); 
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void MAX9814_On(void){
    const Pin Pins[] = {PIN_MAX9814_SD, AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_OUTPUT_1, PIO_DEFAULT}; 
    PIO_Configure(Pins, PIO_LISTSIZE(Pins));   
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void MAX9814_Off(void){
    const Pin Pins[] = {PIN_MAX9814_SD, AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_OUTPUT_0, PIO_DEFAULT}; 
    PIO_Configure(Pins, PIO_LISTSIZE(Pins));   
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void MAX9814_SetAR500(void){
    const Pin Pins[] = {PIN_MAX9814_AR, AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_OUTPUT_0, PIO_DEFAULT}; 
    PIO_Configure(Pins, PIO_LISTSIZE(Pins));   
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void MAX9814_SetAR2000(void){
    const Pin Pins[] = {PIN_MAX9814_AR, AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_OUTPUT_1, PIO_DEFAULT}; 
    PIO_Configure(Pins, PIO_LISTSIZE(Pins));   
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void MAX9814_SetAR4000(void){
    const Pin Pins[] = {PIN_MAX9814_AR, AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_INPUT, PIO_DEFAULT}; 
    PIO_Configure(Pins, PIO_LISTSIZE(Pins));   
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void MAX9814_SetGain40(void){
    const Pin Pins[] = {PIN_MAX9814_GAIN, AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_OUTPUT_1, PIO_DEFAULT}; 
    PIO_Configure(Pins, PIO_LISTSIZE(Pins));   
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void MAX9814_SetGain50(void){
    const Pin Pins[] = {PIN_MAX9814_GAIN, AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_OUTPUT_0, PIO_DEFAULT}; 
    PIO_Configure(Pins, PIO_LISTSIZE(Pins));   
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void MAX9814_SetGain60(void){
    const Pin Pins[] = {PIN_MAX9814_GAIN, AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_INPUT, PIO_DEFAULT}; 
    PIO_Configure(Pins, PIO_LISTSIZE(Pins));   
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void TDA8551_GainUp(void){
    const Pin Pins_T[] = {PIN_TDA8551_GAIN, AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_INPUT, PIO_DEFAULT};
    PIO_Configure(Pins_T, PIO_LISTSIZE(Pins_T));   
    const Pin Pins[] = {PIN_TDA8551_GAIN, AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_OUTPUT_1, PIO_DEFAULT}; 
    PIO_Configure(Pins, PIO_LISTSIZE(Pins)); 
    m_delay(100);  
    PIO_Configure(Pins_T, PIO_LISTSIZE(Pins_T));   
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void TDA8551_GainDown(void){
    const Pin Pins_T[] = {PIN_TDA8551_GAIN, AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_INPUT, PIO_DEFAULT};
    PIO_Configure(Pins_T, PIO_LISTSIZE(Pins_T));   
    const Pin Pins[] = {PIN_TDA8551_GAIN, AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_OUTPUT_0, PIO_DEFAULT}; 
    PIO_Configure(Pins, PIO_LISTSIZE(Pins)); 
    m_delay(100);  
    PIO_Configure(Pins_T, PIO_LISTSIZE(Pins_T));   
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void TDA8551_On(void){
    const Pin Pins[] = {PIN_TDA8551_MODE, AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_OUTPUT_0, PIO_DEFAULT}; 
    PIO_Configure(Pins, PIO_LISTSIZE(Pins));   
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void TDA8551_Off(void){
    const Pin Pins[] = {PIN_TDA8551_MODE, AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_OUTPUT_1, PIO_DEFAULT}; 
    PIO_Configure(Pins, PIO_LISTSIZE(Pins));   
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


//---------------------------------------------------------------------------
// ���� ��������� �� ������� ���������, �� ��� ��������� � ����������� ���������
// �� ������.
// ���� ��������� �� ��������� == NULL ������� �� ���� �� ��������� � ���������� false  
// ID_Unit - ������������� ������ � ������� ��������� ������
//---------------------------------------------------------------------------
bool m_os_mbx_send (OS_ID mailbox, void* message_ptr, U16   timeout, U16 ID_Unit )
{
   if(NULL == message_ptr) return false;

   if(NULL == mailbox){ 
     ((C_Msg *)message_ptr)->Release(); 
      return false;
   }

   if(OS_R_OK != os_mbx_send(mailbox, message_ptr, timeout)){
     ErrorHandler.ErrorToList((ID_Unit << 24) + ERROR_MAILBUX_REDY);
     ((C_Msg *)message_ptr)->Release(); 
      return false;
   }

   else return true; 
}      
//----------------------------------------------------------------------------

bool print_text(const char *pstrz)
{
  if(NULL == pstrz) return true; 
     
  C_Msg* pMsg = NewMsg(ID_LINKPC, ID_PRINT_TEXT, NOMAILBOX, MSG_RESPONSE, strlen(pstrz));
  memcpy(pMsg->PointerStr(), pstrz, strlen(pstrz));
  U32 uiError = LinkPC.SendMsgToPC(pMsg);
  pMsg->Release();
  return uiError;
}
//----------------------------------------------------------------------------

/////---------------------------------------------------------------------------
///// ���������� ��������� ��
/////---------------------------------------------------------------------------
//void MsgToPC(C_Msg * pMsg){
//  pMsg->AddRef();
//  m_os_mbx_send(LinkPC.GetPtrMailbox(), pMsg, TIMEOUT_MAILBUX_REDY, ID_MSGTOPC);
//}
////----------------------------------------------------------------------------
//
///---------------------------------------------------------------------------
/// ��������� ������, ����������� �� ��
///---------------------------------------------------------------------------
S_Parameters tP0, tP1;
volatile char cch;
void C_Management::PC_CommandHandler(C_Msg* pMsg)
{
  U32 uiX;
  char * pcInputString = pMsg->PointerStr() + 1;   /// ������ ��������� ������������

  pMsg->AddRef();
  // ������� ��� GSM-������
  if('m' == pMsg->PointerStr()[1]){
    GSM.SendMsgToGSM(pMsg);  // ��������� ��������� �������� GSM �����
    goto End;
  }

  // ������� ��� ����������� ������
  if('p' == pMsg->PointerStr()[1]){
    /// ������� ������ � ���� ������� �������� ������. ������ ������ 'p'
    U16 usLength = pMsg->GetLength();
    for(int i = 1; i < usLength; ++i)
      pMsg->PointerStr()[i] = pMsg->PointerStr()[i + 1];
    pMsg->PointerStr()[usLength - 1] = 0;
    // ����������� 0 � ������ ���������� �� �����
    pMsg->SetLength(pMsg->GetLength() - 2); 
    // ��������� ��������� �������� ������
    LCD.SendMsgToLCD(pMsg);  
    goto End;
  }

  // **** ������� ������/������ ���������� ��������� ****
  pcInputString[strlen(pcInputString) - 1] = 0;    /// ������ �������� ������������. ����� ������ ������ � ������ !!!

   // �������� ������ ��.
  if(IsCMD("gVer", pcInputString)){  
    strcpy(strz80, szVersion); 
    goto CMDHandler_Response;
  }
    // �������� ����� ����������
  if(IsCMD("gSN", pcInputString)){   
    strcpy(strz80, szSerialNumber); 
    goto CMDHandler_Response;
  }

  // ��������� ����� � ����
  if(IsCMD("sTime", pcInputString)){ 
    // ������������ ��������� �� 1-� ������ ���������
    pcInputString = FindNextArg(pcInputString, ARG_DELIMITER);  
    if(NO_ERROR == RTC1338.SetTimeFromStr(pcInputString)) goto CMDHandler_OK; 
    goto CMDHandler_Error; 
  }
  // �������� ����� � ����
  if(IsCMD("gTime", pcInputString)){ 
    RTC1338.TimeToStr(strz80, sizeof(strz80)); 
    goto CMDHandler_Response;
  }

  // ���������� ��������� ����.   
  if(IsCMDwithArg("sZone", pcInputString, &uiX)){
    if(Parameters->SetTimeZone(uiX)) goto CMDHandler_OK;
    goto CMDHandler_Error; 
  }
  // �������� ��������� ����.
  if(IsCMD("gZone", pcInputString)){
    sprintf(strz80, "%u", Parameters->GetTimeZone());
    goto CMDHandler_Response;
  }

  // ���������� �������� ������ ������������� ������.   
  if(IsCMDwithArg("sTWP", pcInputString, &uiX)){
    if(Parameters->SetWPTimeout(uiX)) goto CMDHandler_OK;
    goto CMDHandler_Error; 
  }
  // �������� �������� ������ ������������� ������.
  if(IsCMD("gTWP", pcInputString)){
    sprintf(strz80, "%u", Parameters->GetWPTimeout());
    goto CMDHandler_Response;
  }

  // ���������� ����� ���������.   
  if(IsCMD("sNumber", pcInputString)){
    // ������������ ��������� �� 1-� ������ ���������
    pcInputString = FindNextArg(pcInputString, ARG_DELIMITER);  
    if(Parameters->SetTerminalNumber(pcInputString)) goto CMDHandler_OK;
    goto CMDHandler_Error; 
  }
  // �������� ����� ���������.
  if(IsCMD("gNumber", pcInputString)){
    const char *pc = Parameters->GetTerminalNumber();
    strcpy(strz80, pc);
    goto CMDHandler_Response;
  }

  // ���������� ������.   
  if(IsCMD("sPass", pcInputString)){
    
    // ������������ ��������� �� 1-� ������ ���������
    pcInputString = FindNextArg(pcInputString, ARG_DELIMITER); 
    if(Parameters->SetGPSPassowrd(pcInputString)) goto CMDHandler_OK;
    goto CMDHandler_Error; 
  }
  // �������� ������.
  if(IsCMD("gPass", pcInputString)){
    strcpy(strz80, Parameters->GetGPSPassowrd());
    goto CMDHandler_Response;
  }

//  // ���������� ���������� ������ ������������� ������.   
//  if(IsCMDwithArg("sTimeout", pstrzInputBuf, &uiX)){
//    if(Parameters->SetWPDistanse(uiX)) goto CMDHandler_OK;
//    else goto CMDHandler_Error; 
//  }
//  // �������� �������� ������ ������������� ������.
//  if(IsCMD("gTimeout", pstrzInputBuf)){
//    sprintf(strz80, "%u", Parameters->GetWPDistanse());
//    goto CMDHandler_Response;
//  }
//
//  // ���������� ������ �������� ������������� ������.   
//  if(IsCMDwithArg("sTimeout", pstrzInputBuf, &uiX)){
//    if(Parameters->SetWPAzimut(uiX)) goto CMDHandler_OK;
//    else goto CMDHandler_Error; 
//  }
//  // �������� �������� ������ ������������� ������.
//  if(IsCMD("gTimeout", pstrzInputBuf)){
//    sprintf(strz80, "%u", Parameters->GetWPAzimut());
//    goto CMDHandler_Response;
//  }
//

  // ������� �����������
  if('d' == pMsg->PointerStr()[1])
  {
    // ��������� ������ ������
    if(strncmp2lower("deErr", pMsg->PointerStr() + 1, 5))   ErrorHandler.boEnableErrPrint = true; 
    // ��������� ������ ������
    if(strncmp2lower("ddErr", pMsg->PointerStr() + 1, 5))   ErrorHandler.boEnableErrPrint = false;   
    // ���������� ����� ������
    if(strncmp2lower("dmHRST", pMsg->PointerStr() + 1, 6))   GSM.HardReset();
    // ��������� �������� ������������� GSM ������ � ��
    if(strncmp2lower("dEMR2PC", pMsg->PointerStr() + 1, 6))  GSM.DiagnosticsRxEnabled();
    // ��������� �������� ������������� GSM ������ � ��
    if(strncmp2lower("dDMR2PC", pMsg->PointerStr() + 1, 6))  GSM.DiagnosticsRxDisable();
    // ��������� �������� ������ ��� GSM ������ � ��
    if(strncmp2lower("dEMC2PC", pMsg->PointerStr() + 1, 6))  GSM.DiagnosticsTxEnabled();
    // ��������� �������� ������ ��� GSM ������ � ��
    if(strncmp2lower("dDMC2PC", pMsg->PointerStr() + 1, 6))  GSM.DiagnosticsTxDisable();
    // ��������� �������� ������ GPS ��������� � ��
    if(strncmp2lower("dEG2PC", pMsg->PointerStr() + 1, 6))   GPS.DiagnosticsRxEnabled();
    // ��������� �������� ������ GPS ��������� � ��
    if(strncmp2lower("dDG2PC", pMsg->PointerStr() + 1, 6))   GPS.DiagnosticsRxDisable();
    // ��������� �������� ������ ��� ������ � ��
    if(IsCMD("dEPC2PC",  pcInputString)) LCD.DiagnosticsTxEnabled();
    // ��������� �������� ������ ��� ������ � ��
    if(IsCMD("dDPC2PC",  pcInputString)) LCD.DiagnosticsTxDisable();
    // ��������� �������� ������������� �� ������ � ��
    if(IsCMD("dEPR2PC",  pcInputString)) LCD.DiagnosticsRxEnabled();
    // ��������� �������� ������������� ��  ������ � ��
    if(IsCMD("dDPR2PC",  pcInputString)) LCD.DiagnosticsRxDisable();

    // ��������� ��������� ��������� ������� � ��
    if(IsCMD("dgState",  pcInputString)) {
      C_Msg* pMsgGPS = StateHandler->GPSStateToText();
      if(NULL != pMsgGPS){
        LinkPC.SendMsgToPC(pMsgGPS);
        pMsgGPS->Release();
      }
      // ��������� ������
      C_Msg* pMsgInput = StateHandler->InputStateToText();
      if(NULL != pMsgInput){
        LinkPC.SendMsgToPC(pMsgInput);
        pMsgInput->Release();
      }
    }


    // ���������� �������� ���������� TDA8551
    if(IsCMD("dAmpOn", pMsg->PointerStr())){   
      TDA8551_On();
      print_text(strzOK);
    }
    if(IsCMD("dAmpOff", pMsg->PointerStr())){  
      TDA8551_Off();
      print_text(strzOK);
    }
    if(IsCMD("dAmpUp", pMsg->PointerStr())){   
      TDA8551_GainUp();
      print_text(strzOK);
    }
    if(IsCMD("dAmpDown", pMsg->PointerStr())){
      TDA8551_GainDown();
      print_text(strzOK);
    }

    // ���������� ����������� ���������� MAX9814
    if(IsCMD("dMICOn", pMsg->PointerStr())){   
      MAX9814_On();
      print_text(strzOK);
    }
    if(IsCMD("dMICOff", pMsg->PointerStr())){  
      MAX9814_Off();
      print_text(strzOK);
    }
    if(IsCMD("dMICA500", pMsg->PointerStr())){  
      MAX9814_SetAR500();
      print_text(strzOK);
    }
    if(IsCMD("dMICA2000", pMsg->PointerStr())){ 
      MAX9814_SetAR2000();
      print_text(strzOK);
    }
    if(IsCMD("dMICA4000", pMsg->PointerStr())){ 
      MAX9814_SetAR4000();
      print_text(strzOK);
    }
    if(IsCMD("dMICG40", pMsg->PointerStr())){   
      MAX9814_SetGain40();
      print_text(strzOK);
    }
    if(IsCMD("dMICG50", pMsg->PointerStr())){   
      MAX9814_SetGain50();
      print_text(strzOK);
    }
    if(IsCMD("dMICG60", pMsg->PointerStr())){   
      MAX9814_SetGain60();
      print_text(strzOK);
    }

  }
  goto CMDHandler_End;

CMDHandler_OK:
  PrintResponse(SIMBOL_NOERROR, "");
  goto CMDHandler_End;
CMDHandler_Error:
  PrintResponse(SIMBOL_ERROR, "");
  goto CMDHandler_End;
CMDHandler_Response:
  PrintResponse(SIMBOL_NOERROR, strz80);
  goto CMDHandler_End;

End:
CMDHandler_End:
  pMsg->Release();
  return;
}
//----------------------------------------------------------------------------

///---------------------------------------------------------------------------
/// ������� ����������� ���������. ��������� ����� ��������� �� ������ ����� ������� 
///---------------------------------------------------------------------------
__task void Task_Management(void) 
{
  C_Msg * pMsg;
  for(;;){
    pMsg = NULL;
    os_mbx_wait(Management.MailBox, (void **)&(pMsg), 0xFFFF); 
    switch(pMsg->GetIDSource()) // ����������� ������������� ��������� ��������� 
    {     

      // ��������� ������ �� ��
      case ID_LINKPC:    
            Management.PC_CommandHandler(pMsg);
            break;
      default:
            break;
    }
    pMsg->Release();
  }
}
//----------------------------------------------------------------------------


