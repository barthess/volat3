#ifndef TERMINAL_H
#define TERMINAL_H
//============================================================================
// ��������                                    
//============================================================================

#include "SysConfig.h"
#include "usart.h"
#include "menu.h"
//----------------------------------------------------------------------------

const char KBD_REGIME_OFF = 0;  // ��������� �� ��, ��� ����� ���������� � ���������� ucKBDRegimeTemp
const char KBD_REGIME_BASE = 1;
const char KBD_REGIME_MENU = 2;
const char KBD_REGIME_TEXT = 3;
const char SCREEN_BASE = 0;     // �������������� ��������� ��������� ��������� ������
//----------------------------------------------------------------------------

class C_Terminal : C_Menu 
{
  friend __task void Task_Terminal(void); 
private:
  C_UART * pUART;          // ����������� �������� � �������� ��������� ��������
  bool boTypeInput;        // ��� ����� � ����. true - �� ������� �������
  U8   ucTextState;        // ��������� ��������� ���������� ���������
  U8   ucKBDRegime;        // ������� ����� ����������
  U8   ucKBDRegimeTemp;    // ��������� ������� ����� ���������� ��� ������ ������
  C_Msg* pTextMsg;         // ��������� �� ��������� � ������� �� ��

  void BASE_up(void);
  void BASE_down(void);
  void BASE_enter(void);
  void BASE_esc(void);
  void MENU_up(void);
  void MENU_down(void);
  void MENU_CMDCursorMove(void);
  void MENU_enter(void);
  void MENU_esc(void);
  void TEXT_up(void);                                  
  void TEXT_down(void);
  void TEXT_enter(void);
  void TEXT_esc(void);
  void OpenMenu(U32 uiID);
  void ShowData(char *strz);
  void ShowTime(char *strz);
  void ShowDataTime(bool);
  void ShowScreen(U8 ucNamberScreen);
  void KBD_Handler(U32 uiKey);
  void BusStop(const char *strz, U8 chCMD);
  void BusStop(const char *strz);
  void NextBusStop(const char *strz);
  void Shift(const char *strz);
  void NumberBus(const char *strz);
  void Icon(const char * pc);
  void IconErase(const char * pc);
  void VoiseOn();
  void VoiseOff();
  U32  ShowTextFromDC(C_Msg* pTextMsg);
public:
  os_mbx_declare(MailBox, TERMINAL_MAILBOX_SIZE); 
  
  C_Terminal();
  void Restore(void);
  bool SetPointerpTextMsg(C_Msg* pMsg);
};
//----------------------------------------------------------------------------

extern C_Terminal Terminal;
extern OS_TID idtTerminal;
__task void Task_Terminal(void); 
//----------------------------------------------------------------------------

#endif //#ifndef TERMINAL_H
