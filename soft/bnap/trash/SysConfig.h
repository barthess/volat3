#ifndef SYSCONFIG_H
#define SYSCONFIG_H
//=============================================================================
//                                        
//=============================================================================

// <<< Use Configuration Wizard in Context Menu >>>                          

#include <RTL.h>
#include "board.h"
//-----------------------------------------------------------------------------

OS_RESULT m_os_mbx_send (OS_ID mailbox, void* message_ptr, U16   timeout );
bool m_os_mbx_send (OS_ID mailbox, void* message_ptr, U16   timeout, U16 ID_Unit);
//extern const Pin pinStrob;  
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// ������������� ������ � rts1338
//-----------------------------------------------------------------------------
const U8 RTC1338_FLASHPOINTER = 8;  // ������ 0 - 7 ������������ �� RTC
//-----------------------------------------------------------------------------

//   <o>  �������� �������� � �����, ����� ��� ����� � ���� ��� ���������  <1 - 10>
#define ALLOX_BOX_DLY 2    
//   <o>  ������������ ����� ������� �������� ������ ��� ���������    <1 - 10>  
#define ALLOX_BOX_ERROR_MAX 10  
//   <o>  �������� ���������� ��������� ����� � �����  <10 - 50>
#define SYSTIME_PERIOD 20

// ����� (� �����) �������� ����� � �������� ����� 
const U16 TIMEOUT_MAILBUX_REDY = 20;
// ����� (� �����) �������� �������������
const U16 TIMEOUT_RESPONSE_REDY = 400;

//   <o>  ������������ ����� �������� ������ �� GPS ��������� � ����� <50 - 200>
#define  TIMEOUT_GPSRX                    150

// <h> ������������ ����� �������� ������������� ������� (� �����) 
// <i> ����� �������� ��������� � ������� �������� 1-�� ���� �������, �.�. �������� 
// <i> �� ������ ����� �������� ������ �� �������, �� � ����� �������� ����� �������.
//   <o0> ��� USART0    <1 - 200>
//   <o1> ��� USART1    <1 - 200>
//   <o2> ��� USART2    <1 - 200>
//   <o3> ��� UARTD     <1 - 200>
//   <o4> ��� TWI       <1 - 100>
//   <o5> ��� GSM_RESPONSE     <1 - 100>
#define  TIMEOUT_USART0                    20                    
#define  TIMEOUT_USART1                    20                    
#define  TIMEOUT_USART2                    20                    
#define  TIMEOUT_UARTD                     20                    
#define  TIMEOUT_TWI                       100
#define  TIMEOUT_GSM_RESPONSE              25
#define  TIMEOUT_RTC_RESPONSE              40
// </h>

// <h> ������������ ������ ������ � ���������� ��� ������������� ���������
// <i> ���������� ������ ������� 4
//   <o0> ������ ������ ��� ������ ������ �� �� (� ������)         <1 - 256>
//   <o1> ������ ������ ��� ������ �� GPS ��������� (� ������)     <1 - 256>
//   <o2> ������ ������ ��� ������������� �� GSM ������            <480 - 512>
//   <i>  (������������ ������ ������ ���������� �� 2 + ����������)
#define LINKPC_INPUT_BUF_SIZE    127
#define GPS_DATA_BUF_SIZE        192 
// 2011.07.15 ������������ ������ ������ �� ������ 252 �����
#define GSM_INPUT_BUF_SIZE       256
// </h>

const U8 USART_MAILBOX_SIZE       = 5;
const U8 MANAGEMENT_MAILBOX_SIZE  = 8;
const U8 LINKPC_MAILBOX_SIZE      = 4;
const U8 LCD_MAILBOX_SIZE         = 15;
const U8 GPS_MAILBOX_SIZE         = 1;
const U8 TWI_MAILBOX_SIZE         = 4;
const U8 RTC1338_MAILBOX_SIZE     = 1;
const U8 TERMINAL_MAILBOX_SIZE    = 4;
const U8 DC_MAILBOX_SIZE          = 4;

//-----------------------------------------------------------------------------
// ��� ��������� � ������� ���� ���������� ��������� � ���������� ������ 
// �������� ������ (���������� str[]). ��� ��� ���������� ����������� 
// �� ������� 32-� ���������� �����, �� ����� ��������� � ������� str 
// ���������� ������ ������� 4. 
// ������������ ��������� �������� ��� ������������� ���������. ��������� 
// ��������� ���� �� ����� ������ ������������ �������� ������.
//-----------------------------------------------------------------------------
                
// <h> ��������� ��������� ���: 
//   <o0> ADC Clock Frequency. [���.] <100 - 5000>
// <i> ADCClock = MCK / ( (PRESCAL+1) * 2 ) 
// <i> ADC_PRESCAL =((BOARD_MCK/ADCClock + 999)/1000/2 - 1)
// <i> 999 �������� ��� ���������� � �������� ������
#define ADC_PRESCAL	((BOARD_MCK/1000 + 999)/1000/2 - 1)

// <o0> ADC Startup Time � �������� ADCClock    <8 - 248:8>
// <i> Startup Time = (STARTUP+1) * 8 / ADCClock
// <i> ADC_STARTUP  = Startup Time/8 - 1 (� �������� ADCClock)
#define ADC_STARTUP	(8/8 - 1)

//   <o0> ����� ������� � �������� ADCClock  <1 - 16>
// <i> Sample & Hold Time = (SHTIM+1) / ADCClock
// <i> SHTIM = (Sample & Hold Time - 1) (� �������� ADCClock)
#define ADC_SHTIM (7 - 1)
// </h>
//-----------------------------------------------------------------------------

const U8 START_DELIMITER = '�';
const U8 END_DELIMITER =   '�';
const U8 ARG_DELIMITER = ' ';
const U8 SIMBOL_ERROR = '1';
const U8 SIMBOL_NOERROR = '0';
const char strzOK[] = {START_DELIMITER, '0', END_DELIMITER, '\r', 0};
const char strzErr[] = {START_DELIMITER, '1', END_DELIMITER, '\r', 0};
const U16 MMC_BLOCKSIZE = 512;
//-----------------------------------------------------------------------------

// ����������� ��������� ������ �� ���������
#define USART0_OUT 1
#define USART1_OUT 2
#define USART2_OUT 3
#define UARTD_OUT  4
//   <o>��������� ��� ��������� ������ �� ���������  <1=> USART0 <2=> USART1 <3=> USART2  <4=> UARTD
#define STDOUT 1
//-----------------------------------------------------------------------------

#define NOMAILBOX    NULL
const U16 DATAREDY    = 0x1;
const U32 SIZEMISSING = 0x4;

//#define CRYPTOBLOKSIZE    36
#define BLOKSIZE          64
#define PERIOD_REQUEST_DC 20
#define MODEM_TIMEOUT_COUNTMAX   500     // ������������ ����� ������� ���������� ����� � �������
#define TIMEPACKET_NOOP_MAX      12000   // ������������ ����� �������� ������ PACKET_NOOP  (2 ���.)
const U16 TASK_DC_TIMEOUT_1_MAX = 1000;  // ������������ ����� �������� ������ �� ��  � ����
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// ����� �����������. 
//-----------------------------------------------------------------------------
enum E_StateMonitoring
{
  D_GPS_TO_PC    = 0x00000001,  // ��������� ��������� ������ �� GPS ��������� � ��
  D_GSMC_TO_PC   = 0x00000002,  // ��������� ��������� ������ ���  ������ � ��
  D_GSMR_TO_PC   = 0x00000004,  // ��������� ��������� ������������� �� ������ � ��
  D_LCDR_TO_PC   = 0x00000008,  // ��������� ��������� ������������� �� ������ � ��
  D_LCDC_TO_PC   = 0x00000010,  // ��������� ��������� ������ ���  ������ � ��
};
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// �������������� ������� � ���������
//-----------------------------------------------------------------------------
enum E_ID
{
  ID_NULL       = 0x00,
  ID_INPUTSTATE = 0x02,
  ID_DC         = 0x03,
  ID_LINKPC     = 0x04,
  ID_LINKPCTX   = 0x05,
  ID_GSM        = 0x06,
  ID_GSMR       = 0x07,
  ID_GSMT       = 0x08,
  ID_MANAGEMENT = 0x09,
  ID_GPS        = 0x0A,
  ID_USART0     = 0x0B,
  ID_USART1     = 0x0C,
  ID_USART2     = 0x0D,
  ID_UARTD      = 0x0E,
  ID_TWI        = 0x0F,
  ID_RTC1338    = 0x10,
  ID_ERRORHANDLER = 0x11,
  ID_PRINT_TEXT = 0x12,
  ID_LCD        = 0x13,
  ID_TERMINAL   = 0x14,
  ID_MSGTOPC    = 0x16,
  ID_MSGTOGSM   = 0x17,
  ID_MSGTOLCD   = 0x18,
  ID_MSGTERMINAL = 0x19,
  ID_MENU       = 0x1A,
  ID_NEW        = 0x1B,
};
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// ���� ������. 
//-----------------------------------------------------------------------------
enum E_ErrorCode
{  
  NO_ERROR                       = 0x00,
  WARNING_FULL_OR_EMPTY          = 0x01,
  WARNING_END                    = 0x09,  // ��� �������������� ������ ����� ������� �����

  ERROR_MEMORY_ALLOC             = 0x0B,
  ERROR_RESPONSE_REDY            = 0x0C,
  ERROR_MAILBUX_REDY             = 0x0D,
  ERROR_TIMEOUT                  = 0x0E,
  ERROR_PACKET_NOOP_MISSING      = 0x21,
  ERROR_PACKET_BEGIN_MISSING     = 0x22,
  ERROR_MMC_MISSING_INIT_0       = 0x30,
  ERROR_MMC_MISSING_INIT_1       = 0x31,
  ERROR_MMC_MISSING_INIT_2       = 0x32,
  ERROR_MMC_MISSING_INIT_3       = 0x33,
  ERROR_MMC_MISSING_WRITEBLOCK_1 = 0x34,
  ERROR_MMC_MISSING_WRITEBLOCK_2 = 0x35,
  ERROR_MMC_MISSING_WRITEBLOCK_3 = 0x36,
  ERROR_MMC_MISSING_WRITEBLOCK_4 = 0x37,
  ERROR_MMC_MISSING_READBLOCK_1  = 0x38,
  ERROR_MMC_MISSING_READBLOCK_2  = 0x39,
  ERROR_FBPOINTER_SAVEPOINTER    = 0x3A,
  ERROR_DS1338_WRITE_1           = 0x41,
  ERROR_DS1338_WRITE_2           = 0x42,
  ERROR_DS1338_WRITE_3           = 0x43,
  ERROR_DS1338_WRITE_4           = 0x44,
  ERROR_DS1338_READ_1            = 0x45,
  ERROR_DS1338_READ_2            = 0x46,
  ERROR_DS1338_READ_3            = 0x47,
  ERROR_DS1338_READ_4            = 0x48,
  ERROR_DS1338_READ_5            = 0x49,
  ERROR_DS1338_READ_6            = 0x4A,
  ERROR_DS1338_SSCANF            = 0x4B,
  ERROR_DS1338_GETTIME           = 0x4C,
  ERROR_GETTIME                  = 0x4D,
  ERROR_PACKETQUEUE_1            = 0x50,
  ERROR_PACKETQUEUE_3            = 0x51,
  ERROR_PACKETQUEUE_4            = 0x52,
  ERROR_DC_TCP                   = 0x53,
  ERROR_DC_1                     = 0x54, 
  ERROR_DC_2                     = 0x55,
  ERROR_DC_3                     = 0x56,
  ERROR_DC_4                     = 0x57,
  ERROR_DC_5                     = 0x58,
  ERROR_DC_6                     = 0x59,
  ERROR_DC_7                     = 0x5A,
  ERROR_GSM_1                    = 0x5B,
  ERROR_GSM_2                    = 0x5C,
  ERROR_GSM_3                    = 0x5D,
  ERROR_GSM_4                    = 0x5E,
  ERROR_LCD_1                    = 0x5F,
  ERROR_LCD_2                    = 0x60,
  ERROR_LCD_3                    = 0x61,
  ERROR_LCD_4                    = 0x62,
  ERROR_LINKPC_1                 = 0x63,
  ERROR_PRINT_TEXT_1             = 0x64, 
  ERROR_PRINT_TEXT_2             = 0x65, 
  ERROR_MSGTOPC_1                = 0x66,
  ERROR_MSGTOPC_2                = 0x67,
};
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// ����� ��������� ������
//-----------------------------------------------------------------------------
enum E_ModemState
{
  STATEGSM_CMDERROR        = 0x00000001,  // 1 - ������ ��� ���������� �������  
  STATEGSM_CMDUNRECOGNIZE  = 0x00000002,  // 1 - ������� �� ����������
  RFU2,
  //STATEGSM_PINERROR        = 0x00000008,  // 1 - PIN ��� ��������
  //STATEGSM_SIMERROR        = 0x00000010,  // 1 - SIM ����� �����������
  RFU5,
  //STATEGSM_GSMMISSING      = 0x00000040,  // 1-  ��� GSM ����������
  RFU7,
  STATEGSM_GPRSMISSING     = 0x00000100,  // 1 - ��� GPRS ����������
  //STATEGSM_GPRSCONNECTEND  = 0x00000200,  // 1 - ��������� ����� ������� ����������  GPRS ����������
  STATEGSM_TCPMISSING      = 0x00000400,  // 1 - ��� TCP ����������
  //STATEGSM_TCPCONNECTEND   = 0x00000800,  // 1 - ��������� ����� ������� ����������  TCP ����������
  //STATEGSM_TCPNOTREAD      = 0x00001000,  // 1 - ����� ������ ��� ������
  //STATEGSM_TCPNOTWRITE     = 0x00002000,  // 1 - ����� ������ ��� ������
  STATEGSM_CALLIN          = 0x00004000,  // 1 - �������� ������
  STATEGSM_CALLOUT         = 0x00008000,  // 1 - ��������� ������
  STATEGSM_CALLVOISE       = 0x00010000,  // 1 - ��������� �����
  RFU17,
  STATEGSM_ARGUMENT        = 0x00040000,  // 1 - � ������������� ������������ ������ �� ��
  STATEGSM_DATAREDY        = 0x00080000,  // 1 - � ������ �����a ���� ������ �� ��
  RFU20,
  STATEGSM_FLASHDATAERROR  = 0x00200000,  // 1- ��� ����������� ��� ���������� �����x �� ����
  RFU22,
  RFU23,
  RFU24,
  RFU25,
  RFU26,
  RFU27,
  STATEGSM_BUILDCMDERROR   = 0x10000000,  // 1 - ������ ��� ������������ ������� 
  STATEGSM_HEXERROR        = 0x20000000,  // 1 - ������������ ������ � HEX ������ 
  STATEGSM_SIZEMISSING     = 0x40000000,  // 1 - � �� ������� ����� ��� HEX ������ �� ��
  STATEGSM_TIMEOUT         = 0x80000000,  // 1 - ������� ����� �������� ������ �� ������
  STATEGSM_ALL_ERROR       = STATEGSM_CMDERROR + STATEGSM_CMDUNRECOGNIZE +\
                             STATEGSM_GPRSMISSING + STATEGSM_TCPMISSING +\
                             STATEGSM_HEXERROR + STATEGSM_TIMEOUT + STATEGSM_SIZEMISSING
//  STATEGSM_ALL_ERROR       = STATEGSM_CMDERROR + STATEGSM_CMDUNRECOGNIZE +  STATEGSM_GSMMISSING +\
//                             STATEGSM_GPRSMISSING + STATEGSM_TCPMISSING + STATEGSM_TCPNOTREAD +\
//                             STATEGSM_TCPNOTWRITE + STATEGSM_HEXERROR + STATEGSM_TIMEOUT + STATEGSM_SIZEMISSING
};

// ���� ������ � �������� ������
const U16 KEY_ALARM = 0x0001;
const U16 KEY_ENTER = 0x0004;
const U16 KEY_ESC   = 0x0010;
const U16 KEY_UP    = 0x0040;
const U16 KEY_DOWN  = 0x0100;
const U16 INPUT0    = 0x0400;
const U16 INPUT1    = 0x1000;
// ���� ���������
const U8 KEY_OFF       = 0;
const U8 KEY_OFF_TO_ON = 1;
const U8 KEY_ON        = 3;
const U8 KEY_ON_TO_OFF = 2;
const U32 STATE_ALARM  = 1;

const U8 ACTIVE_NO = 0;
const U8 ACTIVE_ALARM = 1;
const U8 ACTIVE_ESC   = 2;
const U8 ACTIVE_ENTER = 3;
const U8 ACTIVE_UP    = 4;
const U8 ACTIVE_DOWN  = 5;
 
///---------------------------------------------------------------------------

#define TX_REDY   0x1 // �������� ��������� 
#define RX_REDY   0x2 // ����� �������� 
#define TWI_ERR   0x4 // ������ TWI 

#endif //#ifndef SYSCONFIG_H
