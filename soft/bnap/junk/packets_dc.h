#ifndef PACKETS_DC_H
#define PACKETS_DC_H
//============================================================================
// ���������� ������� �������������� ������                                  
//============================================================================

#include <string.h>
//----------------------------------------------------------------------------

  #define TYPE_PACKET_TERMINAL_INFO               0x01
  #define TYPE_PACKET_WAY_POINT                   0x02
  #define TYPE_PACKET_NOOP_CONFIRM                0x03
  #define TYPE_PACKET_VERSION                     0x04    // ������ ��
  #define TYPE_PACKET_SET_POLLING_CONFIG_CONFIRM  0x05    // ������������� ���������� ������ ���������
  #define TYPE_PACKET_SET_IP_CONFIG_CONFIRM       0x06    // ������������� ���������� ������ ���������
  #define TYPE_PACKET_SET_GPRS_CONFIG_CONFIRM     0x07    // ������������� ���������� ������ ���������
  #define TYPE_PACKET_SET_OUTPUT_CONFIRM          0x08    // ������������� ��������� ��������� ������
#define TYPE_PACKET_GET_CONFIG_CONFIRM          0x09    // ������������� ������ ���������� ������ ���������
#define TYPE_PACKET_GET_FILE_BLOCK              0x0A    // ������ ����� ������ ����������
#define TYPE_PACKET_GET_FILE_SIZE               0x0B    // ������ ������� ����� ����������

// �������������� ������� �� �������
  #define TYPE_PACKET_AUTH_PASSWORD               0x81
  #define TYPE_PACKET_WAY_POINT_CONFIRM           0x82
  #define TYPE_PACKET_NOOP                        0x83
  #define TYPE_PACKET_IS_UPDATE                   0x84    // ���������� � ������� ����������
  #define TYPE_PACKET_SET_POLLING_CONFIG          0x85    // ��������� ���������� ������ ���������
  #define TYPE_PACKET_SET_IP_CONFIG               0x86    // ��������� ���������� ������ ���������
  #define TYPE_PACKET_SET_GPRS_CONFIG             0x87    // ��������� ���������� ������ ���������
#define TYPE_PACKET_SET_OUTPUT                  0x88    // ��������� ��������� ������
#define TYPE_PACKET_GET_CONFIG                  0x89    // ������ ���������� ������ ���������
#define TYPE_PACKET_FILE_BLOCK                  0x8A    // �������� ���� ������ ����������
#define TYPE_PACKET_FILE_SIZE                   0x8B    // ��������� ������� ����� ����������
//----------------------------------------------------------------------------

// #pragma pack(1)
//----------------------------------------------------------------------------

// ��� ������ ����� ����������� ��������� �������� 7 ����
__packed struct S_DefaultPacket
{
  char    pcID[4];        // ������������� ������ "KMRT"
  U16     usSize;         // ������ ������ � ����������
  U8      ucType;         // ��� ������
};
//----------------------------------------------------------------------------

// ����� ������������ ���������� �������� �������� ������� ����� ����������������� ������
const U8 TERMINALNUMBERSIZE = 15;
__packed struct S_TerminalInfoPacket
{
  char    pcID[4];        // ������������� ������ "KMRT"
  U16     usSize;         // ������ ������ � ����������
  U8      ucType;         // ��� ������ TYPE_PACKET_TERMINAL_INFO
  char    pcNumber[TERMINALNUMBERSIZE + 1];   // ����� ��������� (�������� "0001")
  S_TerminalInfoPacket(const char *strzTereminalNumber) : usSize(sizeof(S_TerminalInfoPacket)), ucType(TYPE_PACKET_TERMINAL_INFO)  
  {pcID[0] = 'K'; pcID[1] = 'M'; pcID[2] = 'R'; pcID[3] = 'T'; strncpy(pcNumber, strzTereminalNumber, sizeof(pcNumber) - 1);}
}; 
//----------------------------------------------------------------------------

// � ����� ������ �������� ��������� �������������� � ������� 
const U8 GPSPASSWORDSIZE = 6;
__packed struct S_AuthPacket
{
  char    pcID[4];          // ������������� ������ "KMRT"
  U16     usSize;           // ������ ������ � ����������
  U8      ucType;           // ��� ������ - TYPE_PACKET_AUTH_PASSWORD
  char    pcGPSPassowrd[GPSPASSWORDSIZE + 1]; // ������ (������ �� ��������� "123456"). ������ ��������������� ��� ��������� ���������
  S_AuthPacket(const char *strzGPSPassowrd) : usSize(sizeof(S_AuthPacket)), ucType(TYPE_PACKET_AUTH_PASSWORD)
  {pcID[0] = 'K'; pcID[1] = 'M'; pcID[2] = 'R'; pcID[3] = 'T';
   memset(pcGPSPassowrd, 0, sizeof(pcGPSPassowrd));  strncpy(pcGPSPassowrd, strzGPSPassowrd, sizeof(pcGPSPassowrd) - 1);}
};
// ���� ��������� �������� ������ �� �����, �� �������� ��������� ���������� � �������� ��������� ����������
//----------------------------------------------------------------------------

// ����� ��������� �������� �������������� �������� �������� ������� ����� � �������� �� � �������� SIM-����� 
// � IMEI, ���� ����� ������������ ��� ���������� ���������� ���������� �� ���������
__packed struct S_VersionPacket
{
  char    pcID[4];               // ������������� ������ "KMRT"
  U16     usSize;                // ������ ������ � ����������
  U8      ucType;                // ��� ������ - TYPE_PACKET_VERSION
  char    pcTerminalType[10];    // �������� ��������� ("����-02")
  char    pcOS_Version[40];      // ������ ������������ �������  
  char    pcWDWL_Version[10];    // ������ ����������           
  char    pcIMEI_Number[16];     // IMEI                     
  char    pcIMSI_Number[16];     // IMSI                 
  char    pcTerminalVersion[16]; // ������ �� ���������
  S_VersionPacket(void) : usSize(sizeof(S_VersionPacket)), ucType(TYPE_PACKET_VERSION){  
    pcTerminalType[sizeof(pcTerminalType) - 1] = 0;
    pcOS_Version[sizeof(pcOS_Version) - 1] = 0;
    pcWDWL_Version[sizeof(pcWDWL_Version) - 1] = 0;
    pcIMEI_Number[sizeof(pcIMEI_Number) - 1] = 0;
    pcIMSI_Number[sizeof(pcIMSI_Number) - 1] = 0;
    pcTerminalVersion[sizeof(pcTerminalVersion) - 1] = 0;
    pcID[0] = 'K'; pcID[1] = 'M'; pcID[2] = 'R'; pcID[3] = 'T';
  }
  void    SetTerminalType(const char *strz){ strncpy(pcTerminalType, strz, sizeof(pcTerminalType)) - 1;}
  void    SetOS_Version(const char *strz)  { strncpy(pcOS_Version,   strz, sizeof(pcOS_Version)) - 1;}
  void    SetWDWL_Version(const char *strz){ strncpy(pcWDWL_Version, strz, sizeof(pcWDWL_Version)) - 1;}
  void    SetIMEI_Number(const char *strz) { strncpy(pcIMEI_Number,  strz, sizeof(pcIMEI_Number)) - 1;}
  void    SetIMSI_Number(const char *strz) { strncpy(pcIMSI_Number,  strz, sizeof(pcIMSI_Number)) - 1;}
  void    SetTerminalVersion(const char *strz){ strncpy(pcTerminalVersion, strz, sizeof(pcTerminalVersion) - 1);}
};
//----------------------------------------------------------------------------

// ����� ��������� ������ � �������� �� �� ��������� ����� � ����������� � ������� ���������� ��
__packed struct S_UpdatePacket
{
  char    pcID[4];            // ������������� ������ "KMRT"
  U16     usSize;             // ������ ������ � ����������
  U8      ucType;             // ��� ������ - TYPE_PACKET_IS_UPDATE
  U8      ucIsUpdate;         // ���� �� ����������
  U8      ucTypeUpdate;       // ��� ����������
                              // 1 - ���������� ����������
                              // 2 - ���������� ��
                              // 3 - ���������� ����������
  U8      pucNewVersion[16];  // ������ ����������
  U8      ucCountFiles;       // ���������� ������ � ����������
};
//----------------------------------------------------------------------------

// � �������� ������ �������� ��������� ��������� ���������� ������ ���������, ��� ���� ������������� 
// ��������� ������ ���������� ��
__packed struct S_ConfigPollingPacket
{
  char    pcID[4];          // ������������� ������ "KMRT"
  U16     usSize;           // ������ ������ � ����������
  U8      ucType;           // ��� ������ - TYPE_PACKET_SET_POLLING_CONFIG
  U16     usGPRSDataTimeout;// �������� ������ ������������� ������
  U16     usGPRSDistanse;   // ���������� ������ ������������� ������
  U8      ucGPRSAzimut;     // ������ �������� ������������� ������
  S_ConfigPollingPacket(U16 usTimeout, U16 usDistanse, U8 ucAzimut) : usSize(sizeof(S_ConfigPollingPacket)), ucType(TYPE_PACKET_SET_POLLING_CONFIG)
                          {pcID[0] = 'K'; pcID[1] = 'M'; pcID[2] = 'R'; pcID[3] = 'T';
                          usGPRSDataTimeout = usTimeout; usGPRSDistanse = usDistanse; ucGPRSAzimut = ucAzimut;}
};
//----------------------------------------------------------------------------

__packed struct S_ConfigIPPacket
{
  char    pcID[4];        // ������������� ������ "KMRT"
  U16     usSize;         // ������ ������ � ����������
  U8      ucType;         // ��� ������ - TYPE_PACKET_SET_IP_CONFIG
  char    pcTcpServ[21];  // IP-����� �������
  U16     usTcpPort;      // TCP/IP ����
};
//----------------------------------------------------------------------------

__packed struct S_ConfigGPRSPacket
{
  char    pcID[4];        // ������������� ������ "KMRT"
  U16     usSize;         // ������ ������ � ����������
  U8      ucType;         // ��� ������ - TYPE_PACKET_SET_GPRS_CONFIG
  char    pcApnServ[97];  // ����� ����� � GPRS
  char    pcApnUn[65];    // ��� ������������
  char    pcApnPw[65];    // ������ ������������
};
//----------------------------------------------------------------------------

// � �������� ������ �������� ��������� ������ ������������� ������ � �������� �� �������
//__packed struct S_WayPointPacket 
//{
//  char    pcID[4];        // ������������� ������ "KMRT"
//  U16     usSize;         // ������ ������ � ����������
//  U8      ucType;         // ��� ������ - TYPE_PACKET_WAY_POINT
//  U8      ucTypePoint;    // �� ������������
//  U32     uiData;         // ���� ddmmyy (�������� 250109 - 25/01/2009)
//  U32     uiTime;         // ����� hhmmsssss (�������� 234859000 - 23:48:59.000)
//  float   fLat;           // ������ ddmm.mmmm(�������� 5352.2134 - 53?53.2134')
//  float   fLon;           // ������� ddmm.mmmm(�������� 2752.2134 - 27?53.2134')
//  U16     usAlt;          // ������ � ������
//  U16     usCourse;       // ���� � ��������
//  U16     usSpeed;        // �������� � ���������� � ���
//  U16     usNumberPoint;  // ����� ������� ����� (������������ � ������ �������������) 
//  U32     uiDistanse;     // �� ������������
//  float   fPower;         // �������� ������� � �������
//  U32     uiStatus;       // ������� ��� - ��������� ��������� ������ (0x01)
//  S_WayPointPacket(void) : usSize(sizeof(S_WayPointPacket)), ucType(TYPE_PACKET_WAY_POINT)
//                                    {pcID[0] = 'K'; pcID[1] = 'M'; pcID[2] = 'R'; pcID[3] = 'T'; }
//  U16  GetNumberPoint(void){return usNumberPoint;} 
//  void SetNumberPoint(U16 usX){usNumberPoint = usX;} 
//};
//----------------------------------------------------------------------------

// � ����� �� ����� ������������� ������ ������ ���������� ����� �������������
__packed struct S_WayPointConfirmPacket
{
  char    pcID[4];        // ������������� ������ "KMRT"
  U16     usSize;         // ������ ������ � ����������
  U8      ucType;         // ��� ������ - TYPE_PACKET_WAY_POINT_CONFIRM
  U16     usNumberPoint;  // ����� ������� ����� (��. TWayPointPacket)
  S_WayPointConfirmPacket(U16 usNumber) : usSize(sizeof(S_WayPointConfirmPacket)), ucType(TYPE_PACKET_WAY_POINT_CONFIRM)
                          {pcID[0] = 'K'; pcID[1] = 'M'; pcID[2] = 'R'; pcID[3] = 'T'; usNumberPoint = usNumber;}
};
//----------------------------------------------------------------------------

// ���� �������� �� ������� ����� �������������, �� ��� ��������� ���������� ����� ����� �������� �����
// � �������� ������ ��� �������� �������� ������ 1 ��� � ������ ���������� ����� ��������
__packed struct S_NoopPacket
{
  char    pcID[4];        // ������������� ������ "KMRT"
  U16     usSize;         // ������ ������ � ����������
  U8      ucType;         // ��� ������ - TYPE_PACKET_NOOP
};
//----------------------------------------------------------------------------

// � ����� �������� ���������� ���������� �����  ���� S_NoopConfirmPacket

// ���� �������� � ������� 2-� ����� �� ������� ����� TNoopPacket �������� ��������� ���������� 
// � ��������� ��������� ����������.

// ���� ������ � ������� 2-����� ����� �������� TNoopPacket ��������� TNoopConfirmPacket, 
// ������ ��������� ��������.
//----------------------------------------------------------------------------

// ������ �������������
__packed struct S_NoopConfirmPacket
{
  char    pcID[4];        // ������������� ������ "KMRT"
  U16     usSize;         // ������ ������ � ����������
  U8      ucType;         // ��� ������ - TYPE_PACKET_NOOP_CONFIRM
  S_NoopConfirmPacket(void) : usSize(sizeof(S_NoopConfirmPacket)), ucType(TYPE_PACKET_NOOP_CONFIRM)
                                    {pcID[0] = 'K'; pcID[1] = 'M'; pcID[2] = 'R'; pcID[3] = 'T'; }
};
//----------------------------------------------------------------------------

__packed struct S_SetPollingConfirmPacket
{
  char    pcID[4];        // ������������� ������ "KMRT"
  U16     usSize;         // ������ ������ � ����������
  U8      ucType;         // ��� ������ - TYPE_PACKET_SET_POLLING_CONFIG_CONFIRM
  S_SetPollingConfirmPacket(void) : usSize(sizeof(S_SetPollingConfirmPacket)), ucType(TYPE_PACKET_SET_POLLING_CONFIG_CONFIRM)
                                    {pcID[0] = 'K'; pcID[1] = 'M'; pcID[2] = 'R'; pcID[3] = 'T'; }
};
//----------------------------------------------------------------------------

__packed struct S_SetIPConfirmPacket
{
  char    pcID[4];        // ������������� ������ "KMRT"
  U16     usSize;         // ������ ������ � ����������
  U8      ucType;         // ��� ������ - TYPE_PACKET_SET_IP_CONFIG_CONFIRM
  S_SetIPConfirmPacket(void) : usSize(sizeof(S_SetIPConfirmPacket)), ucType(TYPE_PACKET_SET_IP_CONFIG_CONFIRM)
                               {pcID[0] = 'K'; pcID[1] = 'M'; pcID[2] = 'R'; pcID[3] = 'T'; }
};
//----------------------------------------------------------------------------

__packed struct S_SetGPRSConfirmPacket
{
  char    pcID[4];        // ������������� ������ "KMRT"
  U16     usSize;         // ������ ������ � ����������
  U8      ucType;         // ��� ������ - TYPE_PACKET_SET_GPRS_CONFIG_CONFIRM
  S_SetGPRSConfirmPacket(void) : usSize(sizeof(S_SetGPRSConfirmPacket)), ucType(TYPE_PACKET_SET_GPRS_CONFIG_CONFIRM)
                                 {pcID[0] = 'K'; pcID[1] = 'M'; pcID[2] = 'R'; pcID[3] = 'T'; }
};
//----------------------------------------------------------------------------

__packed struct S_SetOutputConfirmPacket
{
  char    pcID[4];        // ������������� ������ "KMRT"
  U16     usSize;         // ������ ������ � ����������
  U8      ucType;         // ��� ������ - TYPE_PACKET_SET_OUTPUT_CONFIRM
  S_SetOutputConfirmPacket(void) : usSize(sizeof(S_SetOutputConfirmPacket)), ucType(TYPE_PACKET_SET_OUTPUT_CONFIRM)
                                   {pcID[0] = 'K'; pcID[1] = 'M'; pcID[2] = 'R'; pcID[3] = 'T'; }
};
//----------------------------------------------------------------------------

// �������� ������ ��������� ����-05

// �������� ������ ��������� ����-05 �������� � ���� �������� ������ ����-02 
// ���� ��������� �����������.

// ������ �� ����-05 � �������
#define TYPE_PACKET_NUMBER_MESSAGE     0x10    // ��������  ������ ��������� 
#define TYPE_PACKET_MES_CMD_CONFIRM    0x30    // ������������� ������ ���������, ������ ��������� � ������� ���

// ������ �� ������� � ����-05
#define TYPE_PACKET_MESSAGE            0x90    // �������� ���������
#define TYPE_PACKET_CMD_SCRN           0x92    // �������� ��������� ������ A,B,N,Z ��� ���
//#define TYPE_PACKET_DYNAMIC_ELEMENT    0x93    // �������� ��������� �������   D ��� ���
//#define TYPE_PACKET_SET_MAIN_MENU      0x91    // �������� ��������� ������� W ��� ���
//----------------------------------------------------------------------------

// �������� �� ������ ������ ������ ���������. ����������� ����� ���� ���  
// ������������ ����� ������� ���� ��� ������ �� ���� ���������
__packed struct S_NumMessagePacket
{
  char  pcID[4];  // ������������� ������ "KMRT"
  U16   usSize;   // ������ ������
  U8    ucType;   // ��� ������: TYPE_PACKET_NUMBER_MESSAGE
  U32   uiData;   // ���� ddmmyy (�������� 250109 - 25/01/2009)
  U32   uiTime;   // ����� hhmmsssss (�������� 234859000 - 23:48:59.000)
  U32   uiNumMes; // ������ ���������
  S_NumMessagePacket(U32 uiNumberMSG) : usSize(sizeof(S_NumMessagePacket)), ucType(TYPE_PACKET_NUMBER_MESSAGE), uiData(0), uiTime(0)
      {pcID[0] = 'K'; pcID[1] = 'M'; pcID[2] = 'R'; pcID[3] = 'T'; uiNumMes = uiNumberMSG;}
};
//----------------------------------------------------------------------------

// �������� �� ����-05 ������ ��������� (������� T ��� ���)
__packed struct S_MessagePacket
{
  #define  SIZETEXT  250
   char      pcID[4];        // ������������� ������ "KMRT"
   U16       usSize;         // ������ ������
   U8        ucType;         // ��� ������: TYPE_PACKET_MESSAGE
   char      pcMessage[SIZETEXT + 1];   // ����� ���������
   S_MessagePacket(char * strz) : ucType(TYPE_PACKET_MESSAGE) {
     pcID[0] = 'K'; pcID[1] = 'M'; pcID[2] = 'R'; pcID[3] = 'T';
     int iLength = strlen(strz);
     if(SIZETEXT < iLength) iLength = SIZETEXT;
     memcpy(pcMessage, strz, iLength);
     pcMessage[iLength] = 0;
     usSize = 7 + iLength + 1;
   }
   U8 GetTextSizeMax(void) {return SIZETEXT;}
   #undef SIZETEXT
};
//----------------------------------------------------------------------------

// �������� �� ����-05 ������ ������ A,B,N,Z ��� ���
__packed struct S_CmdScrnPacket
{
  #define  SIZETEXT  30
  char     pcID[4];        // ������������� ������ "KMRT"
  U16      usSize;         // ������ ������
  U8       ucType;         // ��� ������: TYPE_PACKET_CMD_SCRN
  U8       ucCmd;          // ������� ��� ���: A,B,N,Z
  char     pcText[SIZETEXT + 1];
  S_CmdScrnPacket(U8 chCMD, char * strz) : usSize(sizeof(S_CmdScrnPacket)), ucType(TYPE_PACKET_CMD_SCRN)
      {pcID[0] = 'K'; pcID[1] = 'M'; pcID[2] = 'R'; pcID[3] = 'T'; ucCmd = chCMD; strncpy(pcText, strz, SIZETEXT);}
   U8 GetTextSizeMax(void) {return SIZETEXT;}
   #undef SIZETEXT
};
//----------------------------------------------------------------------------

//  �������� �� ����-05 ��� ������ ������ ������������� �� ���, ����������������� ������, ��� ����-05
__packed struct S_MesCmdConfirmPacket
{
   char      pcID[4];     // ������������� ������ "KMRT"
   U16       usSize;      // ������ ������
   U8        ucType;      // ��� ������: TYPE_PACKET_MES_CMD_CONFIRM
   U32       uiRetCod;    //  = 0 - ��� ������
                          // != 0 - ����� ������
  S_MesCmdConfirmPacket(void) : usSize(sizeof(S_MesCmdConfirmPacket)), ucType(TYPE_PACKET_MES_CMD_CONFIRM),
                                uiRetCod(0) {pcID[0] = 'K'; pcID[1] = 'M'; pcID[2] = 'R'; pcID[3] = 'T'; }
};
//----------------------------------------------------------------------------

const U32 PACKET_FROM_FLASH = 0x10;
//  ��������� � ������ 15) ����-02. ��������� ���� �� ��������.
__packed struct S_WayPointPacket
{
  char    pcID[4];        // ������������� ������ "KMRT"
  U16     usSize;         // ������ ������ � ����������
  U8      ucType;         // ��� ������ - TYPE_PACKET_WAY_POINT
  U8      ucTypePoint;    // �� ������������
  U32     uiData;         // ���� ddmmyy (�������� 250109 - 25/01/2009)
  U32     uiTime;         // ����� hhmmsssss (�������� 234859000 - 23:48:59.000)
  float   fLat;           // ������ ddmm.mmmm(�������� 5352.2134 - 53?53.2134')
  float   fLon;           // ������� ddmm.mmmm(�������� 2752.2134 - 27?53.2134')
  U16     usAlt;          // ������ � ������
  U16     usCourse;       // ���� � ��������
  U16     usSpeed;        // �������� � ���������� � ���
  U16     usNumberPoint;  // ����� ������� ����� (������������ � ������ �������������) 
  U32     uiDistanse;     // �� ������������
  float   fPower;         // �������� ������� � �������
  U32     uiStatus;       // ������� ��� - ��������� ��������� ������ (0x01)
                          // 4 ��� (0x10) - 0 -- ����������� ����������
                          //              - 1 -- ����-������
  U16     pusAi[4];       // �������� 4-� ������. �������� ��� ����-04
                          // �������� 2-� ������. �������� ��� ����-05, 3 � 4  �� ����� ������
  U8      ucDi;           // �������� 1-�� (0x01) � 2-�� (0x02) �������. ��������
  S_WayPointPacket(void) : usSize(sizeof(S_WayPointPacket)), ucType(TYPE_PACKET_WAY_POINT), uiStatus(0)
                                    {pcID[0] = 'K'; pcID[1] = 'M'; pcID[2] = 'R'; pcID[3] = 'T'; }
  U16  GetNumberPoint(void){return usNumberPoint;} 
  void SetNumberPoint(U16 usX){usNumberPoint = usX;} 
  void SetPacketFromArchiv(void){uiStatus = uiStatus | PACKET_FROM_FLASH; }
};
//----------------------------------------------------------------------------

// #pragma pack(8)
//----------------------------------------------------------------------------

#endif //#ifndef PACKETS_DC_H
