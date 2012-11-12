#ifndef TCPBUF_H
#define TCPBUF_H
//===========================================================================
// Буфер для сборки ТСР пакетов                                    
//============================================================================

#include <RTL.h>
#include <string.h>
#include <climits>
#include "linkpc.h"
#include "SysConfig.h"
//----------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Циклический буфер
//---------------------------------------------------------------------------
template <typename T, U16 BUF_SIZE> 
class C_CyclBuf
{
protected:
  T pBuf[BUF_SIZE];
  unsigned int uiVolume;
  unsigned int uiCount;
  unsigned int uiPtrRead;
  unsigned int uiPtrWrite;
public:
  C_CyclBuf() : uiVolume(BUF_SIZE), uiCount(0), uiPtrRead(0), uiPtrWrite(0) {} 
//---------------------------------------------------------------------------
// Удаляет все элементы из буфера
//---------------------------------------------------------------------------
  void Clear(void) {uiCount = 0; uiPtrWrite = 0; uiPtrRead = 0;}

//---------------------------------------------------------------------------
// Удаляет uiSize элементов из буфера. Если число элементов в 
// буфере меньше uiSize то удаляются все элементы.
// Возвращаемое значение - число удаленных элементов
//---------------------------------------------------------------------------
  unsigned int Erase(int uiSize){
    if(uiSize > uiCount) uiSize = uiCount;  
	  uiPtrRead = uiPtrRead + uiSize;
	  uiPtrRead = uiPtrRead % BUF_SIZE;
	  uiCount = uiCount - uiSize;
	  return uiSize;
  }

//---------------------------------------------------------------------------
// Помещает элемент по адресу pX и удаляет его из буфера.  
// Возвращаемое значение - число полученных элементов (0 или 1)
// 0 если буфер пустой
//---------------------------------------------------------------------------
  unsigned int Get(T *pX){
    if(0 == uiCount) return 0;
	  *pX = pBuf[uiPtrRead++]; 
  	--uiCount;  
	  uiPtrRead = uiPtrRead % BUF_SIZE;
	  return 1;	  
  }

//---------------------------------------------------------------------------
// Помещает uiSize элементов по адресу pX  и удаляет их из буфера. Если число 
// элементов в  буфере меньше uiSize  то забирает все элементы. 
// Возвращаемое значение - число полученных элементов.
// 0 если буфер пустой
//---------------------------------------------------------------------------
  unsigned int Get(T *pX, unsigned int uiSize){
    if(uiSize > uiCount) uiSize = uiCount;
	  for(unsigned int  ui = 0; ui < uiSize; ++ui){
	    pX[ui] = pBuf[uiPtrRead++]; 
	    uiPtrRead = uiPtrRead % BUF_SIZE;
	  }
	  uiCount = uiCount - uiSize;
  	return uiSize;
  }
  
//---------------------------------------------------------------------------
// Помещает элемент в буфер  
// Возвращаемое значение - число помещенных элементов (0 или 1)
// 0 если буфер заполнен
//---------------------------------------------------------------------------
  unsigned int Insert(const T *pX){
    if(BUF_SIZE <= uiCount) return 0;
    pBuf[uiPtrWrite++] = *pX;  
	  uiPtrWrite = uiPtrWrite % BUF_SIZE;
	  ++uiCount;  
	  return 1;
  }

//---------------------------------------------------------------------------
// Помещает uiSize элементов в буфер. Если места в буфере недостаточно, то 
// помещаются до полного заполнения буфера  
// Возвращаемое значение - число помещенных элементов
//---------------------------------------------------------------------------
  unsigned int Insert(const T *pX, unsigned int uiSize) {
    if(uiSize > (BUF_SIZE - uiCount)) uiSize = BUF_SIZE - uiCount;
    for(unsigned int  ui = 0; ui < uiSize; ++ui){
	    pBuf[uiPtrWrite++] = pX[ui]; 
	    uiPtrWrite = uiPtrWrite % BUF_SIZE;
    }
    uiCount = uiCount + uiSize;
    return uiSize;
  }

//---------------------------------------------------------------------------
// Возвращаемое значение - число элементов в буфере
//---------------------------------------------------------------------------
  unsigned int Size(void) const {return uiCount;}

//---------------------------------------------------------------------------
// Возвращаемое значение - максимальный размер буфера (в элементах)
//---------------------------------------------------------------------------
  unsigned int SizeMax(void) const {return BUF_SIZE;}

//---------------------------------------------------------------------------
// Возвращаемое значение - размер свободного места в буфере (в элементах)
//---------------------------------------------------------------------------
  unsigned int SizeFree(void) const {return BUF_SIZE - uiCount;}

//---------------------------------------------------------------------------
// Копирует без удаления uiSize элементов из буфера начиная с позиции uiShift и
// помещает их в массив pX.
// Возвращаемое значение - число прочитанных элементов
//---------------------------------------------------------------------------
  unsigned int Read(T *pX, unsigned int uiShift, unsigned int uiSize) const {
    if(uiShift >= uiCount) return 0;
    if(uiShift + uiSize > uiCount) uiSize = uiCount - uiShift;
    unsigned int uiPtr = uiPtrRead + uiShift;
    for(unsigned int  ui = 0; ui < uiSize; ++ui){
      uiPtr = uiPtr % BUF_SIZE;
      pX[ui] = pBuf[uiPtr++]; 
    }
    return uiSize;
  }
};
//---------------------------------------------------------------------------


const unsigned char BUFTCP_OK     = 0;
const unsigned char BUFTCP_NODATA = 1;
const unsigned char BUFTCP_NOCMP  = 2;

class C_TCPBuf : public C_CyclBuf<unsigned char, 256>
{
public:
  C_TCPBuf(void) {}
  int CmpBufBeg(const unsigned char *pX, unsigned int uiSize) const;
};
///---------------------------------------------------------------------------

#endif //#ifndef TCPBUF_H		   
