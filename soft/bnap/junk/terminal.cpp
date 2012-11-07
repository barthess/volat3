//============================================================================
// Терминал                                    
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
// Сохраняет указатель на сообщение с текстом от ДЦ
// Текущее сообщение игнорируется, если предыдущее сообщение не обработано
//---------------------------------------------------------------------------
bool C_Terminal::SetPointerpTextMsg(C_Msg* pMsg) {
  // Предыдущее сообщение не обработано
  if(0 != ucTextState) return false;  
  else {
    ucTextState = TEXT_STATE_START;
    // Сохраняю указатель на сообщение
    pTextMsg = pMsg;    
    pMsg->AddRef();
    return true;
  }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void C_Terminal::KBD_Handler(U32 uiOnKeys)
//void C_Terminal::KBD_Handler(U32 uiOnKeys)
{
  /// Определяю активную клавишу
//  uiOnKeys = ActiveKey(uiOnKeys);
//  if(ACTIVE_NO != uiOnKeys)
//    __nop();
  /// Наивысший приоритет имеет голосовая связь так как в режиме 
  /// голосовой связи разорвано ТСР соединение
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
    // Устанавливаю флаг Alarm  и очищаю флаг Archiv
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
  boTypeInput = true;             /// Вход в меню по горячей клавише
  ucKBDRegime = KBD_REGIME_MENU;
  OpenMenu(ID_MENU_MSG);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void C_Terminal::BASE_down(void){
  boTypeInput = true;             /// Вход в меню по горячей клавише
  ucKBDRegime = KBD_REGIME_MENU;
  OpenMenu(ID_MENU_CALL);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void C_Terminal::BASE_enter(void){
  boTypeInput = false;            /// Обычный вход в меню
  ucKBDRegime = KBD_REGIME_MENU;
  OpenMenu(ID_MENU_BASE);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void C_Terminal::BASE_esc(void){
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void C_Terminal::MENU_up(void){
  UP();
  SaveBackTrack(); /// Сохраняю путь
  MENU_CMDCursorMove();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void C_Terminal::MENU_down(void){
  Down();
  SaveBackTrack(); /// Сохраняю путь
  MENU_CMDCursorMove();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void C_Terminal::MENU_enter(void){
  const S_ItemProperties * pItemProperties = GetItemProperties();
  if(0 != pItemProperties->ucNumberItems){   
    SaveBackTrack(); /// Сохраняю путь
    /// Промежуточный пункт. Открываю следующее меню
    OpenMenu(GetID());
  }
  else { 
///    PositionToID(pItemProperties->ucCursorPosition);
///    SetNumberItems(pItemProperties->ucNumberItems);
  /// Конечный пункт. Выполняю команду
    pfuncMenuHandler pfunc = pItemProperties->pfunc;
    U32 uiError = (Terminal.*pfunc)(*pItemProperties);
  }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void C_Terminal::MENU_esc(void){
  U32 uiID = GetID();
  /// Возвращаюсь в режим BASE если входил в меню по горячим клавишам или если 
  /// нахожусь в меню 0-го уровня
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
/// Раскрывает меню с идентификаторо uiID, вызывает функцию для форирования 
/// флагов пунктом меню, формирует и отправляет сообщение графическому модулю
/// Если ID == 0, идентификатору маркера присваивается новое значени которое 
/// берется из свойств, полученных для ID = 0
///---------------------------------------------------------------------------
void C_Terminal::OpenMenu(U32 uiID){
  /// Устанавливаю маркер на позицию соответствующую  uiID
  SetID(uiID);
  /// Копирую свойства пункта меню на который установлен маркер в текущие переменные
  U8 ucCursorPosition = GetItemProperties()->ucCursorPosition & 0x07;
  if(POSITION_MAX < ucCursorPosition) return;
  if(0 == ucCursorPosition) return;
  U8 ucNumberItems = GetItemProperties()->ucNumberItems & 0x07;
  if(POSITION_MAX < ucNumberItems) return;
  if(0 == ucNumberItems) return;
  U16 usLength = strlen(GetItemProperties()->pNameList);
  /// Максимальная длина строки 236 символов
  if(236 < usLength) return;
  C_Msg * pMsg  = NewMsg(ID_LCD, ID_TERMINAL, NULL,  MSG_CMDWITHRESPONSE, usLength + 9);
  if(NULL == pMsg) return;	/// Переполнен пул сообщений
  /// Формирую команду
  pMsg->PointerStr()[0] = START_DELIMITER;
  pMsg->PointerStr()[1] = 'M';
  pMsg->PointerStr()[2] = ucCursorPosition + 0x30;
  pMsg->PointerStr()[3] = '0';  // Флаги
  pMsg->PointerStr()[4] = '0';
  pMsg->PointerStr()[5] = '0';
  pMsg->PointerStr()[6] = '0';
  memcpy(&pMsg->PointerStr()[7], GetItemProperties()->pNameList, usLength);
  pMsg->PointerStr()[usLength + 7] = END_DELIMITER;
  pMsg->PointerStr()[usLength + 8] = 0;

  U32 uiError = LCD.SendMsgToLCD(pMsg);   /// Отправляю сообщение
  pMsg->Release();
  /// Изменяю текущие параметры курсора (Раньше не изменять. Получим неверный указатель на строку)
  PositionToID(ucCursorPosition);
  SetNumberItems(ucNumberItems);
  return;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//----------------------------------------------------------------------------
// Формирует и отправляет графическому модулю команду "переместить курсор"
//----------------------------------------------------------------------------
void C_Terminal::MENU_CMDCursorMove(void){
  U8 ucPosition = IDToPosition();
  if(0 == ucPosition) return;  /// Меню не активизировано. 
  if(7 <  ucPosition) return;  /// Нет позиций с кодом > 7 
  /// Формирую и отправляю команду
  /// Начальный ограничитель, 'C', позиция, конечный ограничитель
  C_Msg * pMsg  = NewMsg(ID_LCD, ID_TERMINAL, NULL,  MSG_CMDWITHRESPONSE, 5);
  if(NULL == pMsg) return;	// Переполнен пул сообщений
  /// Формирую команду
  pMsg->PointerStr()[0] = START_DELIMITER;
  pMsg->PointerStr()[1] = 'C';
  pMsg->PointerStr()[2] = ucPosition + 0x30;  /// Преобразую в символ
  pMsg->PointerStr()[3] = END_DELIMITER;
  pMsg->PointerStr()[4] = 0;

  U32 uiError = LCD.SendMsgToLCD(pMsg);   /// Отправляю сообщение
  pMsg->Release();
  return;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//----------------------------------------------------------------------------
// Формирует и отправляет графическому модулю команду "показать заставку" 
//----------------------------------------------------------------------------
void C_Terminal::ShowScreen(U8 ucNamberScreen)
{
  /// Формирую и отправляю команду
  C_Msg * pMsg  = NewMsg(ID_LCD, ID_TERMINAL, NULL,  MSG_CMDWITHRESPONSE, 4);
  if(NULL == pMsg) return;	// Переполнен пул сообщений
  /// Формирую команду
  pMsg->PointerStr()[0] = START_DELIMITER;
  pMsg->PointerStr()[1] = 'W';
  pMsg->PointerStr()[2] = END_DELIMITER;
  pMsg->PointerStr()[3] = 0;

  U32 uiError = LCD.SendMsgToLCD(pMsg);   /// Отправляю сообщение
  pMsg->Release();
  return;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//----------------------------------------------------------------------------
// Формирует и отправляет графическому модулю команду "показать дату" 
//----------------------------------------------------------------------------
void C_Terminal::ShowData(char *strz)
{
  /// Формирую и отправляю команду
  C_Msg * pMsg  = NewMsg(ID_LCD, ID_TERMINAL, NULL,  MSG_CMDWITHRESPONSE, 12);
  if(NULL == pMsg) return;	/// Переполнен пул сообщений
  /// Формирую команду
  pMsg->PointerStr()[0] = START_DELIMITER;
  pMsg->PointerStr()[1] = 'X';
  memcpy(&pMsg->PointerStr()[2], strz, 8); 
  pMsg->PointerStr()[10] = END_DELIMITER;
  pMsg->PointerStr()[11] = 0;

  U32 uiError = LCD.SendMsgToLCD(pMsg);   /// Отправляю сообщение
  pMsg->Release();
  return;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//----------------------------------------------------------------------------
// Формирует и отправляет графическому модулю команду "показать время" 
//----------------------------------------------------------------------------
void C_Terminal::ShowTime(char *strz)
{
  /// Формирую и отправляю команду
  C_Msg * pMsg  = NewMsg(ID_LCD, ID_TERMINAL, NULL,  MSG_CMDWITHRESPONSE, 9);
  if(NULL == pMsg) return;	/// Переполнен пул сообщений
  /// Формирую команду
  pMsg->PointerStr()[0] = START_DELIMITER;
  pMsg->PointerStr()[1] = 'Y';
  memcpy(&pMsg->PointerStr()[2], strz, 5); 
  pMsg->PointerStr()[7] = END_DELIMITER;
  pMsg->PointerStr()[8] = 0;

  U32 uiError = LCD.SendMsgToLCD(pMsg);   /// Отправляю сообщение
  pMsg->Release();
  return;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

///---------------------------------------------------------------------------
/// Отображает дату и время если изменилось число минут 
///---------------------------------------------------------------------------
void C_Terminal::ShowDataTime(bool boRestore)
{
  static U32 minuts0 = 0;          /// Время в минутах
  if(boRestore) minuts0 = 0; 

  time_t t = RTC1338.GetTime();
  if(0xFFFFFFFF == t){
//    return ERROR_DS1338_GETTIME;
// !!! Нужна обработка ошибок  
    return;
  }

  U32 minuts1 = t/60;             /// Время в сек
  if(minuts0 == minuts1) return;  /// Время изменилось меньше чем на 1 минуту
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
  /// Формирую и отправляю команду
  C_Msg * pMsg  = NewMsg(ID_LCD, ID_TERMINAL, NULL,  MSG_CMDWITHRESPONSE, LENGH_NAME_BUS_STOP + 4);
  if(NULL == pMsg) return;	/// Переполнен пул сообщений
  /// Формирую команду
  pMsg->PointerStr()[0] = START_DELIMITER;
  pMsg->PointerStr()[1] = chCMD;
  pMsg->PointerStr()[2] = 0;
  strncat(pMsg->PointerStr(), strz, LENGH_NAME_BUS_STOP); 
  char strzTemp[] = {END_DELIMITER, 0};
  strcat(pMsg->PointerStr(), strzTemp); 

  U32 uiError = LCD.SendMsgToLCD(pMsg);   /// Отправляю сообщение
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
  /// Формирую и отправляю команду
  C_Msg * pMsg  = NewMsg(ID_LCD, ID_TERMINAL, NULL,  MSG_CMDWITHRESPONSE, LENGH_NUMBER_BUS + 4);
  if(NULL == pMsg) return;	/// Переполнен пул сообщений
  /// Формирую команду
  pMsg->PointerStr()[0] = START_DELIMITER;
  pMsg->PointerStr()[1] = 'N';
  pMsg->PointerStr()[2] = 0;
  strncat(pMsg->PointerStr(), strz, LENGH_NUMBER_BUS); 
  char strzTemp[] = {END_DELIMITER, 0};
  strcat(pMsg->PointerStr(), strzTemp); 

  U32 uiError = LCD.SendMsgToLCD(pMsg);   /// Отправляю сообщение
  pMsg->Release();
  return;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

///---------------------------------------------------------------------------
///  Отклонение от графика
///---------------------------------------------------------------------------
void C_Terminal::Shift(const char *strz)
{
  /// Формирую и отправляю команду
  C_Msg * pMsg  = NewMsg(ID_LCD, ID_TERMINAL, NULL,  MSG_CMDWITHRESPONSE, LENGH_NAME_BUS_STOP + 4);
  if(NULL == pMsg) return;	// Переполнен пул сообщений
  /// Формирую команду
  pMsg->PointerStr()[0] = START_DELIMITER;
  pMsg->PointerStr()[1] = 'Z';
  pMsg->PointerStr()[2] = 0;
  strncat(pMsg->PointerStr(), strz, LENGH_NAME_BUS_STOP); 
  char strzTemp[] = {END_DELIMITER, 0};
  strcat(pMsg->PointerStr(), strzTemp); 

  U32 uiError = LCD.SendMsgToLCD(pMsg);   /// Отправляю сообщение
  pMsg->Release();
  return;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

///---------------------------------------------------------------------------
///  Включить динамический элемент
///---------------------------------------------------------------------------
/// Коды и позиции динамических элементов
const char pcGPS[] =   {'1', '1', '4'};   
const char pcTCP[] =   {'1', '1', '5'};
const char pcVOICE[] = {'1', '1', '6'};
void C_Terminal::Icon(const char * pc)
{
  
  C_Msg * pMsg  = NewMsg(ID_LCD, ID_TERMINAL, NULL,  MSG_CMDWITHRESPONSE, 7);
  if(NULL == pMsg) return;	// Переполнен пул сообщений
  pMsg->PointerStr()[0] = START_DELIMITER;
  pMsg->PointerStr()[1] = 'D';
  pMsg->PointerStr()[2] = pc[0];
  pMsg->PointerStr()[3] = pc[1];
  pMsg->PointerStr()[4] = pc[2];
  pMsg->PointerStr()[5] = END_DELIMITER;
  pMsg->PointerStr()[6] = 0;

  U32 uiError = LCD.SendMsgToLCD(pMsg);   /// Отправляю сообщение
  pMsg->Release();
  return;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void C_Terminal::IconErase(const char * pc)
{
  C_Msg * pMsg  = NewMsg(ID_LCD, ID_TERMINAL, NULL,  MSG_CMDWITHRESPONSE, 7);
  if(NULL == pMsg) return;	/// Переполнен пул сообщений

  pMsg->PointerStr()[0] = START_DELIMITER;
  pMsg->PointerStr()[1] = 'D';
  pMsg->PointerStr()[2] = pc[0];   /// Позиция элемента
  pMsg->PointerStr()[3] = '1';     /// Код пробела
  pMsg->PointerStr()[4] = '1';
  pMsg->PointerStr()[5] = END_DELIMITER;
  pMsg->PointerStr()[6] = 0;

  U32 uiError = LCD.SendMsgToLCD(pMsg);   /// Отправляю сообщение
  pMsg->Release();
  return;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
// Получает сообщение с TCP пакетом с текстом.
// Выделяет из него текст и передает его драйверу LCD монитора
// Указательсообщения в поле pTextMsg. Поле инициализируется методом  
// SetPointerpTextMsg() и в этом методе метод AddRef(). Метод Release() должен 
// должен быть вызван в методе ShowTextFromDC().
//---------------------------------------------------------------------------
U32 C_Terminal::ShowTextFromDC(C_Msg* pTextMsg)
{
  if(NULL == pTextMsg) return NO_ERROR;
  U32 uiError;

  // Получаю указатель на пакет с текстом
  S_MessagePacket* Packet = reinterpret_cast<S_MessagePacket*>(pTextMsg->PointerStr());
  // Получаю указатель на текст и определяю его длину
  char* szString = Packet->pcMessage;
  U16 usLength = strlen(szString);

  // Формирую команду и сообщение для отправки текста в LCD
  S_MessagePacket *pMessagePacket = (S_MessagePacket *)(pTextMsg->PointerStr());
  int iLength = strlen(pMessagePacket->pcMessage);
  C_Msg* pMsg  = NewMsg(ID_LCD, ID_TERMINAL, NULL,  MSG_CMDWITHRESPONSE, iLength + 4);
  if(NULL != pMsg){ 
    pMsg->PointerStr()[0] = START_DELIMITER;
    pMsg->PointerStr()[1] = 'T';
    memcpy(pMsg->PointerStr() + 2, pMessagePacket->pcMessage, iLength);
    pMsg->PointerStr()[2 + iLength] = END_DELIMITER;
    pMsg->PointerStr()[2 + iLength + 1] = 0;

    // Отправляю сообщение
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
  /// Посалаю команду поднять трубку
//  char strzResponse[8];
//  U32 Error = GSM.CMDHandler("meCallUp", Parameters->GetTelephoneNumber(0), strzResponse, sizeof(strzResponse) - 1, ID_MENU);
  /// Включаю микрофонный исилитель
//  if(NO_ERROR == Error)  MAX9814_On();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void C_Terminal::VoiseOff(){
  /// Посылаю команду положить трубку
  /// Выключаю микрофонный исилитель
  MAX9814_Off();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

///---------------------------------------------------------------------------
/// Динамические элементы выводим только на основной экран
/// Вывод текста блокирует экран до нажатия любой клавиши
/// После текста на экране отображается основной экран 
///---------------------------------------------------------------------------
//#define D_PROFILER
#ifdef D_PROFILER
  const U8 TERMINAL_B = 3;
  const U8 TERMINAL_E = 4;
  extern C_Profiler Profiler;
#endif
__task void Task_Terminal(void) 
{
  static bool boRestore = true;         /// = true еслии необходимо восстановить основной экран

  for(;;){
    os_dly_wait(10);    // 100 мс.

    // Обработка нажатых клавиш
    U8 ucActiveKey = StateHandler->GetActiveKey();
    if(ACTIVE_NO != ucActiveKey){
      Terminal.KBD_Handler(ucActiveKey);
      StateHandler->ClearActiveKey();
    }
    // Состояния обработчика текстовых сообщений
    // 0 - готов к обработке нового TextMsg
    // 1 - приняли указатель на TextMsg
    // 2 - передали текст LCD монитору, сохранили состояние клавиатуры перевели KBD в режим Text
    /// Вывод текста
    if(TEXT_STATE_START == Terminal.ucTextState) {
      Terminal.ucTextState = TEXT_STATE_END;
      Terminal.ucKBDRegimeTemp = Terminal.ucKBDRegime;
      Terminal.ucKBDRegime = KBD_REGIME_TEXT;
      Terminal.ShowTextFromDC(Terminal.pTextMsg);
    }

    if(KBD_REGIME_OFF == Terminal.ucKBDRegime){ 
      Terminal.ucKBDRegime = Terminal.ucKBDRegimeTemp;
      if(KBD_REGIME_MENU == Terminal.ucKBDRegime) {
        // Восстановление исходного состояния в для режима MENU
        Terminal.MENU_esc();
        Terminal.MENU_enter();
      }
      // Восстановление исходного состояния в для режима BASE
      else boRestore = true;
      Terminal.ucTextState = TEXT_STATE_NO;  // Завершение режима  TEXT
    }

    // Восстановление экрана
    if(KBD_REGIME_BASE == Terminal.ucKBDRegime){ 
      if(StateHandler->GetUpdateStatus()){
        // Если изменилось содержание основного экрана, то запускаю процедуру восстановления экрана
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

