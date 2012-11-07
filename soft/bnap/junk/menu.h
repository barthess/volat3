#ifndef MAIN_H
#define MAIN_H
//============================================================================
// Меню                                     
//============================================================================

#include "sysconfig.h"
//----------------------------------------------------------------------------

/// Максимальное число пунктов меню. Не более 128
const U8 POSITION_MAX = 7;
#define MENU_ITEMS_MAX 128
#define ITEM_ID(M0, M1, M2, M3) ((M0 << 24) + (M1 << 16) + (M2 << 8) + M3)
#define ID_MENU_BASE ITEM_ID(0, 0, 0, 0)


#define ID_MENU_CALL ITEM_ID(1, 0, 0, 0)
#define ID_MENU_MSG  ITEM_ID(2, 0, 0, 0)
//----------------------------------------------------------------------------

class   C_Menu; 
struct  S_ItemProperties;
typedef U32(C_Menu::*pfuncMenuHandler)(const S_ItemProperties &);
C_Menu  * CreatMenu(void);
extern  C_Menu * Menu;
//----------------------------------------------------------------------------

union S_CursorID {
  U32 uiID;
  struct {
    U8 ucID_3;
    U8 ucID_2;
    U8 ucID_1;
    U8 ucID_0;
  }SubID;
};
//----------------------------------------------------------------------------

struct S_ItemProperties {
  S_CursorID stID;
  U8  ucNumberItems;       /// Число пунктов в меню (
  U8  ucCursorPosition;    /// Позиция курсора
  U16 usFlags;             /// 
  const char *pNameList;   /// Указателm на строку с названиями пунктов меню
  pfuncMenuHandler pfunc;  /// Указатель на функцию дополнительной обработки
};
//----------------------------------------------------------------------------

class C_Menu 
{
private:
  S_ItemProperties MenuProperties[MENU_ITEMS_MAX];
  S_CursorID CursorID;    /// Текущий идентификатор маркера
  bool boMenuRound;       /// Если true, то маркер перемещается при движении по меню по кругу
  U8   ucNumberItems;     /// Число пунктов в текущем меню
  U8   ucCountInitItems;  /// Число проинициализированных пунктов меню
  S8   GetLevel(void);
  void Init(void);
  bool InitMenuProperties(U32 uiID, U8 ucNumberItems, U8 ucCursorPosition, U8 ucFlags,
                         const char *pNameList, pfuncMenuHandler pfunc);
public:
  C_Menu(void);
  S_CursorID Down(void);
  S_CursorID UP(void);
  S_CursorID Esc(void); 
  U32  GetID(void) { return CursorID.uiID;}
  void SetID(U32 uiID) {CursorID.uiID = uiID;}
  void  PositionToID(U8 ucPosition);
  U8    IDToPosition(void);
  void  SetNumberItems(U8 ucNumberItems_) {ucNumberItems = ucNumberItems_;}
  const S_ItemProperties * GetItemProperties(U32 uiID);  
  const S_ItemProperties * GetItemProperties(void); 
  void MenuRound(bool boX) {boMenuRound = boX;}
  void SaveBackTrack(void);
  U32  SendIDtoDC(const S_ItemProperties &stPropert);

U32 Menu_11(const S_ItemProperties &stPropert){ return SendIDtoDC(stPropert);  }
  U32 Menu_12(const S_ItemProperties &stPropert){ return SendIDtoDC(stPropert);  }
  U32 Menu_13(const S_ItemProperties &stPropert){ return SendIDtoDC(stPropert);  }
  U32 Menu_14(const S_ItemProperties &stPropert){ return SendIDtoDC(stPropert);  }
  U32 Menu_15(const S_ItemProperties &stPropert){ return SendIDtoDC(stPropert);  }
  U32 Menu_16(const S_ItemProperties &stPropert){ return SendIDtoDC(stPropert);  }
  U32 Menu_17(const S_ItemProperties &stPropert){ return SendIDtoDC(stPropert);  }
  U32 Menu_31(const S_ItemProperties &stPropert){ return SendIDtoDC(stPropert);  }
  U32 Menu_211(const S_ItemProperties &stPropert){ return SendIDtoDC(stPropert);  }
  U32 Menu_212(const S_ItemProperties &stPropert){ return SendIDtoDC(stPropert);  }
  U32 Menu_213(const S_ItemProperties &stPropert){ return SendIDtoDC(stPropert);  }
  U32 Menu_214(const S_ItemProperties &stPropert){ return SendIDtoDC(stPropert);  }
  U32 Menu_215(const S_ItemProperties &stPropert){ return SendIDtoDC(stPropert);  }
  U32 Menu_216(const S_ItemProperties &stPropert){ return SendIDtoDC(stPropert);  }
  U32 Menu_217(const S_ItemProperties &stPropert){ return SendIDtoDC(stPropert);  }
  U32 Menu_221(const S_ItemProperties &stPropert){ return SendIDtoDC(stPropert);  }
  U32 Menu_222(const S_ItemProperties &stPropert){ return SendIDtoDC(stPropert);  }
  U32 Menu_223(const S_ItemProperties &stPropert){ return SendIDtoDC(stPropert);  }
  U32 Menu_224(const S_ItemProperties &stPropert){ return SendIDtoDC(stPropert);  }
  U32 Menu_231(const S_ItemProperties &stPropert){ return SendIDtoDC(stPropert);  }
  U32 Menu_232(const S_ItemProperties &stPropert){ return SendIDtoDC(stPropert);  }
  U32 Menu_233(const S_ItemProperties &stPropert){ return SendIDtoDC(stPropert);  }
  U32 Menu_234(const S_ItemProperties &stPropert){ return SendIDtoDC(stPropert);  }
  U32 Menu_235(const S_ItemProperties &stPropert){ return SendIDtoDC(stPropert);  }
  U32 Menu_241(const S_ItemProperties &stPropert){ return SendIDtoDC(stPropert);  }
  U32 Menu_242(const S_ItemProperties &stPropert){ return SendIDtoDC(stPropert);  }
  U32 Menu_243(const S_ItemProperties &stPropert){ return SendIDtoDC(stPropert);  }
  U32 Menu_244(const S_ItemProperties &stPropert){ return SendIDtoDC(stPropert);  }
  U32 Menu_245(const S_ItemProperties &stPropert){ return SendIDtoDC(stPropert);  }
  U32 Menu_311(const S_ItemProperties &stPropert){ return SendIDtoDC(stPropert);  }
  U32 Menu_312(const S_ItemProperties &stPropert){ return SendIDtoDC(stPropert);  }

  U32 Menu_3131(const S_ItemProperties &stPropert){ return SendIDtoDC(stPropert);  }
  U32 Menu_3132(const S_ItemProperties &stPropert){ return SendIDtoDC(stPropert);  }
  U32 Menu_3141(const S_ItemProperties &stPropert){ return SendIDtoDC(stPropert);  }
  U32 Menu_3142(const S_ItemProperties &stPropert){ return SendIDtoDC(stPropert);  }
};
//----------------------------------------------------------------------------

#endif //#ifndef MAIN_H
