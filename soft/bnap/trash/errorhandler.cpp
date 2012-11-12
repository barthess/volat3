//============================================================================
// Обработка ошибок                                    
//============================================================================
//                                                   
//============================================================================

#include <string.h>
#include <stdio.h>
#include "linkpc.h"
#include "errorhandler.h"
#include "msg.h"
//----------------------------------------------------------------------------

void CharToHex(char* pOut, const char* pIn, U16 usInSize);
//----------------------------------------------------------------------------

// pc - массив на 2 символа
void sprintf_16(char * pc, U16 us){
  CharToHex(pc + 0, reinterpret_cast<const char*>(&us) + 1, 1);
  CharToHex(pc + 2, reinterpret_cast<const char*>(&us) + 0, 1);
}

// pc - массив на 4 символа
//----------------------------------------------------------------------------
void sprintf_32(char * pc, U32 ui){
  CharToHex(pc + 0, reinterpret_cast<const char*>(&ui) + 3, 1);
  CharToHex(pc + 2, reinterpret_cast<const char*>(&ui) + 2, 1);
  CharToHex(pc + 4, reinterpret_cast<const char*>(&ui) + 1, 1);
  CharToHex(pc + 6, reinterpret_cast<const char*>(&ui) + 0, 1);
}
//----------------------------------------------------------------------------

C_ErrorHandler ErrorHandler;
OS_TID idtErrorHandler;
//----------------------------------------------------------------------------

/// Пул памяти для сообщений об ошибках
static const U8 POOL_MSGERROR_NUMBER = 10;
_declare_box(PoolMsgError, sizeof(C_MsgError), POOL_MSGERROR_NUMBER);
//----------------------------------------------------------------------------

inline C_ErrorHandler::C_ErrorHandler(): boEnableErrPrint (false) {
  _init_box(PoolMsgError,  sizeof (PoolMsgError), sizeof(C_MsgError));
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Выделяет память для сообщения и инициализирует сообщение.
//----------------------------------------------------------------------------
C_MsgError * C_MsgError::NewMsgError(U8 ucIDAcceptor, U32 *pMailBoxSource, U16 usFlags, U16 usLength){
  C_MsgError * p = new C_MsgError;
  if(NULL != p){
    p->usRefCount = 1; p->ucIDAcceptor  = ucIDAcceptor;  p->ucIDSource = ID_ERRORHANDLER;
    p->pMailBoxSource = pMailBoxSource; p->usFlags = usFlags; p->usLength = usLength;
  }
  return p;
}
//----------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Для сообщений об ошибках использую свой пул памяти
//----------------------------------------------------------------------------
bool print_error(const char *pstrz){
  C_MsgError * pMsg  = C_MsgError::NewMsgError(ID_LINKPC, NOMAILBOX, MSG_RESPONSE, strlen(pstrz));
  if(NULL == pMsg) return false;    /// Переполнен пул сообщений
  memcpy(pMsg->PointerStr(), pstrz, strlen(pstrz));
  // Отправляю сообщениe
  if(OS_R_OK != os_mbx_send(LinkPC.GetPtrMailbox(), pMsg, TIMEOUT_MAILBUX_REDY)){
    pMsg->Release();
    return false;
  }
  return true;
}
//----------------------------------------------------------------------------

char pc[SIZE_DATA_MSGERROR] = "Err 0x--------\r";  // Не изменять.
__task void Task_ErrorHandler(void)
{
  for(;;){
    os_evt_wait_or (ERROR_FLAG, 0xFFFF);
    if(ErrorHandler.boEnableErrPrint){
      for(U16 i = 0; i < ErrorHandler.ucCount; ++i){
        sprintf_32(pc + 6, ErrorHandler.puiListError[i % ERROR_COUNTMAX]);
        print_error(pc);
      }
    }
    ErrorHandler.ucCount = 0;
  }
}
//----------------------------------------------------------------------------

