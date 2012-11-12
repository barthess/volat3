//============================================================================
//                                    
//============================================================================

#include <RTL.h>
#include "errorhandler.h"
#include "msg.h"
//----------------------------------------------------------------------------

void Restart(void);
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// �������� ������ ��� ��������� � �������������� ���������.
//----------------------------------------------------------------------------
C_Msg * NewMsg(U8 ucIDAcceptor, U8 ucIDSource, U32 *pMailBoxSource, U16 usFlags, U16 usLength){
  C_Msg *pMsg = NULL;
  char *pcData = new char [usLength];
  if(NULL != pcData){
    pMsg = new C_MsgNew(pcData);
    if(NULL != pMsg) {
      pMsg->usRefCount = 1; pMsg->ucIDAcceptor  = ucIDAcceptor;  pMsg->ucIDSource = ucIDSource;
      pMsg->pMailBoxSource = pMailBoxSource; pMsg->usFlags = usFlags; pMsg->usLength = usLength;
      return pMsg;
    }
    else {
      delete[] pcData;
    }
  }
  // ����� ��������� ������. ������� Restart() ������������� �������. return NULL 
  //  ����� ������ ��� ���� ��� �� ������ ��������� �����������.
  Restart();     
  return NULL;
}
//----------------------------------------------------------------------------
