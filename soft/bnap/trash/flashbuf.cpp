//============================================================================
// ����� ��� ���������� �������� ������ � ���                                    
//============================================================================

#include "flashbuf.h"
#include "rtc1338.h"
#include "SysConfig.h"
#include "mmc_spi.h"
#include "parameters.h"
//----------------------------------------------------------------------------

extern bool boMMC_REDY;
extern C_Parameters*  Parameters;
//----------------------------------------------------------------------------

//-------------------  �����  C_FlashBufPointer ------------------------------
///---------------------------------------------------------------------------
/// ������ ���� ���������� �� ��� �� DS1338
///---------------------------------------------------------------------------
U32 C_FlashBufPointer::GetPointer(void){
  // ����� � �������� 1-� ����� ����������
  U32 uiError = pRTC1338->Read(ucBaseAddress, reinterpret_cast<U8*>(&stPointer), sizeof(S_FlashBufPointer));
  if(NO_ERROR != uiError) return uiError;

  // �������� � ��������� ����������� �����. ����� �������������� �������� CRC8 ������ = size + 1
  U8 ucCRC8 = CRC8(reinterpret_cast<const U8*>(&stPointer), sizeof(S_FlashBufPointer) - 2);

  // ������ ���������� �����. ����� � �������� 2-� ����� ����������
  if(stPointer.ucCRC8 != ucCRC8) {   
    uiError = pRTC1338->Read(ucBaseAddress + sizeof(S_FlashBufPointer), reinterpret_cast<U8*>(&stPointer), sizeof(S_FlashBufPointer));
    if(NO_ERROR != uiError) return uiError;

    // �������� � ��������� ����������� �����
    ucCRC8 = CRC8(reinterpret_cast<const U8*>(&stPointer), sizeof(S_FlashBufPointer) - 2);
    if(stPointer.ucCRC8 != ucCRC8) {  // ������ ���������� �����. �������� ��������� �������������� ������
      //  ???????????????????????
      InitPointer();
    }
  }             
  // 
  return NO_ERROR;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
// ���������� ���� ���������� � ��� �� DS1338
//---------------------------------------------------------------------------
U32 C_FlashBufPointer::SavePointer(void){
  // �������� ����������� �����. � ��������� ����� ����������� ����������� �������� CRC8
  stPointer.ucCRC8 = CRC8(reinterpret_cast<const U8  *>(&stPointer), sizeof(S_FlashBufPointer) - 2);

  // �������� ���������
  if(NO_ERROR != pRTC1338->Write(ucBaseAddress, reinterpret_cast<const U8  *>(&stPointer), sizeof(S_FlashBufPointer)))
    return ERROR_FBPOINTER_SAVEPOINTER; // ������. ��� ������� � DS1338
  if(NO_ERROR != pRTC1338->Write(ucBaseAddress + sizeof(S_FlashBufPointer), reinterpret_cast<const U8  *>(&stPointer), sizeof(S_FlashBufPointer)))
    return ERROR_FBPOINTER_SAVEPOINTER; // ������. ��� ������� � DS1338
  return NO_ERROR;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

///---------------------------------------------------------------------------
/// ������������� ��������� � 0
///---------------------------------------------------------------------------
U32 C_FlashBufPointer::InitPointer(void){
  U8 puc[2*sizeof(S_FlashBufPointer)];
  memset(puc, 0 , 2*sizeof(S_FlashBufPointer));
  // � ������ RTC �������� ��� ����� ��������. ������������� ���
  U32 uiError = RTC1338.Write(ucBaseAddress, puc, 2*sizeof(S_FlashBufPointer));
  return uiError;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//-------------------  �����  C_MMCBuffer ------------------------------------
//---------------------------------------------------------------------------
// ���������� � ����� ������� � ID = uiItem 
//---------------------------------------------------------------------------
U32 C_MMCBuffer::Write(const void * pT, U16 usItem){
  U16 usSector = usItem / ITEMS_ON_SECTOR;
  U16 usShift  = usItem % ITEMS_ON_SECTOR;

  if(usSector != uiValidSector){ // ������ � ������ �� �������. 
    /// ����� ������
    uiValidSector  = 0xFFFFFFFF;  // ������� ������ � ������ ��� ����������
    U32 uiError = MMC_ReadBlock(pBuffer, (uiBegin + usSector)*MMC_BLOCKSIZE);
    if(NO_ERROR != uiError) return uiError;
  }
  // ������� � ��� ������� usShift
  uiValidSector  = 0xFFFFFFFF;  // ������� ������ � ������ ��� ����������
  memcpy(pBuffer + usShift*ITEM_SIZE, pT, ITEM_SIZE);

  // ��������� ������ �������
  U32 uiError = MMC_WriteBlock(pBuffer, (uiBegin + usSector)*MMC_BLOCKSIZE);
  if(NO_ERROR != uiError) return uiError;
  uiValidSector  = usSector;    // ������� ������ � ������ ��� ��������
  return NO_ERROR;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
// ������ �� ������ ������� � ID = uiItem � �������� ��� � ���������� pT
//---------------------------------------------------------------------------
U32 C_MMCBuffer::Read(void * pT, U16 usItem){
  U16 usSector = usItem / ITEMS_ON_SECTOR;
  U16 usShift  = usItem % ITEMS_ON_SECTOR;

  // ������ � ������ �� �������.
  if(usSector != uiValidSector){  
    // ����� ������
    uiValidSector  = 0xFFFFFFFF;  // ������� ������ � ������ ��� ����������
    U32 uiError = MMC_ReadBlock(pBuffer, (uiBegin + usSector)*MMC_BLOCKSIZE);
    if(NO_ERROR != uiError) return uiError;
    uiValidSector  = usSector;    // ������� ������ � ������ ��� ��������
  }
  // ������� ������� uiItem
  memcpy(pT, pBuffer + usShift*ITEM_SIZE, ITEM_SIZE);
  return NO_ERROR;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
// ��������� ������� � �����  
//---------------------------------------------------------------------------
U32 C_MMCBuffer::Insert(S_WayPointPacket * pWayPointPacket){

  U32 uiError = GetPointer(); 
  if(NO_ERROR != uiError) return uiError;
  uiError = Read(reinterpret_cast<S_WayPointPacket *>(&Packet), stPointer.usPtrWrite);  
  if(NO_ERROR != uiError) return uiError;
  U16 usWayPoint = Packet.GetNumberPoint(); 
  pWayPointPacket->SetNumberPoint(stPointer.usPtrWrite); 

  // �������� ������� � ������
  uiError = Write(reinterpret_cast<const void* >(pWayPointPacket), stPointer.usPtrWrite); 
  if(NO_ERROR != uiError) return uiError;

  // ����������� ��������� ������
  stPointer.usPtrWrite = ++stPointer.usPtrWrite % BUF_SIZE;
  // ����������� ����� ���������
  if(ID_FREE == usWayPoint)
    if(BUF_SIZE > stPointer.usSize) // ��� �������� ����� ������ � ������ ������ usSize
      ++stPointer.usSize;  

  // �������� ���������
  uiError = SavePointer(); if(NO_ERROR != uiError) return uiError;
  return NO_ERROR;  
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


//---------------------------------------------------------------------------
// ���������� ID ��������  �������������� �� ����������  
// usShift �� ��������� ������
//---------------------------------------------------------------------------
U32 C_MMCBuffer::GetNumberPoint(U16 & rNumberPoint, U16 usShift){
  U32 uiError = GetPointer();
  if(NO_ERROR != uiError) return uiError;

  // �������� ��������� ��� ������
  usShift = (usShift + stPointer.usPtrRead) % BUF_SIZE;
  uiError = Read(reinterpret_cast<void*>(&Packet), usShift);  
  if(NO_ERROR != uiError) return uiError;
  rNumberPoint = Packet.GetNumberPoint(); 
  return NO_ERROR;  
}     
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
// ���������� ID �������� �� ������� ��������� ��������� ������. ���� �������
// ��������� (ID=ID_FREE), �� ���������� ��������� ������ �� ��������� �������
//---------------------------------------------------------------------------
U32 C_MMCBuffer::GarbageCollection(U16 & rNumberPoint){
  U32 uiError = GetPointer(); 
  if(NO_ERROR != uiError) return uiError;
  uiError = Read(reinterpret_cast<void*>(&Packet), stPointer.usPtrRead);  
  if(NO_ERROR != uiError) return uiError;
  rNumberPoint = Packet.GetNumberPoint(); 

  if(ID_FREE == rNumberPoint){
    stPointer.usPtrRead = (stPointer.usPtrRead + 1) % BUF_SIZE;
    uiError = SavePointer(); 
    if(NO_ERROR != uiError) return uiError;
  }
  return NO_ERROR;  
}       
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                    
//---------------------------------------------------------------------------
// ������� �� ������ ������� �� ������� uiItem 
//---------------------------------------------------------------------------
U32 C_MMCBuffer::Delete(U16 uiItem){
  U32 uiError = GetPointer(); 
  if(NO_ERROR != uiError) return uiError;

  uiError = Read(reinterpret_cast<void *>(&Packet), uiItem);  
  if(NO_ERROR != uiError) return uiError;

  if(ID_FREE == Packet.GetNumberPoint()) return NO_ERROR; // ��������� �������
  Packet.SetNumberPoint(ID_FREE);    // ������� ��� ��������� �������

  // �������� ������� � ������
  uiError = Write(reinterpret_cast<void *>(&Packet), uiItem); 
  if(NO_ERROR != uiError) return uiError;

  // ����������� � �������� ���������
  if(0 != stPointer.usSize) --stPointer.usSize; // �� ������ ������ 
  if(uiItem == stPointer.usPtrRead) stPointer.usPtrRead = ++stPointer.usPtrRead % BUF_SIZE;
  uiError = SavePointer(); 
  if(NO_ERROR != uiError) return uiError;
  return NO_ERROR;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//----------------------------------------------------------------------------
// ��������� ������������� ������. 
// ����������� ���� ��� ��� ��������� ��� ����� � �������.
// �������� ��� �������� ������ ��� ��������� � ������������� ��������� � 0
//----------------------------------------------------------------------------
U32 C_MMCBuffer::Init(void){
  // ������������ ��������� � 0 (��������� 0 �� ������� ������ RTC ���������� ��� ���������)
  U32 uiError = InitPointer(); if(NO_ERROR != uiError) return uiError;
  // ������� �������� ������ ��� ���������
  S_WayPointPacket stWayPointPacket;
  stWayPointPacket.SetNumberPoint(ID_FREE); // ������� ������� ��� ���������
  U16 ui;
  for(ui = 0; ui < BUF_SIZE; ++ui){
    uiError = Write(&stWayPointPacket, ui);
    if(NO_ERROR != uiError) break;
 }
 return uiError;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//-------------------  �����  C_PacketList -----------------------------------
//----------------------------------------------------------------------------
// ��������� ������
//----------------------------------------------------------------------------
U32 C_PacketList::GetNumber(U16 & rNumberPoint){

  const U32 TIME_OUT = 50;

  U32 uiError;
  if(0 == ucNumber){
    uiError = pMMCBuffer->GetPointer(); 
    if(NO_ERROR != uiError) return uiError;  
    
    if(0 == pMMCBuffer->GetSize()) return  WARNING_FULL_OR_EMPTY;
     
    U32 uiTimeBegin = DiffTime(0);

    for(;;) {
      U32 uiTime = DiffTime(uiTimeBegin); 
      if(TIME_OUT < uiTime) return WARNING_FULL_OR_EMPTY;

      uiError = pMMCBuffer->GarbageCollection(rNumberPoint); 
      if(NO_ERROR != uiError) return uiError; 
       
      if(ID_FREE != rNumberPoint) break;    
    } 
 
    if(!IsItemFromQueue(rNumberPoint)) pList[ucNumber++] = rNumberPoint;

    for(U16 usShift = 1; usShift < BUF_SIZE; ++usShift){
      U32 uiTime = DiffTime(uiTimeBegin); 
      if(TIME_OUT < uiTime) break;

      uiError = pMMCBuffer->GetNumberPoint(rNumberPoint, usShift); 
      if(NO_ERROR != uiError) return uiError;  

      if(ID_FREE == rNumberPoint) continue;    
      if(IsItemFromQueue(rNumberPoint)) continue;
      pList[ucNumber++] = rNumberPoint;
      if(LIST_SIZE_MAX == ucNumber) break;
    } 
  }

  if(0 == ucNumber) return WARNING_FULL_OR_EMPTY;

  // ������� ������� �� ������
  for(int i = 0; i < LIST_SIZE_MAX; ++i){
    rNumberPoint = pList[i];
    if(ID_FREE == rNumberPoint) continue;
    pList[i] = ID_FREE;         
    --ucNumber;
    break;
  }
  return NO_ERROR;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//-------------------  ����� C_PacketQueue -----------------------------------

C_PacketQueue::C_PacketQueue(C_MMCBuffer* pMMCBuffer_, C_DC* pDC_) : C_PacketList(pMMCBuffer_), pDC(pDC_)
{
  InitQueue();
  // ������������ ������� ������� �������� ( �� �������� �� ������ ������ )
  WayPointPacket.SetNumberPoint(ID_FREE);
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// ������� ������� �������
//----------------------------------------------------------------------------
void C_PacketQueue::InitQueue(void){
  for(int i = 0; i < SIZE_PACKETQUEUE; ++i)
    pQueue[i].usNumberPoint = ID_FREE; 
  ucSizeQueue = 0;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//----------------------------------------------------------------------------
// ���������, ��������� ������� � ������� ��� ���
// ������������ ��������: true ���� ������� ��������� � �������
//----------------------------------------------------------------------------
bool C_PacketQueue::IsItemFromQueue(U16 uiItem){
  for(int i = 0; i < SIZE_PACKETQUEUE; ++i)
    if(uiItem == pQueue[i].usNumberPoint) return true;
  return false;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
// ����� �������� �������� ������� �������� �����
//---------------------------------------------------------------------------
void C_PacketQueue::GetNewWayPacket(void){
  static U32 uiWPTimeout = DiffTime(0);
  U32 uiTime = DiffTime(uiWPTimeout); 
  // GetWPTimeout() ���������� ����� � ���, DiffTime() � ����
  if(Parameters->GetWPTimeout() * 1000 <= uiTime) {
    // �������� ������� �����
    uiWPTimeout = DiffTime(0); 
    StateHandler->BuildWayPointPacket(WayPointPacket, ID_NOBUF);
  }
  else WayPointPacket.usNumberPoint = ID_FREE;
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// ���� ���� ���, �� ��������� ����� � ������
//---------------------------------------------------------------------------
U32 C_PacketQueue::InsertToArchiv(void){

  if(ID_FREE == WayPointPacket.usNumberPoint) return NO_ERROR;
  if(!boMMC_REDY) return NO_ERROR;

  WayPointPacket.SetPacketFromArchiv();
  U32 uiError = pMMCBuffer->Insert(&WayPointPacket);
  if(NO_ERROR != uiError){
       // !!! ��������� ������
      return uiError;
  }
  return NO_ERROR;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//----------------------------------------------------------------------------
// �������� ����� � �������.
//----------------------------------------------------------------------------
U32 C_PacketQueue::InsertToQueue(U8 ucTTL){ 

  U16 usID = WayPointPacket.usNumberPoint;
  if(ID_FREE == usID) return NO_ERROR;
  if(ID_NOBUF == usID) return NO_ERROR;
   // !!! �� �������� ��� ������ �� NO_ERROR
  if(SIZE_PACKETQUEUE - 1 <= ucSizeQueue) return WARNING_FULL_OR_EMPTY;

  // ������ ��������� ������� �������
  U16 usItemFree;
  for(usItemFree = 0; usItemFree < SIZE_PACKETQUEUE; ++usItemFree){
    if(ID_FREE == pQueue[usItemFree].usNumberPoint) break;  
  }
  // ������ � ��������� �������
  if(SIZE_PACKETQUEUE == usItemFree){
    ucSizeQueue = SIZE_PACKETQUEUE;
    return WARNING_FULL_OR_EMPTY;  // ������� ���������
  } 
   
  // ������� ������� � ������� 
  pQueue[usItemFree].usNumberPoint = usID;
  pQueue[usItemFree].ucTTL = ucTTL;       
  pQueue[usItemFree].ucRepeat = REPEAT_MAX;  
  ++ucSizeQueue;
  WayPointPacket.usNumberPoint = ID_FREE;
  return NO_ERROR;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//----------------------------------------------------------------------------
// �������� ����� � �������.
//----------------------------------------------------------------------------
U32 C_PacketQueue::InsertToQueueFromArchiv(void){

  U32 uiError;

  uiError = pMMCBuffer->GetPointer(); 
  if(NO_ERROR != uiError) return uiError;

  // !!! �� �������� ��� ������ �� NO_ERROR
  if(0 == pMMCBuffer->GetSize()) return WARNING_FULL_OR_EMPTY;
  // !!! �� �������� ��� ������ �� NO_ERROR
  // �������� ����� � SIZE_PACKETQUEUE_NEW ��� ����� ������� 
  if((SIZE_PACKETQUEUE - SIZE_PACKETQUEUE_NEW) <= ucSizeQueue) return WARNING_FULL_OR_EMPTY;

  U16 usNumberPoint;
  uiError = GetNumber(usNumberPoint);
  if(NO_ERROR != uiError) return uiError;

  WayPointPacket.usNumberPoint = usNumberPoint;
  InsertToQueue(TTL_MAX);

  return NO_ERROR; 
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// ������������ ������� �������. 
//---------------------------------------------------------------------------
U32 C_PacketQueue::Queue_Handler(void){
  U32 uiError = NO_ERROR;
  // ��������� ����� � ��������������� ID_NOBUF
  if(ID_NOBUF == WayPointPacket.GetNumberPoint())
    uiError = pDC->SendPacketToDC(reinterpret_cast<const char*>(&WayPointPacket), sizeof(WayPointPacket));
  if(0 == ucSizeQueue) goto END;

  // ����������� �������
  for(int i = 0; i < SIZE_PACKETQUEUE; ++i){
    if(ID_FREE == pQueue[i].usNumberPoint) continue;  // ��������� �������
    
    // ���������� ����� �����
    --pQueue[i].ucTTL;  
        
    // ���������� ����� ��������� ��������      
    if(0 == pQueue[i].ucTTL){         
      U16 usID = pQueue[i].usNumberPoint;

      // �������� ��������� �����
      if(0 != pQueue[i].ucRepeat){
        uiError = pMMCBuffer->Read(reinterpret_cast<void*>(&WayPointPacket), usID); 
        if(NO_ERROR != uiError) goto END;
  
        if(ID_FREE == WayPointPacket.GetNumberPoint()) {
          pQueue[i].usNumberPoint = ID_FREE;
          if(0 != ucSizeQueue) --ucSizeQueue;
        }
        uiError = pDC->SendPacketToDC(reinterpret_cast<const char*>(&WayPointPacket), sizeof(WayPointPacket));
        if(NO_ERROR != uiError) goto END;
        // ������������ ����� �����  
        pQueue[i].ucTTL = TTL_MAX; 
        --pQueue[i].ucRepeat;  
      }

      // ������ ����� �� ������  � �������
      else { 
        pQueue[i].usNumberPoint = ID_FREE;
        if(0 != ucSizeQueue) --ucSizeQueue;
        uiError = pMMCBuffer->Delete(usID);
        if(NO_ERROR != uiError) goto END;
      }
    }
  }

END:
  // ������������ ������� ������� �������� 
  WayPointPacket.SetNumberPoint(ID_FREE);
  return uiError;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//----------------------------------------------------------------------------
// ������������ ������������� ������ ������������� ������� WayPointPacket
//----------------------------------------------------------------------------
U32 C_PacketQueue::ResponseHandler(U16 usIDFromResponse){
  if(0 == ucSizeQueue) return NO_ERROR;
  for(int i = 0; i < SIZE_PACKETQUEUE; ++i){
    if(ID_FREE == pQueue[i].usNumberPoint) continue; 
    if(usIDFromResponse != pQueue[i].usNumberPoint) continue; 
    /// ������ ����� �� �������
    pQueue[i].usNumberPoint = ID_FREE;
    pQueue[i].ucTTL = 0; 
    pQueue[i].ucRepeat = 0; 
    --ucSizeQueue;
    // ������ ����� �� ������
    U32 uiError = pMMCBuffer->Delete(usIDFromResponse);
    if(NO_ERROR != uiError) return uiError;
    break;
  }
  return NO_ERROR;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
