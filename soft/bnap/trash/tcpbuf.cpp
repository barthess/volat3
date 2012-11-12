///============================================================================
/// Буфер для сборки ТСР пакетов                                    
//============================================================================

#include "tcpbuf.h"

//---------------------------------------------------------------------------
// Сравнивает uiSize первых символов циклического буфера и uiSize первых 
// символов массива pX
//---------------------------------------------------------------------------
int C_TCPBuf::CmpBufBeg(const unsigned char *pX, unsigned int uiSize) const {
  if(uiSize > uiCount) return BUFTCP_NODATA; // Часть данных еще не получена
  if(SizeMax() > (uiPtrRead + uiSize)){
    if(0 != memcmp(pX, pBuf + uiPtrRead, uiSize)) return BUFTCP_NOCMP;
    else return BUFTCP_OK;
  }
  else {
    unsigned int uiSizeEnd = SizeMax() - uiPtrRead;
    if(0 != memcmp(pX, pBuf + uiPtrRead, uiSizeEnd)) return BUFTCP_NOCMP;
    else if(0 != memcmp(pX + uiSizeEnd, pBuf + 0, uiSize - uiSizeEnd)) return BUFTCP_NOCMP;
         else return BUFTCP_OK;
  }
}
//----------------------------------------------------------------------------
	   
