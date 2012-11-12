//============================================================================
// ����                                     
///----------------------------------------------------------------------------
/// ������������� ���� ������ 0 ��������� �� ��, ��� ������ 
/// ��������� �� ��������� ����.
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
/// ����������� �������������� ������� ����� ��������������� �������� ������� 
/// ������ ����. � ����������� �� �������� ������ ���� ���������� �� ��� ���� 
/// ����� ��������������.
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
/// ���������� ����� ������ �� ������� ���������� ������
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
/// ��������� � ��������� ���� �������� ����. (��� �������� �� ����������� 
/// ������� ����������� ������ ������� ����. (��������  � ��������� ��������� 
/// �������)
///---------------------------------------------------------------------------
void C_Menu::SaveBackTrack(void)
{
  U8  ucCursorPosition = 0; 
  // ������� ������� �������������, � ���������� scCursorPosition �������� �����
  // ������ ���� �� ������� ���������� ������ � ������ ���� �����
  S_CursorID stID;
  stID.uiID = GetID(); 
  if(0 != stID.SubID.ucID_3) {ucCursorPosition = stID.SubID.ucID_3; stID.SubID.ucID_3 = 0;}
  else if(0 != stID.SubID.ucID_2) {ucCursorPosition = stID.SubID.ucID_2; stID.SubID.ucID_2 = 0;}
       else if(0 != stID.SubID.ucID_1) {ucCursorPosition = stID.SubID.ucID_1; stID.SubID.ucID_1 = 0;}
            else if(0 != stID.SubID.ucID_0) {ucCursorPosition = stID.SubID.ucID_0; stID.SubID.ucID_0 = 0;}
                 else return;
  // ������� �������� ��� �������� ������ ����
  const S_ItemProperties * pItemProperties = GetItemProperties(stID.uiID);  
  if(NULL == pItemProperties)  return; // ????? 

  // ������� � ��������� ��������� ������� �� ������� ���������
  const_cast< S_ItemProperties *>(pItemProperties)->ucCursorPosition = ucCursorPosition;
}
//----------------------------------------------------------------------------

///---------------------------------------------------------------------------
/// ��������� ������ �� ���� ������� ����
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
/// ��������� ������ �� ���� ������� �����
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
// ��������� ������� ������� "Esc" 
//----------------------------------------------------------------------------
/// ���� ������ ��������� � ���� 0-�� ������, �� �������������� ������� 
/// ������������� ����������������� �������� 0, ������� ��������� �� 
/// �� ��� ���������� ����� �� ������ ����. � ����� ������ ������ ������ 
/// ����������� �� 2 ������ �����. ������� �� ��� ������ ��������� ��� ��� 
/// ����� ������ ��������� ������������ ���� ���������� ���� ����������� �� 
/// ���� ������� ����. �.� ���������� �� ������������ ������ �� ���� ������� 
/// �����
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
/// ���������� ��������� �� �������� ����� ���� � ��������������� uiID
/// ���� �������� � ����� ��������������� ���, �� ���������� NULL  
///---------------------------------------------------------------------------
const S_ItemProperties * C_Menu::GetItemProperties(U32 uiID) 
{
  U16 usItem;
  for(usItem = 0; usItem < ucCountInitItems; ++usItem)
    if(MenuProperties[usItem].stID.uiID == uiID) break;
  if (ucCountInitItems == usItem) return NULL; /// ��� �������� � ����� ���������������. 
  return &MenuProperties[usItem];  
}
//----------------------------------------------------------------------------

///---------------------------------------------------------------------------
/// ���������� ��������� �� �������� �������� ������ ����
///---------------------------------------------------------------------------
const S_ItemProperties * C_Menu::GetItemProperties(void)
{
  return GetItemProperties(CursorID.uiID); 
}
//----------------------------------------------------------------------------

///---------------------------------------------------------------------------
/// ���������� ������� ����� ������ ����
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
/// �������������� ����� ����
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
/// � ���������� mapMenuProperties �������� ��������� ���� � �������� ��� �������.
/// ����� Init �������������� ��� ����������, �� ���� ������� ����
///---------------------------------------------------------------------------
// ��������� ������� ����
const char pstrMenu_0[]  = "����\r\n���������� ������\r\n���������\r\n������ � ���������";

const char pstrMenu_1[]  = "���������� ������\r\n���\r\n�������\r\n������ ������\r\n���\r\n����������� ������\r\n������ ������������\r\n���������";
const char pstrMenu_2[]  = "���������\r\n����� ��������\r\n�������� ��������\r\n���� � �����\r\n��������";
const char pstrMenu_3[]  = "������ � ���������\r\n��������� ������";

const char pstrMenu_21[]  = "����� ��������\r\n������ ��������\r\n�� ������ ��������� ���\r\n����� �������� ��������\r\n������ ����������-������ ����\r\n������� � ����\r\n������� � ���� �����������\r\n����� ����";
const char pstrMenu_22[]  = "�������� ��������\r\n��������� ����������\r\n��� ����������� �������\r\n�������� ������\r\n�������� �������";
const char pstrMenu_23[]  = "���� � �����\r\n����������� �������������\r\n����������� ������\r\n���������������� �������\r\n��������� �������\r\n���";
const char pstrMenu_24[]  = "��������\r\n����� � ��������\r\n����\r\n��� �����\r\n������ � ����\r\n����� ������";

const char pstrMenu_31[]  = "��������� ������\r\n�������\r\n�������������\r\n���������\r\n������� �������";

const char pstrMenu_313[]  = "���������\r\n���\r\n����";
const char pstrMenu_314[]  = "������� �������\r\n���\r\n����";

void C_Menu::Init(void)
{
  // �������� ���� 0-�� ������
  InitMenuProperties(ITEM_ID(0, 0, 0, 0), 3, 1, 0, pstrMenu_0, NULL);

  // �������� ������� ���� 0-�� ������
  InitMenuProperties(ITEM_ID(1, 0, 0, 0), 7, 1, 0, pstrMenu_1, NULL);
  InitMenuProperties(ITEM_ID(2, 0, 0, 0), 4, 1, 0, pstrMenu_2, NULL);
  InitMenuProperties(ITEM_ID(3, 0, 0, 0), 1, 1, 0, pstrMenu_3, NULL);

  // �������� ������� ���� 1-�� ������
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
                                                           
  // �������� ������� ���� 2-�� ������
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
                                                          
  // �������� ������� ���� 3-�� ������
  InitMenuProperties(ITEM_ID(3, 1, 3, 1), 0, 0, 0, NULL, &C_Menu::Menu_3131);
  InitMenuProperties(ITEM_ID(3, 1, 3, 2), 0, 0, 0, NULL, &C_Menu::Menu_3132);
  InitMenuProperties(ITEM_ID(3, 1, 4, 1), 0, 0, 0, NULL, &C_Menu::Menu_3141);
  InitMenuProperties(ITEM_ID(3, 1, 4, 2), 0, 0, 0, NULL, &C_Menu::Menu_3142);
} 
//----------------------------------------------------------------------------

// ������ 
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
