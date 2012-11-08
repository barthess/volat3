#ifndef MMC_SPI_H
#define MMC_SPI_H
//============================================================================
// 
//============================================================================

#include <RTL.h>
//----------------------------------------------------------------------------

U16 MMC_Init(void);
U16 MMC_ReadBlock(char * pcBuf, U32 ulAddress);
U16 MMC_WriteBlock(char * pcBuf, U32 ulAddress);
//----------------------------------------------------------------------------
#endif //#ifndef MMC_SPI_H