#ifndef ERRORHANDLER_H
#define ERRORUANDLER_H
//=============================================================================
// ��������� ������                                    
//============================================================================

#include <RTL.h>
#include "msg.h"
//----------------------------------------------------------------------------

// ���� ��� ���� �����
const U32 ERROR_FLAG = 0x0001;   
//  ������ ��������� �� ������ ������ � ����������� 0
static const U8 SIZE_DATA_MSGERROR = 16;  
extern U32 PoolMsgError[];
extern OS_TID idtErrorHandler;
//----------------------------------------------------------------------------

class C_MsgError : public C_Msg
{
private:
  C_MsgError(const C_MsgError &);
  C_MsgError &operator=(const C_MsgError & rhs);
public:
  C_MsgError(){};
  char str[SIZE_DATA_MSGERROR]; 
        
  void * operator new(size_t) throw() { void *pMsg = NULL; pMsg = _alloc_box(PoolMsgError); return pMsg; }
  static C_MsgError * NewMsgError(U8 ucIDAcceptor, U32 *pMailBoxSource, U16 usFlags, U16 usLength);
  virtual char * PointerStr(void) {return str;}
  virtual void * Release(void){ if(--usRefCount == 0){ _free_box (PoolMsgError, this); return NULL; } 
                                else return this; }   
};
//----------------------------------------------------------------------------

///---------------------------------------------------------------------------
/// ����  ucCount >= COUNTMAX ������ ��������� ������������, �.� ����� �������������� 
/// ������ ������ �������� ������. �������� ������ ������ ����� �������
/// ����� ErrorToList() ���������� ������ ��������. ����� ��������� ������ � ������ 
/// ������ � ������������� ���� ������.

//----------------------------------------------------------------------------
const U8 ERROR_COUNTMAX = 20;    
class C_ErrorHandler {
  friend void Task_ErrorHandler(void);
private:
  U8 ucCount;
  U32 puiListError[ERROR_COUNTMAX];
public:
  bool boEnableErrPrint;
  C_ErrorHandler();
  void ErrorToList(U32 uiError){ puiListError[ucCount++ % ERROR_COUNTMAX] = uiError;
                                 os_evt_set (ERROR_FLAG, idtErrorHandler); }
};
//----------------------------------------------------------------------------

extern C_ErrorHandler ErrorHandler;
__task void Task_ErrorHandler(void);
//----------------------------------------------------------------------------
#endif //#ifndef ERRORUANDLER_H
