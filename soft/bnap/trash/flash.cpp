//============================================================================
//                                        
//============================================================================

#include <string.h>
#include "SysConfig.h"
//----------------------------------------------------------------------------

bool EFCWritePage(U16 usPage);
#define AT91C_MC_CORRECT_KEY   ((unsigned int) 0x5A << 24)
extern char pBuffer[MMC_BLOCKSIZE];
//----------------------------------------------------------------------------

// С FLASH предпологается выполнение только операции "запись"
void EFCInit(void){
  U32 uiCLK  = (1611*(BOARD_MCK >> 10)) >> 20;  // Master Clock Cycles in 1.5us
  /// Set Flash Microsecond Cycle Number
  /// Set Flash Waite State to max. (Single Cycle Access at Up to 30 MHz)
  AT91C_BASE_MC->MC_FMR = ((AT91C_MC_FMCN) & (uiCLK << 16)) | AT91C_MC_FWS_3FWS;
}
//----------------------------------------------------------------------------

///----------------------------------------------------------------------------
/// pFL_Data - адрес записи
/// pData    - указатель на данные
/// usSize   - размер данных
///----------------------------------------------------------------------------
void WriteToFlash(const char * pFL_Data, const char *pData, U16 usSize){
  U16 usSazeDataSave = 0;   /// Размер данных записанных на FLASH
  /// Определяю страницу и смещение внутри страницы для данных
  U16 usPage  = ((U32)pFL_Data - AT91C_IFLASH) / AT91C_IFLASH_PAGE_SIZE;
  U16 usShift = ((U32)pFL_Data - AT91C_IFLASH) % AT91C_IFLASH_PAGE_SIZE;
                                                        
  while(0 != usSize){
    /// Копирую  из FLASH в буфер страницу в которую будет производится запись
    memcpy(pBuffer, reinterpret_cast<char *>(usPage*AT91C_IFLASH_PAGE_SIZE + AT91C_IFLASH), AT91C_IFLASH_PAGE_SIZE);
    U16 usSizePage = AT91C_IFLASH_PAGE_SIZE - usShift; // Размер данных, которые можно записать на страницу
    if(usSize <= usSizePage){
      /// Помещаю в буфер данные, которые необходимо записать
      memcpy(pBuffer + usShift, pData + usSazeDataSave, usSize);
      /// Копирую  данные в буфер EFC
      memcpy(reinterpret_cast<char *>(usPage*AT91C_IFLASH_PAGE_SIZE + AT91C_IFLASH), pBuffer, AT91C_IFLASH_PAGE_SIZE);
      /// Записываю страницу  во FLASH
      EFCWritePage(usPage);    
      usSize = 0;  /// Все данные записаны
    }
    else {
      /// Помещаю в буфер данные, которые необходимо записать (которые помещаются)
      memcpy(pBuffer + usShift, pData + usSazeDataSave, usSizePage);
      /// Копирую  данные в буфер EFC
      memcpy(reinterpret_cast<char *>(usPage*AT91C_IFLASH_PAGE_SIZE + AT91C_IFLASH), pBuffer, AT91C_IFLASH_PAGE_SIZE);
      /// Записываю страницу  во FLASH
      EFCWritePage(usPage);    
      usSazeDataSave = usSazeDataSave + usSizePage;
      usSize = usSize - usSizePage;  /// Размер данных, которые необходимо записать
      ++usPage;                      /// Страница, на которую будет производится запись
      usShift = 0;
    }
  }
}
//----------------------------------------------------------------------------

