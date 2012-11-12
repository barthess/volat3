#ifndef PACKETS_DC_H
#define PACKETS_DC_H
//============================================================================
// Объявление пакетов диспетчерского центра                                  
//============================================================================

#include <string.h>
//----------------------------------------------------------------------------

  #define TYPE_PACKET_TERMINAL_INFO               0x01
  #define TYPE_PACKET_WAY_POINT                   0x02
  #define TYPE_PACKET_NOOP_CONFIRM                0x03
  #define TYPE_PACKET_VERSION                     0x04    // Версии ПО
  #define TYPE_PACKET_SET_POLLING_CONFIG_CONFIRM  0x05    // Подтверждение параметров работы терминала
  #define TYPE_PACKET_SET_IP_CONFIG_CONFIRM       0x06    // Подтверждение параметров работы терминала
  #define TYPE_PACKET_SET_GPRS_CONFIG_CONFIRM     0x07    // Подтверждение параметров работы терминала
  #define TYPE_PACKET_SET_OUTPUT_CONFIRM          0x08    // Подтверждение установки состояния выхода
#define TYPE_PACKET_GET_CONFIG_CONFIRM          0x09    // Подтверждение приема параметров работы терминала
#define TYPE_PACKET_GET_FILE_BLOCK              0x0A    // Запрос блока данных обновлений
#define TYPE_PACKET_GET_FILE_SIZE               0x0B    // Запрос размера файла обновления

// Идентификаторы пакетов от сервера
  #define TYPE_PACKET_AUTH_PASSWORD               0x81
  #define TYPE_PACKET_WAY_POINT_CONFIRM           0x82
  #define TYPE_PACKET_NOOP                        0x83
  #define TYPE_PACKET_IS_UPDATE                   0x84    // Информация о наличии обновления
  #define TYPE_PACKET_SET_POLLING_CONFIG          0x85    // Установка параметров работы терминала
  #define TYPE_PACKET_SET_IP_CONFIG               0x86    // Установка параметров работы терминала
  #define TYPE_PACKET_SET_GPRS_CONFIG             0x87    // Установка параметров работы терминала
#define TYPE_PACKET_SET_OUTPUT                  0x88    // Установка состояния выхода
#define TYPE_PACKET_GET_CONFIG                  0x89    // Чтение параметров работы терминала
#define TYPE_PACKET_FILE_BLOCK                  0x8A    // Получить блок данных обновления
#define TYPE_PACKET_FILE_SIZE                   0x8B    // Получение размера файла обновления
//----------------------------------------------------------------------------

// #pragma pack(1)
//----------------------------------------------------------------------------

// Все пакеты имеют стандартный заголовок размером 7 байт
__packed struct S_DefaultPacket
{
  char    pcID[4];        // Идентификатор пакета "KMRT"
  U16     usSize;         // Размер пакета с заголовком
  U8      ucType;         // Тип пакета
};
//----------------------------------------------------------------------------

// После установления соединения терминал передает серверу пакет идентификационных данных
const U8 TERMINALNUMBERSIZE = 15;
__packed struct S_TerminalInfoPacket
{
  char    pcID[4];        // Идентификатор пакета "KMRT"
  U16     usSize;         // Размер пакета с заголовком
  U8      ucType;         // Тип пакета TYPE_PACKET_TERMINAL_INFO
  char    pcNumber[TERMINALNUMBERSIZE + 1];   // Номер терминала (например "0001")
  S_TerminalInfoPacket(const char *strzTereminalNumber) : usSize(sizeof(S_TerminalInfoPacket)), ucType(TYPE_PACKET_TERMINAL_INFO)  
  {pcID[0] = 'K'; pcID[1] = 'M'; pcID[2] = 'R'; pcID[3] = 'T'; strncpy(pcNumber, strzTereminalNumber, sizeof(pcNumber) - 1);}
}; 
//----------------------------------------------------------------------------

// В ответ сервер передает терминалу аутентификации с паролем 
const U8 GPSPASSWORDSIZE = 6;
__packed struct S_AuthPacket
{
  char    pcID[4];          // Идентификатор пакета "KMRT"
  U16     usSize;           // Размер пакета с заголовком
  U8      ucType;           // Тип пакета - TYPE_PACKET_AUTH_PASSWORD
  char    pcGPSPassowrd[GPSPASSWORDSIZE + 1]; // Пароль (пароль по умолчанию "123456"). Пароль установливается при настройке терминала
  S_AuthPacket(const char *strzGPSPassowrd) : usSize(sizeof(S_AuthPacket)), ucType(TYPE_PACKET_AUTH_PASSWORD)
  {pcID[0] = 'K'; pcID[1] = 'M'; pcID[2] = 'R'; pcID[3] = 'T';
   memset(pcGPSPassowrd, 0, sizeof(pcGPSPassowrd));  strncpy(pcGPSPassowrd, strzGPSPassowrd, sizeof(pcGPSPassowrd) - 1);}
};
// Если переданый сервером пароль не верен, то терминал разрывает соединение и начинает повторное соединение
//----------------------------------------------------------------------------

// После получения успешной аутентификации терминал посылает серверу пакет с версиями ПО и номерами SIM-карты 
// и IMEI, этот пакет предназначен для реализации удаленного обновления ПО терминала
__packed struct S_VersionPacket
{
  char    pcID[4];               // Идентификатор пакета "KMRT"
  U16     usSize;                // Размер пакета с заголовком
  U8      ucType;                // Тип пакета - TYPE_PACKET_VERSION
  char    pcTerminalType[10];    // Название терминала ("ТИНС-02")
  char    pcOS_Version[40];      // Версия операционной системы  
  char    pcWDWL_Version[10];    // Версия загрузчика           
  char    pcIMEI_Number[16];     // IMEI                     
  char    pcIMSI_Number[16];     // IMSI                 
  char    pcTerminalVersion[16]; // Версия ПО терминала
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

// После получения пакета с версиями ПО ДЦ формирует пакет с информацией о наличии обновления ПО
__packed struct S_UpdatePacket
{
  char    pcID[4];            // Идентификатор пакета "KMRT"
  U16     usSize;             // Размер пакета с заголовком
  U8      ucType;             // Тип пакета - TYPE_PACKET_IS_UPDATE
  U8      ucIsUpdate;         // Есть ли обновление
  U8      ucTypeUpdate;       // Тип обновления
                              // 1 - обновление загрузчика
                              // 2 - обновление ОС
                              // 3 - обновление приложения
  U8      pucNewVersion[16];  // Версия обновления
  U8      ucCountFiles;       // Количество файлов в обновлении
};
//----------------------------------------------------------------------------

// В процессе работы возможно удаленное изменение параметров работы терминала, для чего предназначены 
// следующие пакеты формируемы ДЦ
__packed struct S_ConfigPollingPacket
{
  char    pcID[4];          // Идентификатор пакета "KMRT"
  U16     usSize;           // Размер пакета с заголовком
  U8      ucType;           // Тип пакета - TYPE_PACKET_SET_POLLING_CONFIG
  U16     usGPRSDataTimeout;// Интервал выдачи навигационных данных
  U16     usGPRSDistanse;   // Расстояние выдачи навигационных данных
  U8      ucGPRSAzimut;     // Азимут передачи навигационных данных
  S_ConfigPollingPacket(U16 usTimeout, U16 usDistanse, U8 ucAzimut) : usSize(sizeof(S_ConfigPollingPacket)), ucType(TYPE_PACKET_SET_POLLING_CONFIG)
                          {pcID[0] = 'K'; pcID[1] = 'M'; pcID[2] = 'R'; pcID[3] = 'T';
                          usGPRSDataTimeout = usTimeout; usGPRSDistanse = usDistanse; ucGPRSAzimut = ucAzimut;}
};
//----------------------------------------------------------------------------

__packed struct S_ConfigIPPacket
{
  char    pcID[4];        // Идентификатор пакета "KMRT"
  U16     usSize;         // Размер пакета с заголовком
  U8      ucType;         // Тип пакета - TYPE_PACKET_SET_IP_CONFIG
  char    pcTcpServ[21];  // IP-адрес сервера
  U16     usTcpPort;      // TCP/IP порт
};
//----------------------------------------------------------------------------

__packed struct S_ConfigGPRSPacket
{
  char    pcID[4];        // Идентификатор пакета "KMRT"
  U16     usSize;         // Размер пакета с заголовком
  U8      ucType;         // Тип пакета - TYPE_PACKET_SET_GPRS_CONFIG
  char    pcApnServ[97];  // Точка входа в GPRS
  char    pcApnUn[65];    // Имя пользователя
  char    pcApnPw[65];    // Пароль пользователя
};
//----------------------------------------------------------------------------

// В процессе работы терминал формирует пакеты навигационных данных и передает их серверу
//__packed struct S_WayPointPacket 
//{
//  char    pcID[4];        // Идентификатор пакета "KMRT"
//  U16     usSize;         // Размер пакета с заголовком
//  U8      ucType;         // Тип пакета - TYPE_PACKET_WAY_POINT
//  U8      ucTypePoint;    // Не используется
//  U32     uiData;         // Дата ddmmyy (например 250109 - 25/01/2009)
//  U32     uiTime;         // Время hhmmsssss (например 234859000 - 23:48:59.000)
//  float   fLat;           // Широта ddmm.mmmm(например 5352.2134 - 53?53.2134')
//  float   fLon;           // Долгота ddmm.mmmm(например 2752.2134 - 27?53.2134')
//  U16     usAlt;          // Высота в метрах
//  U16     usCourse;       // Курс в градусах
//  U16     usSpeed;        // Скорость в километрах в час
//  U16     usNumberPoint;  // Номер путевой точки (используется в пакете подтверждения) 
//  U32     uiDistanse;     // Не используется
//  float   fPower;         // Бортовое питание в Вольтах
//  U32     uiStatus;       // Младший бит - состояние тревожной кнопки (0x01)
//  S_WayPointPacket(void) : usSize(sizeof(S_WayPointPacket)), ucType(TYPE_PACKET_WAY_POINT)
//                                    {pcID[0] = 'K'; pcID[1] = 'M'; pcID[2] = 'R'; pcID[3] = 'T'; }
//  U16  GetNumberPoint(void){return usNumberPoint;} 
//  void SetNumberPoint(U16 usX){usNumberPoint = usX;} 
//};
//----------------------------------------------------------------------------

// В ответ на пакет навигационных данных сервер отправляет пакет подтверждения
__packed struct S_WayPointConfirmPacket
{
  char    pcID[4];        // Идентификатор пакета "KMRT"
  U16     usSize;         // Размер пакета с заголовком
  U8      ucType;         // Тип пакета - TYPE_PACKET_WAY_POINT_CONFIRM
  U16     usNumberPoint;  // Номер путевой точки (см. TWayPointPacket)
  S_WayPointConfirmPacket(U16 usNumber) : usSize(sizeof(S_WayPointConfirmPacket)), ucType(TYPE_PACKET_WAY_POINT_CONFIRM)
                          {pcID[0] = 'K'; pcID[1] = 'M'; pcID[2] = 'R'; pcID[3] = 'T'; usNumberPoint = usNumber;}
};
//----------------------------------------------------------------------------

// Если терминал не получил пакет подтверждения, то при повторном соединении точка будет передана снова
// В процессе работы для контроля соединия сервер 1 раз в минуту отправляет пакет контроля
__packed struct S_NoopPacket
{
  char    pcID[4];        // Идентификатор пакета "KMRT"
  U16     usSize;         // Размер пакета с заголовком
  U8      ucType;         // Тип пакета - TYPE_PACKET_NOOP
};
//----------------------------------------------------------------------------

// В ответ терминал немедленно отправляет пакет  типа S_NoopConfirmPacket

// Если терминал в течении 2-х минут не получил пакет TNoopPacket терминал разрывает соединение 
// и иницирует повторное соединение.

// Если сервер в течении 2-минут после отправки TNoopPacket неполучил TNoopConfirmPacket, 
// сервер разрывает соединие.
//----------------------------------------------------------------------------

// Пакеты подтверждений
__packed struct S_NoopConfirmPacket
{
  char    pcID[4];        // Идентификатор пакета "KMRT"
  U16     usSize;         // Размер пакета с заголовком
  U8      ucType;         // Тип пакета - TYPE_PACKET_NOOP_CONFIRM
  S_NoopConfirmPacket(void) : usSize(sizeof(S_NoopConfirmPacket)), ucType(TYPE_PACKET_NOOP_CONFIRM)
                                    {pcID[0] = 'K'; pcID[1] = 'M'; pcID[2] = 'R'; pcID[3] = 'T'; }
};
//----------------------------------------------------------------------------

__packed struct S_SetPollingConfirmPacket
{
  char    pcID[4];        // Идентификатор пакета "KMRT"
  U16     usSize;         // Размер пакета с заголовком
  U8      ucType;         // Тип пакета - TYPE_PACKET_SET_POLLING_CONFIG_CONFIRM
  S_SetPollingConfirmPacket(void) : usSize(sizeof(S_SetPollingConfirmPacket)), ucType(TYPE_PACKET_SET_POLLING_CONFIG_CONFIRM)
                                    {pcID[0] = 'K'; pcID[1] = 'M'; pcID[2] = 'R'; pcID[3] = 'T'; }
};
//----------------------------------------------------------------------------

__packed struct S_SetIPConfirmPacket
{
  char    pcID[4];        // Идентификатор пакета "KMRT"
  U16     usSize;         // Размер пакета с заголовком
  U8      ucType;         // Тип пакета - TYPE_PACKET_SET_IP_CONFIG_CONFIRM
  S_SetIPConfirmPacket(void) : usSize(sizeof(S_SetIPConfirmPacket)), ucType(TYPE_PACKET_SET_IP_CONFIG_CONFIRM)
                               {pcID[0] = 'K'; pcID[1] = 'M'; pcID[2] = 'R'; pcID[3] = 'T'; }
};
//----------------------------------------------------------------------------

__packed struct S_SetGPRSConfirmPacket
{
  char    pcID[4];        // Идентификатор пакета "KMRT"
  U16     usSize;         // Размер пакета с заголовком
  U8      ucType;         // Тип пакета - TYPE_PACKET_SET_GPRS_CONFIG_CONFIRM
  S_SetGPRSConfirmPacket(void) : usSize(sizeof(S_SetGPRSConfirmPacket)), ucType(TYPE_PACKET_SET_GPRS_CONFIG_CONFIRM)
                                 {pcID[0] = 'K'; pcID[1] = 'M'; pcID[2] = 'R'; pcID[3] = 'T'; }
};
//----------------------------------------------------------------------------

__packed struct S_SetOutputConfirmPacket
{
  char    pcID[4];        // Идентификатор пакета "KMRT"
  U16     usSize;         // Размер пакета с заголовком
  U8      ucType;         // Тип пакета - TYPE_PACKET_SET_OUTPUT_CONFIRM
  S_SetOutputConfirmPacket(void) : usSize(sizeof(S_SetOutputConfirmPacket)), ucType(TYPE_PACKET_SET_OUTPUT_CONFIRM)
                                   {pcID[0] = 'K'; pcID[1] = 'M'; pcID[2] = 'R'; pcID[3] = 'T'; }
};
//----------------------------------------------------------------------------

// Протокол работы терминала ТИНС-05

// Протокол работы терминала ТИНС-05 включает в себя протокол работы ТИНС-02 
// плюс следующие возможности.

// Пакеты от ТИНС-05 к Серверу
#define TYPE_PACKET_NUMBER_MESSAGE     0x10    // Передача  номера сообщения 
#define TYPE_PACKET_MES_CMD_CONFIRM    0x30    // Подтверждение приема сообщения, номера сообщения и команды ЖКИ

// Пакеты от Сервера к ТИНС-05
#define TYPE_PACKET_MESSAGE            0x90    // Передача сообщения
#define TYPE_PACKET_CMD_SCRN           0x92    // Передача терминалу команд A,B,N,Z для ЖКЭ
//#define TYPE_PACKET_DYNAMIC_ELEMENT    0x93    // Передача терминалу команды   D для ЖКЭ
//#define TYPE_PACKET_SET_MAIN_MENU      0x91    // Передача терминалу команды W для ЖКЭ
//----------------------------------------------------------------------------

// Передача на Сервер пакета номера сообщения. Выполняется после того как  
// пользователь нажмёт клавишу Ввод при выборе из меню сообщений
__packed struct S_NumMessagePacket
{
  char  pcID[4];  // Идентификатор пакета "KMRT"
  U16   usSize;   // Размер пакета
  U8    ucType;   // Тип пакета: TYPE_PACKET_NUMBER_MESSAGE
  U32   uiData;   // Дата ddmmyy (например 250109 - 25/01/2009)
  U32   uiTime;   // Время hhmmsssss (например 234859000 - 23:48:59.000)
  U32   uiNumMes; // Номера сообщения
  S_NumMessagePacket(U32 uiNumberMSG) : usSize(sizeof(S_NumMessagePacket)), ucType(TYPE_PACKET_NUMBER_MESSAGE), uiData(0), uiTime(0)
      {pcID[0] = 'K'; pcID[1] = 'M'; pcID[2] = 'R'; pcID[3] = 'T'; uiNumMes = uiNumberMSG;}
};
//----------------------------------------------------------------------------

// Передача на ТИНС-05 пакета сообщения (команда T для ЖКЭ)
__packed struct S_MessagePacket
{
  #define  SIZETEXT  250
   char      pcID[4];        // Идентификатор пакета "KMRT"
   U16       usSize;         // Размер пакета
   U8        ucType;         // Тип пакета: TYPE_PACKET_MESSAGE
   char      pcMessage[SIZETEXT + 1];   // Текст сообщения
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

// Передача на ТИНС-05 пакета команд A,B,N,Z для ЖКЭ
__packed struct S_CmdScrnPacket
{
  #define  SIZETEXT  30
  char     pcID[4];        // Идентификатор пакета "KMRT"
  U16      usSize;         // Размер пакета
  U8       ucType;         // Тип пакета: TYPE_PACKET_CMD_SCRN
  U8       ucCmd;          // Команда для ЖКЭ: A,B,N,Z
  char     pcText[SIZETEXT + 1];
  S_CmdScrnPacket(U8 chCMD, char * strz) : usSize(sizeof(S_CmdScrnPacket)), ucType(TYPE_PACKET_CMD_SCRN)
      {pcID[0] = 'K'; pcID[1] = 'M'; pcID[2] = 'R'; pcID[3] = 'T'; ucCmd = chCMD; strncpy(pcText, strz, SIZETEXT);}
   U8 GetTextSizeMax(void) {return SIZETEXT;}
   #undef SIZETEXT
};
//----------------------------------------------------------------------------

//  Передача на ТИНС-05 или Сервер пакета подтверждения на все, вышеперечисленные пакеты, для ТИНС-05
__packed struct S_MesCmdConfirmPacket
{
   char      pcID[4];     // Идентификатор пакета "KMRT"
   U16       usSize;      // Размер пакета
   U8        ucType;      // Тип пакета: TYPE_PACKET_MES_CMD_CONFIRM
   U32       uiRetCod;    //  = 0 - нет ошибки
                          // != 0 - номер ошибки
  S_MesCmdConfirmPacket(void) : usSize(sizeof(S_MesCmdConfirmPacket)), ucType(TYPE_PACKET_MES_CMD_CONFIRM),
                                uiRetCod(0) {pcID[0] = 'K'; pcID[1] = 'M'; pcID[2] = 'R'; pcID[3] = 'T'; }
};
//----------------------------------------------------------------------------

const U32 PACKET_FROM_FLASH = 0x10;
//  Изменение к пункту 15) ТИНС-02. Добавлены поля по датчикам.
__packed struct S_WayPointPacket
{
  char    pcID[4];        // Идентификатор пакета "KMRT"
  U16     usSize;         // Размер пакета с заголовком
  U8      ucType;         // Тип пакета - TYPE_PACKET_WAY_POINT
  U8      ucTypePoint;    // Не используется
  U32     uiData;         // Дата ddmmyy (например 250109 - 25/01/2009)
  U32     uiTime;         // Время hhmmsssss (например 234859000 - 23:48:59.000)
  float   fLat;           // Широта ddmm.mmmm(например 5352.2134 - 53?53.2134')
  float   fLon;           // Долгота ddmm.mmmm(например 2752.2134 - 27?53.2134')
  U16     usAlt;          // Высота в метрах
  U16     usCourse;       // Курс в градусах
  U16     usSpeed;        // Скорость в километрах в час
  U16     usNumberPoint;  // Номер путевой точки (используется в пакете подтверждения) 
  U32     uiDistanse;     // Не используется
  float   fPower;         // Бортовое питание в Вольтах
  U32     uiStatus;       // Младший бит - состояние тревожной кнопки (0x01)
                          // 4 бит (0x10) - 0 -- оперативная информация
                          //              - 1 -- флэш-память
  U16     pusAi[4];       // Значения 4-х аналог. датчиков лля ТИНС-04
                          // Значения 2-х аналог. датчиков лля ТИНС-05, 3 и 4  не имеют смысла
  U8      ucDi;           // Значения 1-го (0x01) и 2-го (0x02) дискрет. датчиков
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
