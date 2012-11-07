//============================================================================
// ��������                                    
//============================================================================

#include <RTL.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include "sysconfig.h"
#include "parameters.h"
#include "gsm.h"
#include "errorhandler.h"
#include "menu.h"
#include "statehandler.h"
#include "terminal.h"
#include "msg.h"
#include "lcd.h"
#include "rtc1338.h"
#include "dc.h"
#include "profiler.h"
//----------------------------------------------------------------------------

void TDA8551_On(void);
void TDA8551_Off(void);
void MAX9814_On(void);
void MAX9814_Off(void);
//----------------------------------------------------------------------------

const U8 TEXT_STATE_NO = 0;
const U8 TEXT_STATE_START = 1;
const U8 TEXT_STATE_END = 2;
const U32 TERMINAL_ERROR_1 = 0x11;
//----------------------------------------------------------------------------

C_Terminal::C_Terminal(void) : boTypeInput(false), 
  ucTextState(0), ucKBDRegime(KBD_REGIME_BASE), pTextMsg(NULL) {
  os_mbx_init(MailBox, sizeof(MailBox));
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
// ��������� ��������� �� ��������� � ������� �� ��
// ������� ��������� ������������, ���� ���������� ��������� �� ����������
//---------------------------------------------------------------------------
bool C_Terminal::SetPointerpTextMsg(C_Msg* pMsg) {
  // ���������� ��������� �� ����������
  if(0 != ucTextState) return false;  
  else {
    ucTextState = TEXT_STATE_START;
    // �������� ��������� �� ���������
    pTextMsg = pMsg;    
    pMsg->AddRef();
    return true;
  }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void C_Terminal::KBD_Handler(U32 uiOnKeys)
//void C_Terminal::KBD_Handler(U32 uiOnKeys)
{
  /// ��������� �������� �������
//  uiOnKeys = ActiveKey(uiOnKeys);
//  if(ACTIVE_NO != uiOnKeys)
//    __nop();
  /// ��������� ��������� ����� ��������� ����� ��� ��� � ������ 
  /// ��������� ����� ��������� ��� ����������
  if(StateHandler->GetVoiseStatus()){
     switch(uiOnKeys) {
       case ACTIVE_ENTER: VoiseOn(); break;
       case ACTIVE_ESC:   VoiseOff();   break;
     }
     return;           
  }

  if(ACTIVE_ALARM == uiOnKeys) {
    S_WayPointPacket tWayPointPacket;
    StateHandler->BuildWayPointPacket(tWayPointPacket, 0xFFFF);
    // ������������ ���� Alarm  � ������ ���� Archiv
    tWayPointPacket.uiStatus = tWayPointPacket.uiStatus | STATE_ALARM; 
    DC.SendPacketToDC((char *)&tWayPointPacket, sizeof(S_WayPointPacket));
    return;
  }

  switch(ucKBDRegime) {
    case KBD_REGIME_BASE:  
         switch(uiOnKeys) {
           case ACTIVE_ENTER: BASE_enter(); break;
           case ACTIVE_ESC:   BASE_esc();   break;
           case ACTIVE_UP:    BASE_up();    break;
           case ACTIVE_DOWN:  BASE_down();  break;
         }           
         break;
    case KBD_REGIME_MENU:
         switch(uiOnKeys) {
           case ACTIVE_ENTER: MENU_enter(); break;
           case ACTIVE_ESC:   MENU_esc();   break;
           case ACTIVE_UP:    MENU_up();    break;
           case ACTIVE_DOWN:  MENU_down();  break;
         }
         break;
    case KBD_REGIME_TEXT:
         if(ACTIVE_NO != uiOnKeys) {
           Terminal.ucKBDRegime = KBD_REGIME_OFF;
         }
         break;
  }
  return;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void C_Terminal::BASE_up(void){
  boTypeInput = true;             /// ���� � ���� �� ������� �������
  ucKBDRegime = KBD_REGIME_MENU;
  OpenMenu(ID_MENU_MSG);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void C_Terminal::BASE_down(void){
  boTypeInput = true;             /// ���� � ���� �� ������� �������
  ucKBDRegime = KBD_REGIME_MENU;
  OpenMenu(ID_MENU_CALL);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void C_Terminal::BASE_enter(void){
  boTypeInput = false;            /// ������� ���� � ����
  ucKBDRegime = KBD_REGIME_MENU;
  OpenMenu(ID_MENU_BASE);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void C_Terminal::BASE_esc(void){
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void C_Terminal::MENU_up(void){
  UP();
  SaveBackTrack(); /// �������� ����
  MENU_CMDCursorMove();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void C_Terminal::MENU_down(void){
  Down();
  SaveBackTrack(); /// �������� ����
  MENU_CMDCursorMove();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void C_Terminal::MENU_enter(void){
  const S_ItemProperties * pItemProperties = GetItemProperties();
  if(0 != pItemProperties->ucNumberItems){   
    SaveBackTrack(); /// �������� ����
    /// ������������� �����. �������� ��������� ����
    OpenMenu(GetID());
  }
  else { 
///    PositionToID(pItemProperties->ucCursorPosition);
///    SetNumberItems(pItemProperties->ucNumberItems);
  /// �������� �����. �������� �������
    pfuncMenuHandler pfunc = pItemProperties->pfunc;
    U32 uiError = (Terminal.*pfunc)(*pItemProperties);
  }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void C_Terminal::MENU_esc(void){
  U32 uiID = GetID();
  /// ����������� � ����� BASE ���� ������ � ���� �� ������� �������� ��� ���� 
  /// �������� � ���� 0-�� ������
  if(boTypeInput || (0 == ((ITEM_ID(0, 0x0F, 0x0F, 0x0F) & uiID)))){
    SetID(ITEM_ID(0, 0, 0, 0));
    ucKBDRegime = KBD_REGIME_BASE;
  }
  else {   
    Esc();                  
    OpenMenu(GetID());
  }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

///---------------------------------------------------------------------------
/// ���������� ���� � �������������� uiID, �������� ������� ��� ����������� 
/// ������ ������� ����, ��������� � ���������� ��������� ������������ ������
/// ���� ID == 0, �������������� ������� ������������� ����� ������� ������� 
/// ������� �� �������, ���������� ��� ID = 0
///---------------------------------------------------------------------------
void C_Terminal::OpenMenu(U32 uiID){
  /// ������������ ������ �� ������� ���������������  uiID
  SetID(uiID);
  /// ������� �������� ������ ���� �� ������� ���������� ������ � ������� ����������
  U8 ucCursorPosition = GetItemProperties()->ucCursorPosition & 0x07;
  if(POSITION_MAX < ucCursorPosition) return;
  if(0 == ucCursorPosition) return;
  U8 ucNumberItems = GetItemProperties()->ucNumberItems & 0x07;
  if(POSITION_MAX < ucNumberItems) return;
  if(0 == ucNumberItems) return;
  U16 usLength = strlen(GetItemProperties()->pNameList);
  /// ������������ ����� ������ 236 ��������
  if(236 < usLength) return;
  C_Msg * pMsg  = NewMsg(ID_LCD, ID_TERMINAL, NULL,  MSG_CMDWITHRESPONSE, usLength + 9);
  if(NULL == pMsg) return;	/// ���������� ��� ���������
  /// �������� �������
  pMsg->PointerStr()[0] = START_DELIMITER;
  pMsg->PointerStr()[1] = 'M';
  pMsg->PointerStr()[2] = ucCursorPosition + 0x30;
  pMsg->PointerStr()[3] = '0';  // �����
  pMsg->PointerStr()[4] = '0';
  pMsg->PointerStr()[5] = '0';
  pMsg->PointerStr()[6] = '0';
  memcpy(&pMsg->PointerStr()[7], GetItemProperties()->pNameList, usLength);
  pMsg->PointerStr()[usLength + 7] = END_DELIMITER;
  pMsg->PointerStr()[usLength + 8] = 0;

  U32 uiError = LCD.SendMsgToLCD(pMsg);   /// ��������� ���������
  pMsg->Release();
  /// ������� ������� ��������� ������� (������ �� ��������. ������� �������� ��������� �� ������)
  PositionToID(ucCursorPosition);
  SetNumberItems(ucNumberItems);
  return;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//----------------------------------------------------------------------------
// ��������� � ���������� ������������ ������ ������� "����������� ������"
//----------------------------------------------------------------------------
void C_Terminal::MENU_CMDCursorMove(void){
  U8 ucPosition = IDToPosition();
  if(0 == ucPosition) return;  /// ���� �� ��������������. 
  if(7 <  ucPosition) return;  /// ��� ������� � ����� > 7 
  /// �������� � ��������� �������
  /// ��������� ������������, 'C', �������, �������� ������������
  C_Msg * pMsg  = NewMsg(ID_LCD, ID_TERMINAL, NULL,  MSG_CMDWITHRESPONSE, 5);
  if(NULL == pMsg) return;	// ���������� ��� ���������
  /// �������� �������
  pMsg->PointerStr()[0] = START_DELIMITER;
  pMsg->PointerStr()[1] = 'C';
  pMsg->PointerStr()[2] = ucPosition + 0x30;  /// ���������� � ������
  pMsg->PointerStr()[3] = END_DELIMITER;
  pMsg->PointerStr()[4] = 0;

  U32 uiError = LCD.SendMsgToLCD(pMsg);   /// ��������� ���������
  pMsg->Release();
  return;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//----------------------------------------------------------------------------
// ��������� � ���������� ������������ ������ ������� "�������� ��������" 
//----------------------------------------------------------------------------
void C_Terminal::ShowScreen(U8 ucNamberScreen)
{
  /// �������� � ��������� �������
  C_Msg * pMsg  = NewMsg(ID_LCD, ID_TERMINAL, NULL,  MSG_CMDWITHRESPONSE, 4);
  if(NULL == pMsg) return;	// ���������� ��� ���������
  /// �������� �������
  pMsg->PointerStr()[0] = START_DELIMITER;
  pMsg->PointerStr()[1] = 'W';
  pMsg->PointerStr()[2] = END_DELIMITER;
  pMsg->PointerStr()[3] = 0;

  U32 uiError = LCD.SendMsgToLCD(pMsg);   /// ��������� ���������
  pMsg->Release();
  return;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//----------------------------------------------------------------------------
// ��������� � ���������� ������������ ������ ������� "�������� ����" 
//----------------------------------------------------------------------------
void C_Terminal::ShowData(char *strz)
{
  /// �������� � ��������� �������
  C_Msg * pMsg  = NewMsg(ID_LCD, ID_TERMINAL, NULL,  MSG_CMDWITHRESPONSE, 12);
  if(NULL == pMsg) return;	/// ���������� ��� ���������
  /// �������� �������
  pMsg->PointerStr()[0] = START_DELIMITER;
  pMsg->PointerStr()[1] = 'X';
  memcpy(&pMsg->PointerStr()[2], strz, 8); 
  pMsg->PointerStr()[10] = END_DELIMITER;
  pMsg->PointerStr()[11] = 0;

  U32 uiError = LCD.SendMsgToLCD(pMsg);   /// ��������� ���������
  pMsg->Release();
  return;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//----------------------------------------------------------------------------
// ��������� � ���������� ������������ ������ ������� "�������� �����" 
//----------------------------------------------------------------------------
void C_Terminal::ShowTime(char *strz)
{
  /// �������� � ��������� �������
  C_Msg * pMsg  = NewMsg(ID_LCD, ID_TERMINAL, NULL,  MSG_CMDWITHRESPONSE, 9);
  if(NULL == pMsg) return;	/// ���������� ��� ���������
  /// �������� �������
  pMsg->PointerStr()[0] = START_DELIMITER;
  pMsg->PointerStr()[1] = 'Y';
  memcpy(&pMsg->PointerStr()[2], strz, 5); 
  pMsg->PointerStr()[7] = END_DELIMITER;
  pMsg->PointerStr()[8] = 0;

  U32 uiError = LCD.SendMsgToLCD(pMsg);   /// ��������� ���������
  pMsg->Release();
  return;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

///---------------------------------------------------------------------------
/// ���������� ���� � ����� ���� ���������� ����� ����� 
///---------------------------------------------------------------------------
void C_Terminal::ShowDataTime(bool boRestore)
{
  static U32 minuts0 = 0;          /// ����� � �������
  if(boRestore) minuts0 = 0; 

  time_t t = RTC1338.GetTime();
  if(0xFFFFFFFF == t){
//    return ERROR_DS1338_GETTIME;
// !!! ����� ��������� ������  
    return;
  }

  U32 minuts1 = t/60;             /// ����� � ���
  if(minuts0 == minuts1) return;  /// ����� ���������� ������ ��� �� 1 ������
  tm * pstTime = localtime(&t);
  char strz[10];
  strftime(strz, sizeof(strz),"%d.%m.%y", pstTime);
  ShowData(strz);
  strftime(strz, sizeof(strz),"%H:%M", pstTime);
  ShowTime(strz);  
  minuts0 = minuts1;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void C_Terminal::BusStop(const char *strz, U8 chCMD)
{
  /// �������� � ��������� �������
  C_Msg * pMsg  = NewMsg(ID_LCD, ID_TERMINAL, NULL,  MSG_CMDWITHRESPONSE, LENGH_NAME_BUS_STOP + 4);
  if(NULL == pMsg) return;	/// ���������� ��� ���������
  /// �������� �������
  pMsg->PointerStr()[0] = START_DELIMITER;
  pMsg->PointerStr()[1] = chCMD;
  pMsg->PointerStr()[2] = 0;
  strncat(pMsg->PointerStr(), strz, LENGH_NAME_BUS_STOP); 
  char strzTemp[] = {END_DELIMITER, 0};
  strcat(pMsg->PointerStr(), strzTemp); 

  U32 uiError = LCD.SendMsgToLCD(pMsg);   /// ��������� ���������
  pMsg->Release();
  return;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void C_Terminal::BusStop(const char *strz){
  BusStop(strz, 'A');
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void C_Terminal::NextBusStop(const char *strz){
  BusStop(strz, 'B');
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void C_Terminal::NumberBus(const char *strz)
{
  /// �������� � ��������� �������
  C_Msg * pMsg  = NewMsg(ID_LCD, ID_TERMINAL, NULL,  MSG_CMDWITHRESPONSE, LENGH_NUMBER_BUS + 4);
  if(NULL == pMsg) return;	/// ���������� ��� ���������
  /// �������� �������
  pMsg->PointerStr()[0] = START_DELIMITER;
  pMsg->PointerStr()[1] = 'N';
  pMsg->PointerStr()[2] = 0;
  strncat(pMsg->PointerStr(), strz, LENGH_NUMBER_BUS); 
  char strzTemp[] = {END_DELIMITER, 0};
  strcat(pMsg->PointerStr(), strzTemp); 

  U32 uiError = LCD.SendMsgToLCD(pMsg);   /// ��������� ���������
  pMsg->Release();
  return;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

///---------------------------------------------------------------------------
///  ���������� �� �������
///---------------------------------------------------------------------------
void C_Terminal::Shift(const char *strz)
{
  /// �������� � ��������� �������
  C_Msg * pMsg  = NewMsg(ID_LCD, ID_TERMINAL, NULL,  MSG_CMDWITHRESPONSE, LENGH_NAME_BUS_STOP + 4);
  if(NULL == pMsg) return;	// ���������� ��� ���������
  /// �������� �������
  pMsg->PointerStr()[0] = START_DELIMITER;
  pMsg->PointerStr()[1] = 'Z';
  pMsg->PointerStr()[2] = 0;
  strncat(pMsg->PointerStr(), strz, LENGH_NAME_BUS_STOP); 
  char strzTemp[] = {END_DELIMITER, 0};
  strcat(pMsg->PointerStr(), strzTemp); 

  U32 uiError = LCD.SendMsgToLCD(pMsg);   /// ��������� ���������
  pMsg->Release();
  return;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

///---------------------------------------------------------------------------
///  �������� ������������ �������
///---------------------------------------------------------------------------
/// ���� � ������� ������������ ���������
const char pcGPS[] =   {'1', '1', '4'};   
const char pcTCP[] =   {'1', '1', '5'};
const char pcVOICE[] = {'1', '1', '6'};
void C_Terminal::Icon(const char * pc)
{
  
  C_Msg * pMsg  = NewMsg(ID_LCD, ID_TERMINAL, NULL,  MSG_CMDWITHRESPONSE, 7);
  if(NULL == pMsg) return;	// ���������� ��� ���������
  pMsg->PointerStr()[0] = START_DELIMITER;
  pMsg->PointerStr()[1] = 'D';
  pMsg->PointerStr()[2] = pc[0];
  pMsg->PointerStr()[3] = pc[1];
  pMsg->PointerStr()[4] = pc[2];
  pMsg->PointerStr()[5] = END_DELIMITER;
  pMsg->PointerStr()[6] = 0;

  U32 uiError = LCD.SendMsgToLCD(pMsg);   /// ��������� ���������
  pMsg->Release();
  return;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void C_Terminal::IconErase(const char * pc)
{
  C_Msg * pMsg  = NewMsg(ID_LCD, ID_TERMINAL, NULL,  MSG_CMDWITHRESPONSE, 7);
  if(NULL == pMsg) return;	/// ���������� ��� ���������

  pMsg->PointerStr()[0] = START_DELIMITER;
  pMsg->PointerStr()[1] = 'D';
  pMsg->PointerStr()[2] = pc[0];   /// ������� ��������
  pMsg->PointerStr()[3] = '1';     /// ��� �������
  pMsg->PointerStr()[4] = '1';
  pMsg->PointerStr()[5] = END_DELIMITER;
  pMsg->PointerStr()[6] = 0;

  U32 uiError = LCD.SendMsgToLCD(pMsg);   /// ��������� ���������
  pMsg->Release();
  return;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
// �������� ��������� � TCP ������� � �������.
// �������� �� ���� ����� � �������� ��� �������� LCD ��������
// ������������������ � ���� pTextMsg. ���� ���������������� �������  
// SetPointerpTextMsg() � � ���� ������ ����� AddRef(). ����� Release() ������ 
// ������ ���� ������ � ������ ShowTextFromDC().
//---------------------------------------------------------------------------
U32 C_Terminal::ShowTextFromDC(C_Msg* pTextMsg)
{
  if(NULL == pTextMsg) return NO_ERROR;
  U32 uiError;

  // ������� ��������� �� ����� � �������
  S_MessagePacket* Packet = reinterpret_cast<S_MessagePacket*>(pTextMsg->PointerStr());
  // ������� ��������� �� ����� � ��������� ��� �����
  char* szString = Packet->pcMessage;
  U16 usLength = strlen(szString);

  // �������� ������� � ��������� ��� �������� ������ � LCD
  S_MessagePacket *pMessagePacket = (S_MessagePacket *)(pTextMsg->PointerStr());
  int iLength = strlen(pMessagePacket->pcMessage);
  C_Msg* pMsg  = NewMsg(ID_LCD, ID_TERMINAL, NULL,  MSG_CMDWITHRESPONSE, iLength + 4);
  if(NULL != pMsg){ 
    pMsg->PointerStr()[0] = START_DELIMITER;
    pMsg->PointerStr()[1] = 'T';
    memcpy(pMsg->PointerStr() + 2, pMessagePacket->pcMessage, iLength);
    pMsg->PointerStr()[2 + iLength] = END_DELIMITER;
    pMsg->PointerStr()[2 + iLength + 1] = 0;

    // ��������� ���������
    uiError = LCD.SendMsgToLCD(pMsg);   
    pMsg->Release();
 }

  else uiError = TERMINAL_ERROR_1;
  return uiError;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void C_Terminal::Restore(void)
{
  Terminal.ShowScreen(SCREEN_BASE);
  Terminal.ShowDataTime(true);
  Terminal.Shift(StateHandler->GetShift());
  Terminal.NumberBus(StateHandler->GetNumberBus());
  if(StateHandler->GetGPSStatus())   Icon(pcGPS);
  if(StateHandler->GetTCPStatus())   Icon(pcTCP);
  if(StateHandler->GetVoiseStatus()) Icon(pcVOICE);
  Terminal.BusStop(StateHandler->GetBusStop());
  Terminal.NextBusStop(StateHandler->GetNextBusStop());
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void C_Terminal::VoiseOn(){
  /// ������� ������� ������� ������
//  char strzResponse[8];
//  U32 Error = GSM.CMDHandler("meCallUp", Parameters->GetTelephoneNumber(0), strzResponse, sizeof(strzResponse) - 1, ID_MENU);
  /// ������� ����������� ���������
//  if(NO_ERROR == Error)  MAX9814_On();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void C_Terminal::VoiseOff(){
  /// ������� ������� �������� ������
  /// �������� ����������� ���������
  MAX9814_Off();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

///---------------------------------------------------------------------------
/// ������������ �������� ������� ������ �� �������� �����
/// ����� ������ ��������� ����� �� ������� ����� �������
/// ����� ������ �� ������ ������������ �������� ����� 
///---------------------------------------------------------------------------
//#define D_PROFILER
#ifdef D_PROFILER
  const U8 TERMINAL_B = 3;
  const U8 TERMINAL_E = 4;
  extern C_Profiler Profiler;
#endif
__task void Task_Terminal(void) 
{
  static bool boRestore = true;         /// = true ����� ���������� ������������ �������� �����

  for(;;){
    os_dly_wait(10);    // 100 ��.

    // ��������� ������� ������
    U8 ucActiveKey = StateHandler->GetActiveKey();
    if(ACTIVE_NO != ucActiveKey){
      Terminal.KBD_Handler(ucActiveKey);
      StateHandler->ClearActiveKey();
    }
    // ��������� ����������� ��������� ���������
    // 0 - ����� � ��������� ������ TextMsg
    // 1 - ������� ��������� �� TextMsg
    // 2 - �������� ����� LCD ��������, ��������� ��������� ���������� �������� KBD � ����� Text
    /// ����� ������
    if(TEXT_STATE_START == Terminal.ucTextState) {
      Terminal.ucTextState = TEXT_STATE_END;
      Terminal.ucKBDRegimeTemp = Terminal.ucKBDRegime;
      Terminal.ucKBDRegime = KBD_REGIME_TEXT;
      Terminal.ShowTextFromDC(Terminal.pTextMsg);
    }

    if(KBD_REGIME_OFF == Terminal.ucKBDRegime){ 
      Terminal.ucKBDRegime = Terminal.ucKBDRegimeTemp;
      if(KBD_REGIME_MENU == Terminal.ucKBDRegime) {
        // �������������� ��������� ��������� � ��� ������ MENU
        Terminal.MENU_esc();
        Terminal.MENU_enter();
      }
      // �������������� ��������� ��������� � ��� ������ BASE
      else boRestore = true;
      Terminal.ucTextState = TEXT_STATE_NO;  // ���������� ������  TEXT
    }

    // �������������� ������
    if(KBD_REGIME_BASE == Terminal.ucKBDRegime){ 
      if(StateHandler->GetUpdateStatus()){
        // ���� ���������� ���������� ��������� ������, �� �������� ��������� �������������� ������
        boRestore = true;
        StateHandler->ResetUpdateStatus();
      }
      if(boRestore){
        Terminal.Restore();
        boRestore = false;
      }
      Terminal.ShowDataTime(false);
    }
    else boRestore = true;
  }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

