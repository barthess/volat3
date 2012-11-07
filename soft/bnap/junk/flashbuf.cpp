//============================================================================
// Буфер для временного хранения данных в ММС                                    
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

//-------------------  Класс  C_FlashBufPointer ------------------------------
///---------------------------------------------------------------------------
/// Читает блок указателей из ОЗУ МС DS1338
///---------------------------------------------------------------------------
U32 C_FlashBufPointer::GetPointer(void){
  // Читаю и проверяю 1-ю копию указателей
  U32 uiError = pRTC1338->Read(ucBaseAddress, reinterpret_cast<U8*>(&stPointer), sizeof(S_FlashBufPointer));
  if(NO_ERROR != uiError) return uiError;

  // Вычисляю и сравниваю контрольную сумму. Число обрабатываемых функцией CRC8 байтов = size + 1
  U8 ucCRC8 = CRC8(reinterpret_cast<const U8*>(&stPointer), sizeof(S_FlashBufPointer) - 2);

  // Ошибка котрольной суммы. Читаю и проверяю 2-ю копию указателей
  if(stPointer.ucCRC8 != ucCRC8) {   
    uiError = pRTC1338->Read(ucBaseAddress + sizeof(S_FlashBufPointer), reinterpret_cast<U8*>(&stPointer), sizeof(S_FlashBufPointer));
    if(NO_ERROR != uiError) return uiError;

    // Вычисляю и сравниваю контрольную сумму
    ucCRC8 = CRC8(reinterpret_cast<const U8*>(&stPointer), sizeof(S_FlashBufPointer) - 2);
    if(stPointer.ucCRC8 != ucCRC8) {  // Ошибка котрольной суммы. Запускаю процедуру восстановления буфера
      //  ???????????????????????
      InitPointer();
    }
  }             
  // 
  return NO_ERROR;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
// Записывает блок указателей в ОЗУ МС DS1338
//---------------------------------------------------------------------------
U32 C_FlashBufPointer::SavePointer(void){
  // Вычисляю контрольную сумму. В последнем байте сохраняется вычисленное значение CRC8
  stPointer.ucCRC8 = CRC8(reinterpret_cast<const U8  *>(&stPointer), sizeof(S_FlashBufPointer) - 2);

  // Сохраняю указатели
  if(NO_ERROR != pRTC1338->Write(ucBaseAddress, reinterpret_cast<const U8  *>(&stPointer), sizeof(S_FlashBufPointer)))
    return ERROR_FBPOINTER_SAVEPOINTER; // Ошибка. Нет доступа к DS1338
  if(NO_ERROR != pRTC1338->Write(ucBaseAddress + sizeof(S_FlashBufPointer), reinterpret_cast<const U8  *>(&stPointer), sizeof(S_FlashBufPointer)))
    return ERROR_FBPOINTER_SAVEPOINTER; // Ошибка. Нет доступа к DS1338
  return NO_ERROR;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

///---------------------------------------------------------------------------
/// Устанавливает указатели в 0
///---------------------------------------------------------------------------
U32 C_FlashBufPointer::InitPointer(void){
  U8 puc[2*sizeof(S_FlashBufPointer)];
  memset(puc, 0 , 2*sizeof(S_FlashBufPointer));
  // В памяти RTC хранится две копии указвтей. Инициализирую оба
  U32 uiError = RTC1338.Write(ucBaseAddress, puc, 2*sizeof(S_FlashBufPointer));
  return uiError;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//-------------------  Класс  C_MMCBuffer ------------------------------------
//---------------------------------------------------------------------------
// Записывает в архив элемент с ID = uiItem 
//---------------------------------------------------------------------------
U32 C_MMCBuffer::Write(const void * pT, U16 usItem){
  U16 usSector = usItem / ITEMS_ON_SECTOR;
  U16 usShift  = usItem % ITEMS_ON_SECTOR;

  if(usSector != uiValidSector){ // Данные в буфере не валидны. 
    /// Читаю сектор
    uiValidSector  = 0xFFFFFFFF;  // Помечаю данные в буфере как невалидные
    U32 uiError = MMC_ReadBlock(pBuffer, (uiBegin + usSector)*MMC_BLOCKSIZE);
    if(NO_ERROR != uiError) return uiError;
  }
  // Заменяю в нем элемент usShift
  uiValidSector  = 0xFFFFFFFF;  // Помечаю данные в буфере как невалидные
  memcpy(pBuffer + usShift*ITEM_SIZE, pT, ITEM_SIZE);

  // Записываю сектор обратно
  U32 uiError = MMC_WriteBlock(pBuffer, (uiBegin + usSector)*MMC_BLOCKSIZE);
  if(NO_ERROR != uiError) return uiError;
  uiValidSector  = usSector;    // Помечаю данные в буфере как валидные
  return NO_ERROR;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
// Читает из архива элемент с ID = uiItem и помещает его в переменную pT
//---------------------------------------------------------------------------
U32 C_MMCBuffer::Read(void * pT, U16 usItem){
  U16 usSector = usItem / ITEMS_ON_SECTOR;
  U16 usShift  = usItem % ITEMS_ON_SECTOR;

  // Данные в буфере не валидны.
  if(usSector != uiValidSector){  
    // Читаю сектор
    uiValidSector  = 0xFFFFFFFF;  // Помечаю данные в буфере как невалидные
    U32 uiError = MMC_ReadBlock(pBuffer, (uiBegin + usSector)*MMC_BLOCKSIZE);
    if(NO_ERROR != uiError) return uiError;
    uiValidSector  = usSector;    // Помечаю данные в буфере как валидные
  }
  // Копирую элемент uiItem
  memcpy(pT, pBuffer + usShift*ITEM_SIZE, ITEM_SIZE);
  return NO_ERROR;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
// Добавляет элемент в архив  
//---------------------------------------------------------------------------
U32 C_MMCBuffer::Insert(S_WayPointPacket * pWayPointPacket){

  U32 uiError = GetPointer(); 
  if(NO_ERROR != uiError) return uiError;
  uiError = Read(reinterpret_cast<S_WayPointPacket *>(&Packet), stPointer.usPtrWrite);  
  if(NO_ERROR != uiError) return uiError;
  U16 usWayPoint = Packet.GetNumberPoint(); 
  pWayPointPacket->SetNumberPoint(stPointer.usPtrWrite); 

  // Сохраняю элемент в буфере
  uiError = Write(reinterpret_cast<const void* >(pWayPointPacket), stPointer.usPtrWrite); 
  if(NO_ERROR != uiError) return uiError;

  // Корректирую указатель записи
  stPointer.usPtrWrite = ++stPointer.usPtrWrite % BUF_SIZE;
  // Корректирую число элементов
  if(ID_FREE == usWayPoint)
    if(BUF_SIZE > stPointer.usSize) // Эта проверка может помочь в случае ошибки usSize
      ++stPointer.usSize;  

  // Сохраняю указатели
  uiError = SavePointer(); if(NO_ERROR != uiError) return uiError;
  return NO_ERROR;  
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


//---------------------------------------------------------------------------
// Возвращает ID элемента  расположенного на расстоянии  
// usShift от указателя чтения
//---------------------------------------------------------------------------
U32 C_MMCBuffer::GetNumberPoint(U16 & rNumberPoint, U16 usShift){
  U32 uiError = GetPointer();
  if(NO_ERROR != uiError) return uiError;

  // Формирую указатель для чтения
  usShift = (usShift + stPointer.usPtrRead) % BUF_SIZE;
  uiError = Read(reinterpret_cast<void*>(&Packet), usShift);  
  if(NO_ERROR != uiError) return uiError;
  rNumberPoint = Packet.GetNumberPoint(); 
  return NO_ERROR;  
}     
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
// Возвращает ID элемента на который указывает указатель чтения. Если элемент
// свободный (ID=ID_FREE), то перемещает указатель чтения на следующий элемент
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
// Удаляет из буфера элемент из позиции uiItem 
//---------------------------------------------------------------------------
U32 C_MMCBuffer::Delete(U16 uiItem){
  U32 uiError = GetPointer(); 
  if(NO_ERROR != uiError) return uiError;

  uiError = Read(reinterpret_cast<void *>(&Packet), uiItem);  
  if(NO_ERROR != uiError) return uiError;

  if(ID_FREE == Packet.GetNumberPoint()) return NO_ERROR; // Свободный элемент
  Packet.SetNumberPoint(ID_FREE);    // Помечаю как свободный элемент

  // Сохраняю элемент в буфере
  uiError = Write(reinterpret_cast<void *>(&Packet), uiItem); 
  if(NO_ERROR != uiError) return uiError;

  // Корректирую и сохраняю указатель
  if(0 != stPointer.usSize) --stPointer.usSize; // На всякий случай 
  if(uiItem == stPointer.usPtrRead) stPointer.usPtrRead = ++stPointer.usPtrRead % BUF_SIZE;
  uiError = SavePointer(); 
  if(NO_ERROR != uiError) return uiError;
  return NO_ERROR;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//----------------------------------------------------------------------------
// Первичная инициализация буфера. 
// Выполняется один раз при установке ММС карты в систему.
// Помечает все элементы буфера как свободные и устанавливает указатели в 0
//----------------------------------------------------------------------------
U32 C_MMCBuffer::Init(void){
  // Устанавливаю указатели в 0 (Записываю 0 во область памяти RTC отведенную под указатели)
  U32 uiError = InitPointer(); if(NO_ERROR != uiError) return uiError;
  // Помечаю элементы буфера как свободные
  S_WayPointPacket stWayPointPacket;
  stWayPointPacket.SetNumberPoint(ID_FREE); // Помечаю элемент как свободный
  U16 ui;
  for(ui = 0; ui < BUF_SIZE; ++ui){
    uiError = Write(&stWayPointPacket, ui);
    if(NO_ERROR != uiError) break;
 }
 return uiError;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//-------------------  Класс  C_PacketList -----------------------------------
//----------------------------------------------------------------------------
// Заполняет список
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

  // Забираю элемент из списка
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

//-------------------  Класс C_PacketQueue -----------------------------------

C_PacketQueue::C_PacketQueue(C_MMCBuffer* pMMCBuffer_, C_DC* pDC_) : C_PacketList(pMMCBuffer_), pDC(pDC_)
{
  InitQueue();
  // Устанавливаю признак пустого элемента ( не содержит ни какого пакета )
  WayPointPacket.SetNumberPoint(ID_FREE);
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Очищает очередь пакетов
//----------------------------------------------------------------------------
void C_PacketQueue::InitQueue(void){
  for(int i = 0; i < SIZE_PACKETQUEUE; ++i)
    pQueue[i].usNumberPoint = ID_FREE; 
  ucSizeQueue = 0;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//----------------------------------------------------------------------------
// Проверяет, находится элемент в очереди или нет
// Возвращаемое значение: true если элемент находится в очереди
//----------------------------------------------------------------------------
bool C_PacketQueue::IsItemFromQueue(U16 uiItem){
  for(int i = 0; i < SIZE_PACKETQUEUE; ++i)
    if(uiItem == pQueue[i].usNumberPoint) return true;
  return false;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
// Через заданный интервал времени формрует пакет
//---------------------------------------------------------------------------
void C_PacketQueue::GetNewWayPacket(void){
  static U32 uiWPTimeout = DiffTime(0);
  U32 uiTime = DiffTime(uiWPTimeout); 
  // GetWPTimeout() возвращает время в сек, DiffTime() в мсек
  if(Parameters->GetWPTimeout() * 1000 <= uiTime) {
    // Сохраняю текущее время
    uiWPTimeout = DiffTime(0); 
    StateHandler->BuildWayPointPacket(WayPointPacket, ID_NOBUF);
  }
  else WayPointPacket.usNumberPoint = ID_FREE;
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Если есть ММС, то сохраняет пакет в буфере
//---------------------------------------------------------------------------
U32 C_PacketQueue::InsertToArchiv(void){

  if(ID_FREE == WayPointPacket.usNumberPoint) return NO_ERROR;
  if(!boMMC_REDY) return NO_ERROR;

  WayPointPacket.SetPacketFromArchiv();
  U32 uiError = pMMCBuffer->Insert(&WayPointPacket);
  if(NO_ERROR != uiError){
       // !!! Обработка ошибок
      return uiError;
  }
  return NO_ERROR;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//----------------------------------------------------------------------------
// Помещает пакет в очередь.
//----------------------------------------------------------------------------
U32 C_PacketQueue::InsertToQueue(U8 ucTTL){ 

  U16 usID = WayPointPacket.usNumberPoint;
  if(ID_FREE == usID) return NO_ERROR;
  if(ID_NOBUF == usID) return NO_ERROR;
   // !!! Не изменять тип ошибки на NO_ERROR
  if(SIZE_PACKETQUEUE - 1 <= ucSizeQueue) return WARNING_FULL_OR_EMPTY;

  // Нахожу свободный элемент очереди
  U16 usItemFree;
  for(usItemFree = 0; usItemFree < SIZE_PACKETQUEUE; ++usItemFree){
    if(ID_FREE == pQueue[usItemFree].usNumberPoint) break;  
  }
  // Ошибка в структуре очереди
  if(SIZE_PACKETQUEUE == usItemFree){
    ucSizeQueue = SIZE_PACKETQUEUE;
    return WARNING_FULL_OR_EMPTY;  // Очередь заполнена
  } 
   
  // Помещаю элемент в очередь 
  pQueue[usItemFree].usNumberPoint = usID;
  pQueue[usItemFree].ucTTL = ucTTL;       
  pQueue[usItemFree].ucRepeat = REPEAT_MAX;  
  ++ucSizeQueue;
  WayPointPacket.usNumberPoint = ID_FREE;
  return NO_ERROR;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//----------------------------------------------------------------------------
// Помещает пакет в очередь.
//----------------------------------------------------------------------------
U32 C_PacketQueue::InsertToQueueFromArchiv(void){

  U32 uiError;

  uiError = pMMCBuffer->GetPointer(); 
  if(NO_ERROR != uiError) return uiError;

  // !!! Не изменять тип ошибки на NO_ERROR
  if(0 == pMMCBuffer->GetSize()) return WARNING_FULL_OR_EMPTY;
  // !!! Не изменять тип ошибки на NO_ERROR
  // Оставляю запас в SIZE_PACKETQUEUE_NEW для новых пакетов 
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
// Обрабатывает очередь пакетов. 
//---------------------------------------------------------------------------
U32 C_PacketQueue::Queue_Handler(void){
  U32 uiError = NO_ERROR;
  // Отправляю пакет с идентификатором ID_NOBUF
  if(ID_NOBUF == WayPointPacket.GetNumberPoint())
    uiError = pDC->SendPacketToDC(reinterpret_cast<const char*>(&WayPointPacket), sizeof(WayPointPacket));
  if(0 == ucSizeQueue) goto END;

  // Обрабатываю очередь
  for(int i = 0; i < SIZE_PACKETQUEUE; ++i){
    if(ID_FREE == pQueue[i].usNumberPoint) continue;  // Свободный элемент
    
    // Коректирую время жизни
    --pQueue[i].ucTTL;  
        
    // Коректирую число повторных отправок      
    if(0 == pQueue[i].ucTTL){         
      U16 usID = pQueue[i].usNumberPoint;

      // Повторно отправляю пакет
      if(0 != pQueue[i].ucRepeat){
        uiError = pMMCBuffer->Read(reinterpret_cast<void*>(&WayPointPacket), usID); 
        if(NO_ERROR != uiError) goto END;
  
        if(ID_FREE == WayPointPacket.GetNumberPoint()) {
          pQueue[i].usNumberPoint = ID_FREE;
          if(0 != ucSizeQueue) --ucSizeQueue;
        }
        uiError = pDC->SendPacketToDC(reinterpret_cast<const char*>(&WayPointPacket), sizeof(WayPointPacket));
        if(NO_ERROR != uiError) goto END;
        // Устанавливаю время жизни  
        pQueue[i].ucTTL = TTL_MAX; 
        --pQueue[i].ucRepeat;  
      }

      // Удаляю пакет из буфера  и очереди
      else { 
        pQueue[i].usNumberPoint = ID_FREE;
        if(0 != ucSizeQueue) --ucSizeQueue;
        uiError = pMMCBuffer->Delete(usID);
        if(NO_ERROR != uiError) goto END;
      }
    }
  }

END:
  // Устанавливаю признак пустого элемента 
  WayPointPacket.SetNumberPoint(ID_FREE);
  return uiError;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//----------------------------------------------------------------------------
// Обрабатывает подтверждения приема диспетчерским центром WayPointPacket
//----------------------------------------------------------------------------
U32 C_PacketQueue::ResponseHandler(U16 usIDFromResponse){
  if(0 == ucSizeQueue) return NO_ERROR;
  for(int i = 0; i < SIZE_PACKETQUEUE; ++i){
    if(ID_FREE == pQueue[i].usNumberPoint) continue; 
    if(usIDFromResponse != pQueue[i].usNumberPoint) continue; 
    /// Удаляю пакет из очереди
    pQueue[i].usNumberPoint = ID_FREE;
    pQueue[i].ucTTL = 0; 
    pQueue[i].ucRepeat = 0; 
    --ucSizeQueue;
    // Удаляю пакет из буфера
    U32 uiError = pMMCBuffer->Delete(usIDFromResponse);
    if(NO_ERROR != uiError) return uiError;
    break;
  }
  return NO_ERROR;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
