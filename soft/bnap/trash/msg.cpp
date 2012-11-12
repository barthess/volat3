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
// Выделяет память для сообщения и инициализирует сообщение.
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
  // Очень серьезная ошибка. Функция Restart() перезагружает систему. return NULL 
  //  нужен только для тогл что бы небыло замечаний компилятора.
  Restart();     
  return NULL;
}
//----------------------------------------------------------------------------
