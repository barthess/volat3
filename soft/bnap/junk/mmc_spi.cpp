//============================================================================
// 
//============================================================================

#include <RTL.h>
#include "pio/pio.h"
#include "SysConfig.h"
#include "mmc_spi.h"
//----------------------------------------------------------------------------

struct S_CMD { 
  U8 ucCod;				///       [7] 	        [6]				        [5:0]
                  /// Start bit = 0	  Transmission bit = 1		Command index
  U32 uiArgment;  /// Argument 
  U8 ucCRC7;      /// � ������ SPI ������ 0xFF
};
//----------------------------------------------------------------------------

const U8 GO_IDLE_STATE = 0;
const U8 SEND_OP_COND  = 1;
const U8 SET_BLOCKLEN  = 16;
const U8 READ_SINGLE_BLOCK = 17;
const U8 WRITE_BLOCK = 24;

const S_CMD stCMD0  = {0x40 + GO_IDLE_STATE, 0, 0x95};
const S_CMD stCMD1  = {0x40 + SEND_OP_COND, 0, 0xFF};
const S_CMD stCMD16 = {0x40 + SET_BLOCKLEN, MMC_BLOCKSIZE, 0xFF};

#define SET_MMC_SPI_CS_LO   *AT91C_PIOA_CODR = MMC_SPI_CS
#define SET_MMC_SPI_CS_HI   *AT91C_PIOA_SODR = MMC_SPI_CS
#define SET_MMC_SPI_DO_LO   *AT91C_PIOA_CODR = MMC_SPI_DO
#define SET_MMC_SPI_DO_HI   *AT91C_PIOA_SODR = MMC_SPI_DO
#define SET_MMC_SPI_SCLK_LO *AT91C_PIOA_CODR = MMC_SPI_SCLK
#define SET_MMC_SPI_SCLK_HI *AT91C_PIOA_SODR = MMC_SPI_SCLK

const U8 DATA_TOKEN = 0xFE;
const U8 REDY = 0;
const U8 IDLE_STATE = 0x01;
extern bool boMMC_REDY;       // == true, ���� ����� �������������������
//----------------------------------------------------------------------------

U8 Send8(U8 ucOutputData){
  /// ������� � �������� ������
  U8 ucMask = 0x80;
  U8 ucInputData = 0;
  while(0 != ucMask){
    /// ��������� ������ �� �������� ����
    if(0 == (ucMask & ucOutputData)) SET_MMC_SPI_DO_LO; 
    else SET_MMC_SPI_DO_HI;
    /// �������� ������ � ������� ����
    if(0 != (MMC_SPI_DI & *AT91C_PIOA_PDSR)) ucInputData = ucInputData | ucMask;
    /// �������� clock
    SET_MMC_SPI_SCLK_HI;	   
    ucMask = ucMask >> 1;
    SET_MMC_SPI_SCLK_LO;	   
  }
  return ucInputData;
}
//----------------------------------------------------------------------------

void Send32(U32 uiOutputData){
  U32 uiMask = 0x80000000;
  while(0 != uiMask){
    /// ��������� ������ �� �������� ����
    if(0 == (uiMask & uiOutputData)) SET_MMC_SPI_DO_LO; 
    else SET_MMC_SPI_DO_HI;
    /// �������� clock
    SET_MMC_SPI_SCLK_HI;	   
    uiMask = uiMask >> 1;
    SET_MMC_SPI_SCLK_LO;	   
  }
}
//----------------------------------------------------------------------------

U8 SendCMD(const S_CMD * stCMD){ 
  Send8(0xFF);
  Send8(stCMD->ucCod);
  Send32(stCMD->uiArgment);
  Send8(stCMD->ucCRC7);
  return Send8(0xFF);               
}
//----------------------------------------------------------------------------

U8 CMD_0_1_16(const S_CMD *stCMD, U8 ucResponseCod){ 
  U16 usCount = 0;

  SET_MMC_SPI_CS_LO;
  U8 ucResponse = SendCMD(stCMD);
  for(;;){
    if(ucResponseCod == ucResponse) goto END;;
    ucResponse = Send8(0xFF);
    if(8 < usCount++) break;
  }
  Send8(0xFF);
  SET_MMC_SPI_CS_HI;
  return false;

END:  
  Send8(0xFF);
  SET_MMC_SPI_CS_HI;
  return true;
}
//----------------------------------------------------------------------------

U16 MMC_Init(void){
  U16 usCount;
  U16 usError = NO_ERROR;

  // ������������ ������ ��� ���
  const Pin Pins[] = {PINS_MMC};  
  PIO_Configure(Pins, PIO_LISTSIZE(Pins)); 

  // �������� ����������� ����� ��� ���
  if(0 == (MMC_CDT & *AT91C_PIOA_PDSR)){ 
    usError = ERROR_MMC_MISSING_INIT_0;
    goto END;
  }

  // �� �������. ��� ����� ����� �� ����������� 
  // ������������� ��� ��������� �������
  for(int i = 0; i < 64; ++i)  Send8(0xFF);

  // �������� ��� �  idle state
  usCount = 0;
  for(;;){
    if(CMD_0_1_16(&stCMD0, IDLE_STATE)) break;
    if(1000 > usCount++) continue;
    usError = ERROR_MMC_MISSING_INIT_1;  // ��������� ����� �������
    goto END;
  }

  // ������������ ��� � SPI �����
  usCount = 0;
  for(;;){
    if(CMD_0_1_16(&stCMD1, REDY)) break;
    if(1000 > usCount++) continue;
    usError = ERROR_MMC_MISSING_INIT_2;  // ��������� ����� �������
    goto END;
  }

  // ������������ ������� ����� ������/������ ������ �������� � MMC_BLOCKSIZE (� ������)
  if(!CMD_0_1_16(&stCMD16, REDY)){
    usError = ERROR_MMC_MISSING_INIT_3;
    goto END;
  }
END:
  return usError;
}
//----------------------------------------------------------------------------

//---------------------------------------------------------------------------
// ����������� ������ ������
// ulAddress - ����� 1-�� ����� �����. ����������� ������ MMC_BLOCKSIZE
// ����� ������ ����� �������� 9+ ����
//----------------------------------------------------------------------------
U16 MMC_ReadBlock(char * pcBuf, U32 ulAddress){
  U16 usError = NO_ERROR;
  // �������� �������
  S_CMD stCMD  = {0x40 + READ_SINGLE_BLOCK, 0, 0xFF};
  stCMD.uiArgment = ulAddress;

  SET_MMC_SPI_CS_LO;

  // ������� �������
  U8 ucResponse = SendCMD(&stCMD);
  U16 usCount = 0;
  for(;;){
    if(REDY == (ucResponse & 0x1F)) break;
    ucResponse = Send8(0xFF); 
    if(8 > usCount++) continue;
    usError = ERROR_MMC_MISSING_READBLOCK_1;
    goto END;
  }
   
  // ������ ������ ������ ������
  usCount = 0;
  for(;;){
    if(DATA_TOKEN == ucResponse & 0x1F) break;
    ucResponse = Send8(0xFF); 
    if(1000 > usCount++) continue;
    usError = ERROR_MMC_MISSING_READBLOCK_2;
    goto END;
  }

  // �������� ������                                                 
	for(int i = 0; i < MMC_BLOCKSIZE; i++)
    pcBuf[i] = Send8(0xFF);

  /// �������� �����
  Send8(0xFF);    // � ����� ��� ���������� �����:   Nec
  Send8(0xFF);	
  	
END:
  SET_MMC_SPI_CS_HI;
   return usError;
}
//----------------------------------------------------------------------------

//---------------------------------------------------------------------------
//   ����������� ������ ������
// ulAddress - ����� 1-�� ����� �����. ����������� ������ MMC_BLOCKSIZE
// ����� ������ ����� �������� 18+ ����
//---------------------------------------------------------------------------
const U8 DATA_ACCEPTED = 0x05;
const U8 BUSY = 0;

U16 MMC_WriteBlock(char * pcBuf, U32 ulAddress){
  U16 usError = NO_ERROR;

  // �������� �������
  S_CMD stCMD  = {0x40 + WRITE_BLOCK, 0, 0xFF};
  stCMD.uiArgment = ulAddress;

  SET_MMC_SPI_CS_LO;

  // ������� �������
  U8 ucResponse = SendCMD(&stCMD);
  U16 usCount = 0;
  for(;;){
    if(REDY == (ucResponse & 0x1F)) break;
    ucResponse = Send8(0xFF); 
    if(8 > usCount++) continue;

    usError = ERROR_MMC_MISSING_WRITEBLOCK_1;
    goto END;
  }

  // ��������� ������� ������ ������ 
	Send8(0xFF); 
  Send8(DATA_TOKEN);		   					
	
  /// ������� ������ ������                                                 
	for(int i = 0; i < MMC_BLOCKSIZE; i++)  ucResponse = Send8(pcBuf[i]);

  // �������� ��������
  usCount = 0;
  for(;;){
    if(DATA_ACCEPTED == (ucResponse & 0x1F)) break;
    ucResponse = Send8(0xFF); 
    if(8 > usCount++) continue;

    usError = ERROR_MMC_MISSING_WRITEBLOCK_2;
    goto END;
  }

  // ������ ��������� BUSY
  usCount = 0;
  for(;;){
    if(BUSY  == ucResponse) break;
    ucResponse = Send8(0xFF); 
    if(8 > usCount++) continue;

    usError = ERROR_MMC_MISSING_WRITEBLOCK_3;
    goto END;
  }

  // ������ ���������� �������� BUSY
  usCount = 0;
  for(;;){
    if(BUSY  != ucResponse) break;
    ucResponse = Send8(0xFF); 
    if(10000 > usCount++) continue;

    usError = ERROR_MMC_MISSING_WRITEBLOCK_4;
    goto END;
  }

END:
  SET_MMC_SPI_CS_HI;
  return usError;
}
//----------------------------------------------------------------------------
