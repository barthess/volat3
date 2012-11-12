//============================================================================
// Меню                                     
///----------------------------------------------------------------------------
/// Идентификатор меню равный 0 указывает на то, что курсор 
/// находится за пределами меню.
//============================================================================

#include <string.h>
#include "parameters.h"
#include "gsm.h"
#include "menu.h"
#include "dc.h"
//----------------------------------------------------------------------------

void MAX9814_On(void);
//----------------------------------------------------------------------------

C_Menu::C_Menu(void): boMenuRound(false), ucCountInitItems(0) {
  CursorID.uiID = 0; 
  Init();
}
//----------------------------------------------------------------------------

///---------------------------------------------------------------------------
/// Присваивает идентификатору курсора номер соответствующий заданной позиции 
/// внутри меню. В зависимости от текущего уровня меню изменяется та или иная 
/// часть идентификатора.
///---------------------------------------------------------------------------
void C_Menu::PositionToID(U8 ucPosition)
{
  ucPosition = ucPosition & 0x0F;
  switch(GetLevel())
  {
    case -1: CursorID.SubID.ucID_0 = ucPosition;
             break;
    case  0: CursorID.SubID.ucID_1 = ucPosition;
             CursorID.SubID.ucID_2 = 0;
             CursorID.SubID.ucID_3 = 0;
             break;
    case  1: CursorID.SubID.ucID_2 = ucPosition;
             CursorID.SubID.ucID_3 = 0;
             break;
    case  2: CursorID.SubID.ucID_3 = ucPosition;
  }  
}
//----------------------------------------------------------------------------

///---------------------------------------------------------------------------
/// Возвращает номер пункта на котором расположен курсор
///---------------------------------------------------------------------------
U8 C_Menu::IDToPosition(void)
{
  if(0 == CursorID.uiID) return 0;
  if(0 == CursorID.SubID.ucID_1) return CursorID.SubID.ucID_0;
  if(0 == CursorID.SubID.ucID_2) return CursorID.SubID.ucID_1;
  if(0 == CursorID.SubID.ucID_3) return CursorID.SubID.ucID_2;
  return CursorID.SubID.ucID_3;
}
//----------------------------------------------------------------------------

///---------------------------------------------------------------------------
/// Сохраняет в свойствах меню обратный путь. (При переходе на нижележащий 
/// уровень модифицирут массив свойств меню. (Изменяет  в свойствах положение 
/// курсора)
///---------------------------------------------------------------------------
void C_Menu::SaveBackTrack(void)
{
  U8  ucCursorPosition = 0; 
  // Получаю текущий идентификатор, в переменной scCursorPosition сохраняю номер
  // пункта меню на котором установлен курсор и очищаю этот номер
  S_CursorID stID;
  stID.uiID = GetID(); 
  if(0 != stID.SubID.ucID_3) {ucCursorPosition = stID.SubID.ucID_3; stID.SubID.ucID_3 = 0;}
  else if(0 != stID.SubID.ucID_2) {ucCursorPosition = stID.SubID.ucID_2; stID.SubID.ucID_2 = 0;}
       else if(0 != stID.SubID.ucID_1) {ucCursorPosition = stID.SubID.ucID_1; stID.SubID.ucID_1 = 0;}
            else if(0 != stID.SubID.ucID_0) {ucCursorPosition = stID.SubID.ucID_0; stID.SubID.ucID_0 = 0;}
                 else return;
  // Получаю свойства для текущего пункта меню
  const S_ItemProperties * pItemProperties = GetItemProperties(stID.uiID);  
  if(NULL == pItemProperties)  return; // ????? 

  // Заменяю в свойствах положение курсора на текущее положение
  const_cast< S_ItemProperties *>(pItemProperties)->ucCursorPosition = ucCursorPosition;
}
//----------------------------------------------------------------------------

///---------------------------------------------------------------------------
/// Переводит курсор на одну позицию вниз
///---------------------------------------------------------------------------
S_CursorID C_Menu::Down(void)
{
  switch(GetLevel())
  {
    case 0: if(CursorID.SubID.ucID_0 < ucNumberItems) ++CursorID.SubID.ucID_0;
            else if(boMenuRound) CursorID.SubID.ucID_0 = ucNumberItems; 
            break;
    case 1: if(CursorID.SubID.ucID_1 < ucNumberItems) ++CursorID.SubID.ucID_1;
            else if(boMenuRound) CursorID.SubID.ucID_1 = ucNumberItems; 
            break;
    case 2: if(CursorID.SubID.ucID_2 < ucNumberItems) ++CursorID.SubID.ucID_2;
            else if(boMenuRound) CursorID.SubID.ucID_2 = ucNumberItems; 
            break;
    case 3: if(CursorID.SubID.ucID_3 < ucNumberItems) ++CursorID.SubID.ucID_3;
            else if(boMenuRound) CursorID.SubID.ucID_3 = ucNumberItems; 
            break;
  }
  return CursorID;
}
//----------------------------------------------------------------------------

///---------------------------------------------------------------------------
/// Переводит курсор на одну позицию вверх
///---------------------------------------------------------------------------
S_CursorID C_Menu::UP(void)
{
  switch(GetLevel())
  {
    case 0: if(CursorID.SubID.ucID_0 > 1) --CursorID.SubID.ucID_0;
            else if(boMenuRound) CursorID.SubID.ucID_0 = ucNumberItems; 
            break;
    case 1: if(CursorID.SubID.ucID_1 > 1) --CursorID.SubID.ucID_1;
            else if(boMenuRound) CursorID.SubID.ucID_1 = ucNumberItems; 
            break;
    case 2: if(CursorID.SubID.ucID_2 > 1) --CursorID.SubID.ucID_2;
            else if(boMenuRound) CursorID.SubID.ucID_2 = ucNumberItems; 
            break;
    case 3: if(CursorID.SubID.ucID_3 > 1) --CursorID.SubID.ucID_3;
            else if(boMenuRound) CursorID.SubID.ucID_3 = ucNumberItems; 
            break;
  }
  return CursorID;
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Обработка нажатия клавиши "Esc" 
//----------------------------------------------------------------------------
/// Если маркер находится в меню 0-го уровня, то идентификатору маркера 
/// присваивается зарезервированное значение 0, которое указывает на 
/// то что необходимо выйти из режима меню. В любом другом случае маркер 
/// переводится на 2 уровня вверх. Переход на два уровня необходим так как 
/// после вызова процедуры формирования меню раскроется меню находящееся на 
/// один уровень ниже. Т.е фактически мы переместимся только на один уровень 
/// вверх
///---------------------------------------------------------------------------
S_CursorID C_Menu::Esc(void)
{
  switch(GetLevel()){
    case 1: CursorID.SubID.ucID_0 = 0;
    case 2: CursorID.SubID.ucID_1 = 0;
  }  
  CursorID.SubID.ucID_2 = 0;
  CursorID.SubID.ucID_3 = 0;
  return CursorID;
}
//----------------------------------------------------------------------------

///---------------------------------------------------------------------------
/// Возвращает указатель на свойства пукта меню с идентификатором uiID
/// Если элемента с таким идентификатором нет, то возвращает NULL  
///---------------------------------------------------------------------------
const S_ItemProperties * C_Menu::GetItemProperties(U32 uiID) 
{
  U16 usItem;
  for(usItem = 0; usItem < ucCountInitItems; ++usItem)
    if(MenuProperties[usItem].stID.uiID == uiID) break;
  if (ucCountInitItems == usItem) return NULL; /// Нет элемента с таким идентификатором. 
  return &MenuProperties[usItem];  
}
//----------------------------------------------------------------------------

///---------------------------------------------------------------------------
/// Возвращает указатель на свойства текущего пункта меню
///---------------------------------------------------------------------------
const S_ItemProperties * C_Menu::GetItemProperties(void)
{
  return GetItemProperties(CursorID.uiID); 
}
//----------------------------------------------------------------------------

///---------------------------------------------------------------------------
/// Возвращает текущий номер уровня меню
///---------------------------------------------------------------------------
S8 C_Menu::GetLevel(void)
{
  if(0 == CursorID.uiID) return -1;
  if(0 == CursorID.SubID.ucID_1) return 0;
  if(0 == CursorID.SubID.ucID_2) return 1;
  if(0 == CursorID.SubID.ucID_3) return 2;
  return 3;
}
//----------------------------------------------------------------------------

///---------------------------------------------------------------------------
/// Инициализирует пункт меню
///---------------------------------------------------------------------------
bool C_Menu::InitMenuProperties(U32 uiID, U8 ucNumberItems, U8 ucCursorPosition, 
                        U8 ucFlags, const char *pNameList, pfuncMenuHandler pfunc)
{
  if(MENU_ITEMS_MAX <= ucCountInitItems) return false;
  MenuProperties[ucCountInitItems].stID.uiID = uiID;
  MenuProperties[ucCountInitItems].ucNumberItems = ucNumberItems;
  MenuProperties[ucCountInitItems].ucCursorPosition = ucCursorPosition;
  MenuProperties[ucCountInitItems].pNameList = pNameList;
  MenuProperties[ucCountInitItems].pfunc = pfunc;
  ++ucCountInitItems;
  return true; 

}
//----------------------------------------------------------------------------
 
///---------------------------------------------------------------------------
/// В переменной mapMenuProperties хранится структура меню и свойства его пунктов.
/// Метод Init инициализирует эту переменную, по сути создает меню
///---------------------------------------------------------------------------
// Заголовки пунктов меню
const char pstrMenu_0[]  = "Меню\r\nЭкстренные вызовы\r\nСообщения\r\nСервис и настройки";

const char pstrMenu_1[]  = "Экстренные вызовы\r\nМЧС\r\nМилиция\r\nСкорая помощь\r\nГАИ\r\nТехническая служба\r\nСлужба безопасности\r\nДиспетчер";
const char pstrMenu_2[]  = "Сообщения\r\nРежим движения\r\nЗадержка движения\r\nСход с линии\r\nПарковка";
const char pstrMenu_3[]  = "Сервис и настройки\r\nНастройки экрана";

const char pstrMenu_21[]  = "Режим движения\r\nНачало движения\r\nНа трассе замечаний нет\r\nСмена маршрута движения\r\nРабота законченна-ранний сход\r\nВозврат в парк\r\nВозврат в парк буксировкой\r\nНужен обед";
const char pstrMenu_22[]  = "Задержка движения\r\nСкопление транспорта\r\nДТП посторонних средств\r\nДорожные работы\r\nПогодные условия";
const char pstrMenu_23[]  = "Сход с линии\r\nТехническая неисправность\r\nНеисправная резина\r\nЭксплуатационные причины\r\nРемонтная бригада\r\nДТП";
const char pstrMenu_24[]  = "Парковка\r\nГотов к движению\r\nОбед\r\nНет смены\r\nприбыл в парк\r\nКонец работы";

const char pstrMenu_31[]  = "Настройки экрана\r\nЯркость\r\nКонтрастность\r\nПодсветка\r\nМигание курсора";

const char pstrMenu_313[]  = "Подсветка\r\nВкл\r\nОткл";
const char pstrMenu_314[]  = "Мигание курсора\r\nВкл\r\nОткл";

void C_Menu::Init(void)
{
  // Свойства меню 0-го уровня
  InitMenuProperties(ITEM_ID(0, 0, 0, 0), 3, 1, 0, pstrMenu_0, NULL);

  // Свойства пунктов меню 0-го уровня
  InitMenuProperties(ITEM_ID(1, 0, 0, 0), 7, 1, 0, pstrMenu_1, NULL);
  InitMenuProperties(ITEM_ID(2, 0, 0, 0), 4, 1, 0, pstrMenu_2, NULL);
  InitMenuProperties(ITEM_ID(3, 0, 0, 0), 1, 1, 0, pstrMenu_3, NULL);

  // Свойства пунктов меню 1-го уровня
  InitMenuProperties(ITEM_ID(1, 1, 0, 0), 0, 0, 0, NULL, &C_Menu::Menu_11);
  InitMenuProperties(ITEM_ID(1, 2, 0, 0), 0, 0, 0, NULL, &C_Menu::Menu_12);
  InitMenuProperties(ITEM_ID(1, 3, 0, 0), 0, 0, 0, NULL, &C_Menu::Menu_13);
  InitMenuProperties(ITEM_ID(1, 4, 0, 0), 0, 0, 0, NULL, &C_Menu::Menu_14);
  InitMenuProperties(ITEM_ID(1, 5, 0, 0), 0, 0, 0, NULL, &C_Menu::Menu_15);
  InitMenuProperties(ITEM_ID(1, 6, 0, 0), 0, 0, 0, NULL, &C_Menu::Menu_16);
  InitMenuProperties(ITEM_ID(1, 7, 0, 0), 0, 0, 0, NULL, &C_Menu::Menu_17);

  InitMenuProperties(ITEM_ID(2, 1, 0, 0), 7, 1, 0, pstrMenu_21, NULL);
  InitMenuProperties(ITEM_ID(2, 2, 0, 0), 4, 1, 0, pstrMenu_22, NULL);
  InitMenuProperties(ITEM_ID(2, 3, 0, 0), 5, 1, 0, pstrMenu_23, NULL);
  InitMenuProperties(ITEM_ID(2, 4, 0, 0), 5, 1, 0, pstrMenu_24, NULL);

  InitMenuProperties(ITEM_ID(3, 1, 0, 0), 4, 1, 0, pstrMenu_31, NULL);
                                                           
  // Свойства пунктов меню 2-го уровня
  InitMenuProperties(ITEM_ID(2, 1, 1, 0), 0, 0, 0, NULL, &C_Menu::Menu_211);
  InitMenuProperties(ITEM_ID(2, 1, 2, 0), 0, 0, 0, NULL, &C_Menu::Menu_212);
  InitMenuProperties(ITEM_ID(2, 1, 3, 0), 0, 0, 0, NULL, &C_Menu::Menu_213);
  InitMenuProperties(ITEM_ID(2, 1, 4, 0), 0, 0, 0, NULL, &C_Menu::Menu_214);
  InitMenuProperties(ITEM_ID(2, 1, 5, 0), 0, 0, 0, NULL, &C_Menu::Menu_215);
  InitMenuProperties(ITEM_ID(2, 1, 6, 0), 0, 0, 0, NULL, &C_Menu::Menu_216);
  InitMenuProperties(ITEM_ID(2, 1, 7, 0), 0, 0, 0, NULL, &C_Menu::Menu_217);

  InitMenuProperties(ITEM_ID(2, 2, 1, 0), 0, 0, 0, NULL, &C_Menu::Menu_221);
  InitMenuProperties(ITEM_ID(2, 2, 2, 0), 0, 0, 0, NULL, &C_Menu::Menu_222);
  InitMenuProperties(ITEM_ID(2, 2, 3, 0), 0, 0, 0, NULL, &C_Menu::Menu_223);
  InitMenuProperties(ITEM_ID(2, 2, 4, 0), 0, 0, 0, NULL, &C_Menu::Menu_224);

  InitMenuProperties(ITEM_ID(2, 3, 1, 0), 0, 0, 0, NULL, &C_Menu::Menu_231);
  InitMenuProperties(ITEM_ID(2, 3, 2, 0), 0, 0, 0, NULL, &C_Menu::Menu_232);
  InitMenuProperties(ITEM_ID(2, 3, 3, 0), 0, 0, 0, NULL, &C_Menu::Menu_233);
  InitMenuProperties(ITEM_ID(2, 3, 4, 0), 0, 0, 0, NULL, &C_Menu::Menu_234);
  InitMenuProperties(ITEM_ID(2, 3, 5, 0), 0, 0, 0, NULL, &C_Menu::Menu_235);

  InitMenuProperties(ITEM_ID(2, 4, 1, 0), 0, 0, 0, NULL, &C_Menu::Menu_241);
  InitMenuProperties(ITEM_ID(2, 4, 2, 0), 0, 0, 0, NULL, &C_Menu::Menu_242);
  InitMenuProperties(ITEM_ID(2, 4, 3, 0), 0, 0, 0, NULL, &C_Menu::Menu_243);
  InitMenuProperties(ITEM_ID(2, 4, 4, 0), 0, 0, 0, NULL, &C_Menu::Menu_244);
  InitMenuProperties(ITEM_ID(2, 4, 5, 0), 0, 0, 0, NULL, &C_Menu::Menu_245);

  InitMenuProperties(ITEM_ID(3, 1, 1, 0), 0, 0, 0, NULL, &C_Menu::Menu_311);
  InitMenuProperties(ITEM_ID(3, 1, 2, 0), 0, 0, 0, NULL, &C_Menu::Menu_312);
  InitMenuProperties(ITEM_ID(3, 1, 3, 0), 2, 1, 0, pstrMenu_313, NULL);
  InitMenuProperties(ITEM_ID(3, 1, 4, 0), 2, 1, 0, pstrMenu_314, NULL);
                                                          
  // Свойства пунктов меню 3-го уровня
  InitMenuProperties(ITEM_ID(3, 1, 3, 1), 0, 0, 0, NULL, &C_Menu::Menu_3131);
  InitMenuProperties(ITEM_ID(3, 1, 3, 2), 0, 0, 0, NULL, &C_Menu::Menu_3132);
  InitMenuProperties(ITEM_ID(3, 1, 4, 1), 0, 0, 0, NULL, &C_Menu::Menu_3141);
  InitMenuProperties(ITEM_ID(3, 1, 4, 2), 0, 0, 0, NULL, &C_Menu::Menu_3142);
} 
//----------------------------------------------------------------------------

// Звонок 
//U32 C_Menu::Menu_11(const S_ItemProperties &stPropert){
//  char strzResponse[8];
//  U32 uiError = GSM.CMDHandler("msTelNum", Parameters->GetTelephoneNumber(0), strzResponse, sizeof(strzResponse) - 1, ID_MENU);
//  if(NO_ERROR == uiError){
//    uiError = GSM.CMDHandler("meCallUp", Parameters->GetTelephoneNumber(0), strzResponse, sizeof(strzResponse) - 1, ID_MENU);
//    if(NO_ERROR == uiError) MAX9814_On();
//  }
//  return uiError;  
//}
//----------------------------------------------------------------------------

U32 C_Menu::SendIDtoDC(const S_ItemProperties &stPropert){
  S_NumMessagePacket NumMessagePacket(GetID());
  DC.SendPacketToDC((char *)&NumMessagePacket, sizeof(S_NumMessagePacket));
  return 0;
}
//----------------------------------------------------------------------------
