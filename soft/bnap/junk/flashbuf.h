#ifndef FLASHBUF_H
#define FLASHBUF_H
//============================================================================
// ����� ��� ���������� �������� ������ � ���                                    
//============================================================================

#include <RTL.h>
#include "SysConfig.h"
#include "rtc1338.h"
#include "packets_dc.h"
#include "dc.h"
//----------------------------------------------------------------------------

U8 CRC8(const U8 * ucBUF, U8 ucSize);
// ���������
const U16 ID_FREE = 0xFFFF;        // NumberPoint ���������� ��������
const U16 ID_NOBUF = 0xFFFE;       // ID ������ �� ����������� � �����
const U8 REPEAT_MAX = 5;           // ������������ ����� ��������� �������� ������
const U8 TTL_MAX = 20;             // ������������ ����� �������� ������������� (���)
const U8 SIZE_PACKETQUEUE = 9;     // ������������ ������ ������� ������������ �������
const U8 SIZE_PACKETQUEUE_NEW = 2; // ��������� ����� ��� ����� �������
const U8 LIST_SIZE_MAX = 9;        // ������������ ������ ������ 
//----------------------------------------------------------------------------

#pragma pack(1)
struct S_PacketQueueItem
{
  U16 usNumberPoint;
  U8  ucTTL;
  U8  ucRepeat;
};
//----------------------------------------------------------------------------

struct S_FlashBufPointer
{
  U16 usSize;      /// ����� ��������� � ������
  U16 usPtrRead;   /// ��������� �� 1-� ������������� �������
  U16 usPtrWrite;  /// ��������� �� 1-� ��������� �������
  U8  ucCRC8;      /// ����������� �����
};
//----------------------------------------------------------------------------
#pragma pack(8)

///---------------------------------------------------------------------------
/// ������� � ������ RTC1338 � ����������� ��������� ���������� S_FlashBufPointer
/// pRTC1338 - ��������� �� ������ RTC
/// ucBaseAddress_ = ����� ��������� � ������ RTC
///---------------------------------------------------------------------------
class C_FlashBufPointer
{
protected:
  C_RTC1338 *pRTC1338;  /// ��������� �� ������� ���������� DS1338
  U8 ucBaseAddress;
  S_FlashBufPointer stPointer;
public:
  C_FlashBufPointer(C_RTC1338 *pRTC1338_, U8 ucBaseAddress_) 
                       {pRTC1338 = pRTC1338_; ucBaseAddress = ucBaseAddress_; }
  U32 InitPointer(void);
  U32 SavePointer(void);
  U32 GetPointer(void);
  U32 GetSize(void) {return stPointer.usSize;}
};
//----------------------------------------------------------------------------

//---------------------------------------------------------------------------
// ��������� ���������� ������ � ���� ������ �������������� ������� �� ��� �����.
// ������� Delete(), Insert() GarbageCollection() � GetNumberPoint() �������� 
// � ����� S_WayPointPacket
// !!! ����� ����� ���������� � ������. 
//---------------------------------------------------------------------------
const U32 MMC_STARTBLOCK = 2048;
const U16 BUF_SIZE = 1000;                             // ����� ��������� � ������
const U16 ITEM_SIZE = sizeof(S_WayPointPacket);        // ������ �������� � ������
const U16 ITEMS_ON_SECTOR = MMC_BLOCKSIZE/ITEM_SIZE;   // ����� ��������� � �������

class C_MMCBuffer : public C_FlashBufPointer
{
protected:
  char * pBuffer;            // ��������� �� ������ �������� MMC_BLOCK_SIZE ������. 
                             // ������������ ��� ��������� ������/������ � ���
  U32 uiBegin;               // ���������� ����� ����� ��� ����� � �������� ���������� �����
  U32 uiValidSector;         // ����� ������� ������ ��������  ��������� � pBuffer
                             // �������� 0xFFFFFFFF - ������������� �� �������� ������
  S_WayPointPacket Packet;   // ��� ���������� �������� ��������
  U32 Write(const void * pT, U16 usItem); /// ���������� ������� � ������� usItem
public:
  C_MMCBuffer(char * pBuffer_, C_RTC1338 *pRTC1338_, U8 ucPointerAddress, U32 uiMMCBegin) :  
       C_FlashBufPointer(pRTC1338_, ucPointerAddress), uiValidSector(0xFFFFFFFF) {pBuffer = pBuffer_; uiBegin = uiMMCBegin;} 
  U32 Init(void);                    // �������������� �����
  U32 Delete(U16 usShift);           // �������� ������� ������������� �� ���������� 
                                     // usShift �� ��������� ������ ��� ��������� 
  U32 Insert(S_WayPointPacket *pT);  // ��������� ������� � �����
  U32 Read(void * pT, U16 usItem);   // ������ ������� �� ������� usItem
  U32 GarbageCollection(U16 & rNumberPoint);        
  U32 GetNumberPoint(U16 & rNumberPoint, U16 usShift); 
};
//----------------------------------------------------------------------------


///---------------------------------------------------------------------------
/// �������� �������������� ������� ��������������� ��� �������� � ��
/// � ��������� �������� ������ ������������ �������� ID_FREE
/// LIST_SIZE_MAX - ������������ ������ ������
///---------------------------------------------------------------------------
class C_PacketList
{
protected:
  U16 pList[LIST_SIZE_MAX]; /// ������ ��������������� ������� ��� �������������� ��������
  U8  ucNumber;             /// ����� ��������� � ������.
  C_MMCBuffer * pMMCBuffer; /// ��������� �� ��������� ������ ���������� � Flash �������
  U32 BuildList(void);      /// ������������� Flash ����� � ������� ������ ������� 
                            /// ��� �������� � ��
  void Init(void) { for(int i = 0; i < LIST_SIZE_MAX; ++i) pList[i] = ID_FREE;}
public:
  C_PacketList(C_MMCBuffer * pMMCBuffer_) : ucNumber(0) {pMMCBuffer = pMMCBuffer_; Init();}
  U32 GetNumber(U16 & rNumberPoint);  /// ���������� ������������� ������ �� ������.
                                       /// ������� ������ ���������� ��� ���������
  virtual bool IsItemFromQueue(U16 uiItem) = 0;
};
//----------------------------------------------------------------------------


class C_PacketQueue : public C_PacketList
{
public:
private:
  S_PacketQueueItem  pQueue[SIZE_PACKETQUEUE];  // ������� �������
  C_DC * pDC;                      // ��������� �� ������� �������������� ������
  U8   ucSizeQueue;                // ����� ������� � �������
  virtual bool IsItemFromQueue(U16 uiItem);
public:
  S_WayPointPacket WayPointPacket; // �������������� �������
  C_PacketQueue(C_MMCBuffer* pMMCBuffer_, C_DC* pDC_);  

  void GetNewWayPacket(void);
  void InitQueue(void);
  U32 InsertToArchiv(void);
  U32 InsertToQueue(U8 ucTTL);   
  U32 InsertToQueueFromArchiv(void);
  U32 Queue_Handler(void);
  U32 ResponseHandler(U16 usNumberPointFromResponse);
};
//----------------------------------------------------------------------------

#endif //#ifndef FLASHBUF_H		   

