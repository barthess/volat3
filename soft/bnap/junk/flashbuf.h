#ifndef FLASHBUF_H
#define FLASHBUF_H
//============================================================================
// Буфер для временного хранения данных в ММС                                    
//============================================================================

#include <RTL.h>
#include "SysConfig.h"
#include "rtc1338.h"
#include "packets_dc.h"
#include "dc.h"
//----------------------------------------------------------------------------

U8 CRC8(const U8 * ucBUF, U8 ucSize);
// Константы
const U16 ID_FREE = 0xFFFF;        // NumberPoint свободного элемента
const U16 ID_NOBUF = 0xFFFE;       // ID пакета не помещенного в буфер
const U8 REPEAT_MAX = 5;           // Максимальное число повторных отправок пакета
const U8 TTL_MAX = 20;             // Максимальное время ожидания подтверждения (тик)
const U8 SIZE_PACKETQUEUE = 9;     // Максимальное размер очереди отправленных пакетов
const U8 SIZE_PACKETQUEUE_NEW = 2; // Свободное место для новых пакетов
const U8 LIST_SIZE_MAX = 9;        // Максимальное размер списка 
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
  U16 usSize;      /// Число элементов в буфере
  U16 usPtrRead;   /// Указатель на 1-й непрочитанный элемент
  U16 usPtrWrite;  /// Указатель на 1-й свободный элемент
  U8  ucCRC8;      /// Контрольная сумма
};
//----------------------------------------------------------------------------
#pragma pack(8)

///---------------------------------------------------------------------------
/// Создает в памяти RTC1338 и обслуживает структуру указателей S_FlashBufPointer
/// pRTC1338 - указатель на драйве RTC
/// ucBaseAddress_ = адрес указателя в памяти RTC
///---------------------------------------------------------------------------
class C_FlashBufPointer
{
protected:
  C_RTC1338 *pRTC1338;  /// Указатель на драйвер микросхемы DS1338
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
// Реализует сохранение данных в виде блоков фиксированного размера на ММС карте.
// Функции Delete(), Insert() GarbageCollection() и GetNumberPoint() работают 
// с типом S_WayPointPacket
// !!! Нужно класс превратить в шаблон. 
//---------------------------------------------------------------------------
const U32 MMC_STARTBLOCK = 2048;
const U16 BUF_SIZE = 1000;                             // Число элементов в буфере
const U16 ITEM_SIZE = sizeof(S_WayPointPacket);        // Размер элемента в байтах
const U16 ITEMS_ON_SECTOR = MMC_BLOCKSIZE/ITEM_SIZE;   // Число элементов в секторе

class C_MMCBuffer : public C_FlashBufPointer
{
protected:
  char * pBuffer;            // Указатель на массив размером MMC_BLOCK_SIZE байтов. 
                             // Используется при операциях запись/чтение с ММС
  U32 uiBegin;               // Физический номер блока ММС карты с которого начинается буфер
  U32 uiValidSector;         // Номер сектора данные которого  находятся в pBuffer
                             // Значение 0xFFFFFFFF - соответствует не валидным данным
  S_WayPointPacket Packet;   // Для временного хранения элемента
  U32 Write(const void * pT, U16 usItem); /// Записывает элемент в позицию usItem
public:
  C_MMCBuffer(char * pBuffer_, C_RTC1338 *pRTC1338_, U8 ucPointerAddress, U32 uiMMCBegin) :  
       C_FlashBufPointer(pRTC1338_, ucPointerAddress), uiValidSector(0xFFFFFFFF) {pBuffer = pBuffer_; uiBegin = uiMMCBegin;} 
  U32 Init(void);                    // Инициализирует буфер
  U32 Delete(U16 usShift);           // Помечает элемент расположенный на расстоянии 
                                     // usShift от указателя чтения как свободный 
  U32 Insert(S_WayPointPacket *pT);  // Добавляет элемент в буфер
  U32 Read(void * pT, U16 usItem);   // Читает элемент из позиции usItem
  U32 GarbageCollection(U16 & rNumberPoint);        
  U32 GetNumberPoint(U16 & rNumberPoint, U16 usShift); 
};
//----------------------------------------------------------------------------


///---------------------------------------------------------------------------
/// Содержит идентификаторы пакетов предназначенных для отправки в ДЦ
/// В свободные элементы списка записывается значение ID_FREE
/// LIST_SIZE_MAX - максимальный размер списка
///---------------------------------------------------------------------------
class C_PacketList
{
protected:
  U16 pList[LIST_SIZE_MAX]; /// Список идентификаторов пакетов для первоочередной отправки
  U8  ucNumber;             /// Число элементов в списке.
  C_MMCBuffer * pMMCBuffer; /// Указатель на экземпляр класса работающий с Flash буфером
  U32 BuildList(void);      /// Просматривает Flash буфер и создает список пакетов 
                            /// для отправки в ДЦ
  void Init(void) { for(int i = 0; i < LIST_SIZE_MAX; ++i) pList[i] = ID_FREE;}
public:
  C_PacketList(C_MMCBuffer * pMMCBuffer_) : ucNumber(0) {pMMCBuffer = pMMCBuffer_; Init();}
  U32 GetNumber(U16 & rNumberPoint);  /// Возвращает идентификатор пакета из списка.
                                       /// Элемент списка помечается как свободный
  virtual bool IsItemFromQueue(U16 uiItem) = 0;
};
//----------------------------------------------------------------------------


class C_PacketQueue : public C_PacketList
{
public:
private:
  S_PacketQueueItem  pQueue[SIZE_PACKETQUEUE];  // Очередь пакетов
  C_DC * pDC;                      // Указатель на драйвер диспетчерского центра
  U8   ucSizeQueue;                // Число пакетов в очереди
  virtual bool IsItemFromQueue(U16 uiItem);
public:
  S_WayPointPacket WayPointPacket; // Обрабатываемый элемент
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

