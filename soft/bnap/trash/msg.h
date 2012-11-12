#ifndef MSG_H
#define MSG_H
//============================================================================
// 
//============================================================================

#include <RTL.h>
#include "SysConfig.h"
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// ���� ���������
//----------------------------------------------------------------------------
enum  E_MSG_Type{
  MSG_CMDWITHOUTRESPONSE = 1,///  ��������� �������� �������� ��� ������������� 
  MSG_CMDWITHRESPONSE,       ///  ��������� �������� �������� � ��������������  
  MSG_RESPONSE,              ///  ��������� �������� ��������������     
  MSG_SIZEMISSING,           ///  ��������� �� ������ ��������: ��� ����� ��� ������ 
  MSG_TIMEOUT,               ///  ��������� �� ������ ��������: TIMEOUT   
  MSG_ERROR                  ///  ��������� �� ������ ��������   
};
//----------------------------------------------------------------------------

class C_Msg;
C_Msg * NewMsg(U8 ucIDAcceptor, U8 ucIDSource, U32 *pMailBoxSource, U16 usFlags, U16 usLength);
//----------------------------------------------------------------------------

class C_Msg
{
  friend C_Msg * NewMsg(U8 ucIDAcceptor, U8 ucIDSource, U32 *pMailBoxSource, U16 usFlags, U16 usLength);
private:
  C_Msg(const C_Msg &);
  C_Msg &operator=(const C_Msg & rhs);
protected:
  U16 usRefCount;      /// ������� ������.
  U16 usFlags;         /// �����.
  U16 usLength;        /// ����� ������ � ������.
  U8  ucIDAcceptor;    /// ������������� ���������� ���������
  U8  ucIDSource;      /// ������������� ��������� ���������
  U32 *pMailBoxSource; /// ��������� �� �������� ���� ��������� ���������
  C_Msg() {}
public:
  void AddRef(void){++usRefCount;}
  U16  GetFlags(void)  {return usFlags;}
  void SetFlags(U16 usFlags_) {usFlags = usFlags_;}
  U8   GetIDSource(void) {return ucIDSource;}
  void SetIDSource(U8 ucIDSource_) {ucIDSource = ucIDSource_;}
  U8   GetIDAcceptor(void) {return ucIDAcceptor;}
  void SetIDAcceptor(U8 ucIDAcceptor_) {ucIDAcceptor = ucIDAcceptor_;}
  U16  GetLength(void) {return usLength;}
  void SetLength(U16 usLength_) {usLength = usLength_;}
  U32* GetPointerMailbox(void){return pMailBoxSource;}
  void SetPointerMailbox(U32 *pMailBoxSource_){pMailBoxSource = pMailBoxSource_;}
  virtual char * PointerStr(void) = 0;
  virtual void * Release(void) = 0;
};
//----------------------------------------------------------------------------

class C_MsgNew : public C_Msg
{
private:
  C_MsgNew(const C_MsgNew &);
  C_MsgNew & operator=(const C_MsgNew & rhs);
  char *str;
public:
  C_MsgNew(char *str_) : str(str_) {};
  virtual char * PointerStr(void) {return str;}
  virtual void * Release(void){ if(--usRefCount == 0){ delete[] str; delete this; return NULL; } 
                                else return this;
  }   
};
//----------------------------------------------------------------------------

#endif //#ifndef MSG_H
