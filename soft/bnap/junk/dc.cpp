//============================================================================
// Обработка команд и подтверждений диспетчерского центра                                   
//============================================================================
// 
//============================================================================

#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "errorhandler.h"
#include "gsm.h"
#include "msg.h"
#include "flashbuf.h"
#include "dc.h"
#include "parameters.h"
#include "terminal.h"
#include "profiler.h"
//----------------------------------------------------------------------------

#define D_PROFILER
#ifdef D_PROFILER
  extern C_Profiler Profiler;
#endif

U32 DiffTime(U32 uiOldTime);
extern C_PacketQueue PacketQueue;
extern bool boMMC_REDY;
extern char strz80[80];
extern C_Terminal    Terminal;
//----------------------------------------------------------------------------

const U8 STATE_LINK = 0;
const U8 STATE_LOGIN = 1;
const U8 STATE_WORK = 2;
const U8 MIN_PACKET_SIZE = 7;
//---------------------------------------------------------------------------

void Restart(void){
//  GSM.HardReset();
  DC.TCPbufClear();
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Конвертирует массив элементов типа  char в массив 16-тиричных символов 
// Одному входному элементу соответствует два выходнх
// usInSize - число входных элементов. 
//----------------------------------------------------------------------------
void CharToHex(char *pOut, const char *pIn, U16 usInSize){
  U8 ucY;
  int i;
  for(i = 0; i < usInSize; ++i){
    U8 ucX = (pIn[i] >> 4) & 0x0F;
    if(9 >= ucX) ucY = ucX + '0';
    else ucY = ucX + 'A';
    pOut[2*i] = ucY;
    ucX = pIn[i] & 0x0F;
    if(9 >= ucX) ucY = ucX + '0';
    else ucY = ucX + 'A';
    pOut[2*i + 1] = ucY;
  }
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Конвертирует массив 16-тиричных символов в массив элементов типа char
// Двум входным элементам соответствует один выходной
// usOutSize - число выходных элементов. 
// Возвращаемое значение: true - ОК, false - ошибка, недопустимый символ
//----------------------------------------------------------------------------
bool HexToChar(U8 *pOut, const char *pIn, U16 usOutSize){
  U8 ucY;
  int i;
  for(i = 0; i < usOutSize; ++i){
    U8 ucX = pIn[2*i];
    if(('0' <= ucX) && ('9'>= ucX))       ucY = (ucX - '0') << 4;
    else if(('A' <= ucX) && ('F' >= ucX)) ucY = (ucX - 'A' + 10) << 4;
         else if(('a' <= ucX) && ('f' >= ucX)) ucY = (ucX - 'a' + 10) << 4;
              else return false;
    ucX = pIn[2*i + 1];
    if(('0' <= ucX) && ('9'>= ucX))       ucY = ucY + (ucX - '0');
    else if(('A' <= ucX) && ('F' >= ucX)) ucY = ucY + (ucX - 'A' + 10);
         else if(('a' <= ucX) && ('f' >= ucX)) ucY = ucY + (ucX - 'a' + 10);
              else return false;        
    pOut[i] = ucY;
  }
  return true;
}
//----------------------------------------------------------------------------

//---------------------------------------------------------------------------
// формирует сообщение с командой mgData.
//---------------------------------------------------------------------------
C_Msg * C_DC::BuildCMD_ReceiveData(void)
{
  C_Msg * pMsg  = NewMsg(ID_GSM, ID_DC, MailBox, MSG_CMDWITHRESPONSE, 9);   
  if(NULL != pMsg){  
    char* p = pMsg->PointerStr(); 
    p[0] = START_DELIMITER;
    p[1] = 'm';
    p[2] = 'g';
    p[3] = 'D';
    p[4] = 'a';
    p[5] = 't';
    p[6] = 'a';
    p[7] = END_DELIMITER;
    p[8] = 0;
  }
  return pMsg;
}
//----------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Помещает данные из подтверждения в ТСР буфер.
//---------------------------------------------------------------------------
U32 C_DC::SetDataToBuf(char* szArg)
{
  // В подтверждении нет аргумента
  if(ARG_DELIMITER != szArg[0]) return NO_ERROR;
  
  // Устанавливаю указатель на 1-й символ аргумента
  ++szArg;      
  // Вместо конечного ограничителя записываю завершающий 0
  szArg[strlen(szArg) - 1] = 0;   
            
  // Проверяю корректность формата. Аргумент должен содержать четное число символов
  U16 usArgumentLength = strlen(szArg);
  if(0 != (usArgumentLength % 2)) return ERROR_DC_1;

  // Проверяю аргумент на наличие посторонних символов
  for(int i = 0; i < usArgumentLength; ++i){
    if(0 == isxdigit(szArg[i])) return ERROR_DC_2;
  }

  // Проверяю наличие места в ТСР буфере. ( Два HEX символа конвертируются в 1 char ) 
  usArgumentLength /= 2;
  if(TCPBufer.SizeFree() < usArgumentLength/2) return ERROR_DC_3;

  // Конвертирую данные из HEX формата в двоичный формат и помещаю их в циклический буфер
  for(int i = 0; i < usArgumentLength; ++i){ 
    U8 cTemp;
    if(!HexToChar(&cTemp, szArg + 2*i, 1)) break;
    TCPBufer.Insert(&cTemp);
  }
  boDataRedy = true;
  return NO_ERROR;
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Выделяет из подтверждения слово состояния и данные.
// Слово состояния, первые 8 байтов строки подтверждения (в HEX формате)
// аргумент (в HEX формате) - все остальное 
//---------------------------------------------------------------------------
U32 C_DC::GetStateAndDataFromArg(char* szResponse)
{
  U32 uiError;
  // Выделяю слово состояния. Пропускаю начальный ограничитель  
  // Нужно повернуть, поэтому преобразую побайтно
  szResponse += 1;
  uiError = ERROR_DC_4;
  if(HexToChar(reinterpret_cast<U8*>(&uiStateGSM) + 0, szResponse + 6, 1))  
    if(HexToChar(reinterpret_cast<U8*>(&uiStateGSM) + 1, szResponse + 4, 1))  
      if(HexToChar(reinterpret_cast<U8*>(&uiStateGSM) + 2, szResponse + 2, 1))  
        if(HexToChar(reinterpret_cast<U8*>(&uiStateGSM) + 3, szResponse + 0, 1)){  
          // Выделяю аргумент.  pArg - указатель на разделитель между словом состояния и данными
          char *pArg = szResponse + 8;  
          uiError = SetDataToBuf(pArg);
        }

  return uiError;
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Получает и обрабатывает подтверждение
//---------------------------------------------------------------------------
U32 C_DC::GetResponse(void)
{
  C_Msg* pResponse;
  U32 uiError = os_mbx_wait(MailBox, (void **)&(pResponse), TIMEOUT_RESPONSE_REDY);

  // Подтверждение не получено
  if(OS_R_TMO == uiError) return ERROR_DC_5;   
  // Подтверждение получено. 
  else {                
    uiError = GetStateAndDataFromArg(pResponse->PointerStr());
    pResponse->Release();
    return uiError;
  }
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Выполняет команду "Получить данные" от модема.
//---------------------------------------------------------------------------
U32 C_DC::ReceiveData(void)
{
  U32 uiError;
  // Отправляю команду "Получить данные"
  C_Msg* pMsg = BuildCMD_ReceiveData();
  uiError = GSM.SendMsgToGSM(pMsg);
  if(NO_ERROR != uiError){
    uiError = 0;
  }
  pMsg->Release();

  // Ожидаю подтверждение
  return GetResponse();
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Формирует команду посылки данных в модем. Данные в HEX формате 
// следуют сразу за последним символом команды
// pData  - указатель на данные
// usSize - размер данных
//----------------------------------------------------------------------------
C_Msg* C_DC::BuildCMD_SendData(const char* pData, U16 usSize)
{
  C_Msg * pMsg  = NewMsg(ID_GSM, ID_DC, MailBox, MSG_CMDWITHRESPONSE, usSize + 10);   
  if(NULL != pMsg){  
    char* p = pMsg->PointerStr(); 
    p[0] = START_DELIMITER;
    p[1] = 'm';
    p[2] = 's';
    p[3] = 'D';
    p[4] = 'a';
    p[5] = 't';
    p[6] = 'a';
    p[7] = ARG_DELIMITER; 
    memcpy(p + 8, pData, usSize);
    p[usSize + 8] = END_DELIMITER;
    p[usSize + 9] = 0;
  }
  return pMsg;
}
//---------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Отправляет данные в HEX формате в модем. Принимает подтверждение. 
// В подтверждении кроме слова состояния могут содержаться данные от модема.
// pData  - указатель на данные
// usSize - размер данных
//----------------------------------------------------------------------------
U32 C_DC::SendData(const char* pData, U16 usSize)
{
  // Отправляю команду "Передать данные"
  C_Msg* pMsg = BuildCMD_SendData(pData, usSize);
  GSM.SendMsgToGSM(pMsg);
  pMsg->Release();

  // Ожидаю подтверждение
  return GetResponse();
}
//---------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Отправляет пакет в диспетчерский центр
// Данные отправляются поблочно. Максимальный размер блок равен BLOKSIZE. 
// (После преобразование в HEX 2*BLOKSIZE). 
// pData  - указатель на пакет, отправляемый диспетчерскому центру
// usSize - размер пакета
//----------------------------------------------------------------------------
U32 C_DC::SendPacketToDC(const char *pData, U16 usSize)
{
  U32 uiError;
  while(BLOKSIZE < usSize){
    // Преобразую в HEX
    CharToHex(pHEXData, pData, BLOKSIZE);
    // Отправляю блок модему
    uiError = SendData(pHEXData, 2*BLOKSIZE);
    if(NO_ERROR != uiError) return uiError;
    usSize -= BLOKSIZE;
    pData  += BLOKSIZE;
  }
  // Преобразую в HEX
  CharToHex(pHEXData, pData, usSize);
  // Отправляю блок модему
  uiError = SendData(pHEXData, 2*usSize);
  return uiError;
}
//----------------------------------------------------------------------------

inline void C_DC::SetStateLINK(void) { ucStateTask = STATE_LINK; } 

//----------------------------------------------------------------------------
// Отправляет пакет с номером терминала
//----------------------------------------------------------------------------
U32 C_DC::Link(void)
{  
  S_TerminalInfoPacket TerminalInfoPacket(Parameters->GetTerminalNumber());
  U32 uiError = SendPacketToDC((char *)&TerminalInfoPacket, sizeof(S_TerminalInfoPacket));

  // Переожу в состояние STATE_LOGIN
  if(NO_ERROR == uiError) {
    ucStateTask = STATE_LOGIN; 
    TCPbufClear();
  }

  return uiError; 
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Открывает сессию. 
//----------------------------------------------------------------------------
void C_DC::Login(C_Msg* pMsg)
{
  S_AuthPacket* AuthPacket;
  pMsg->AddRef(); 

  S_DefaultPacket* DefaultPacket = reinterpret_cast<S_DefaultPacket*>(pMsg->PointerStr());

  // Проверяю тип полученного пакета. Неверный тип пакета
  if(TYPE_PACKET_AUTH_PASSWORD != DefaultPacket->ucType) goto END;

  // Сравниваю пароли
  AuthPacket = reinterpret_cast<S_AuthPacket*>(pMsg->PointerStr());

  // Пароль принят, перехожу в состояние STATE_WORK
  if(0 == strcmp(AuthPacket->pcGPSPassowrd, Parameters->GetGPSPassowrd()))
    ucStateTask = STATE_WORK; 
  // Пароль не принят, возвращаюсь в состояние STATE_LINK
  else SetStateLINK();     

END:
  pMsg->Release();    
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Обрабатывает подтверждение пакета с путевеой информацией
//----------------------------------------------------------------------------
U32 C_DC::WayPointConfirm(C_Msg* pMsg)
{
  U32 uiError;
  pMsg->AddRef(); 

  S_WayPointConfirmPacket * WayPointConfirmPacket = reinterpret_cast<S_WayPointConfirmPacket*>(pMsg->PointerStr());
  U16 usNumber = WayPointConfirmPacket->usNumberPoint;
  // Удаляю пакет на который получено подтверждение из очереди и буфера
  uiError = PacketQueue.ResponseHandler(usNumber);

  pMsg->Release();    
  return uiError;
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Передает терминалу указатель на сообщение с текстом
//----------------------------------------------------------------------------
U32 C_DC::TextForLCD(C_Msg* pMsg){
  U32 uiError;
  pMsg->AddRef();
   
  // Передаю указатель    
  Terminal.SetPointerpTextMsg(pMsg);
  // Отправляю подтверждение ДЦ
  const S_SetPollingConfirmPacket SetPollingConfirmPacket;
  uiError = SendPacketToDC((char *)&SetPollingConfirmPacket, sizeof(S_SetPollingConfirmPacket));

  pMsg->Release();    
  return uiError;
}
//----------------------------------------------------------------------------

U32 C_DC::CMDforLCD(C_Msg* pMsg){
  U32 uiError;
  pMsg->AddRef(); 

  // Команда A, B, N, Z панели
  S_CmdScrnPacket *pCmdScrnPacket = reinterpret_cast<S_CmdScrnPacket*>(pMsg->PointerStr());
  bool boConfirm = true;
  switch(pCmdScrnPacket->ucCmd){
        case 'A':
             StateHandler->SetBusStop(pCmdScrnPacket->pcText);
             break;  
        case 'B':
             StateHandler->SetNextBusStop(pCmdScrnPacket->pcText);
             break;  
        case 'N':
             StateHandler->SetNumberBus(pCmdScrnPacket->pcText);
             break;  
        case 'Z':
             StateHandler->SetShift(pCmdScrnPacket->pcText);
             break;  
         default:
             boConfirm = false;
             break;  
  } 

  // Если команда принята то, отправляю подтверждение ДЦ
  if(boConfirm){
    const S_MesCmdConfirmPacket MesCmdConfirmPacket;
    uiError = SendPacketToDC((char *)&MesCmdConfirmPacket, sizeof(S_MesCmdConfirmPacket));
    boConfirm = false;
  }

  pMsg->Release();    
  return uiError;
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
U32 C_DC::NOOPPacketHandler(void)
{
  // Сохраняю время получения пакета PACKET_NOOP
  uiTimeNOOP = DiffTime(0);
  const S_NoopConfirmPacket  NoopConfirmPacket;
  return SendPacketToDC((char *)&NoopConfirmPacket, sizeof(S_NoopConfirmPacket));
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Устанавливает параметры режима формирования путевой информации          
//----------------------------------------------------------------------------
U32 C_DC::SetPollingConfig(C_Msg* pMsg)
{
  pMsg->AddRef(); 

  S_ConfigPollingPacket *pConfigPollingPacket = (S_ConfigPollingPacket *)(pMsg->PointerStr());
  // Сохраняю принятые данные в структуре Parameters 
  Parameters->SetWPTimeout(pConfigPollingPacket->usGPRSDataTimeout);
  Parameters->SetWPDistanse(pConfigPollingPacket->usGPRSDistanse);
  Parameters->SetWPAzimut(pConfigPollingPacket->ucGPRSAzimut);
  // Отправляю подтверждение ДЦ
  const S_SetPollingConfirmPacket SetPollingConfirmPacket;
  U32 uiError = SendPacketToDC((char *)&SetPollingConfirmPacket, sizeof(S_SetPollingConfirmPacket));

  pMsg->Release();    
  return uiError;
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
U32 C_DC::PacketHandler(C_Msg* pMsg)
{
  U32 uiError;
  pMsg->AddRef(); 

  S_DefaultPacket* DefaultPacket = reinterpret_cast<S_DefaultPacket*>(pMsg->PointerStr());

  switch(DefaultPacket->ucType)
  {
    // Подтверждение пакета с путевой информацией     
    case TYPE_PACKET_WAY_POINT_CONFIRM: 
        uiError = WayPointConfirm(pMsg);
        break;

    // Команда A, B, N, Z панели
    case TYPE_PACKET_CMD_SCRN: 
        uiError = CMDforLCD(pMsg);
        break;

    // Cообщение с текстом от ДЦ
    case TYPE_PACKET_MESSAGE:
        uiError = TextForLCD(pMsg);          
        break;

    // Пустой пакет для поддержки соединения     
    case TYPE_PACKET_NOOP:
        uiError = NOOPPacketHandler();
        break;
    // Параметры, определяющиме режим формирования путевой информации          
    case TYPE_PACKET_SET_POLLING_CONFIG:
        uiError = SetPollingConfig(pMsg);
        break;

    default:
        break;
  }

  pMsg->Release();    
  return uiError;
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Выделяет пакеты из данных ТСР буфера
//----------------------------------------------------------------------------
U32 C_DC::GetPacketFromTCPBufer(C_Msg** pMsg)
{
  U32 uiError;

  // Есть начало пакета
  if(BUFTCP_OK == DC.TCPBufer.CmpBufBeg("KMRT", 4))
  {
    U16 usPacketSize;
    DC.TCPBufer.Read(reinterpret_cast<U8 *>(&usPacketSize), 4, 2);  

    // Есть полный пакет.
    if(DC.TCPBufer.Size() >= usPacketSize){ 
      // Забираю принятый пакет
      *pMsg = NewMsg(ID_DC, ID_DC, NOMAILBOX, MSG_CMDWITHOUTRESPONSE, usPacketSize);
      DC.TCPBufer.Get(reinterpret_cast<U8*>((*pMsg)->PointerStr()), usPacketSize);
      if(DC.TCPBufer.Size() < MIN_PACKET_SIZE) boDataRedy = false;
      uiError = NO_ERROR;
    }           
    // В буфере нет полного пакета. 
    else{ 
      boDataRedy = false;  
      *pMsg = NULL;
      uiError = NO_ERROR;
    } 
  }
  // Нет начала пакета
  else uiError = ERROR_DC_TCP;

  return uiError;
}
//----------------------------------------------------------------------------

//---------------------------------------------------------------------------
// !!! Только это задача может забирать данные от ДС !!!
//---------------------------------------------------------------------------
__task void Task_DC(void) 
{
  U32 TIME_MAX_NOOP = 120000;  // в мсек. 2 минуту
  U32 TIME_MAX_LOGIN = 10000;  // в мсек. 10 сек

  U32 uiError;
  // Время нахождения Task_DC в состоянии STATE_LOGIN
  static U32 uiTimeLOGIN;

  for(;;)
  {
    os_dly_wait(20);   // 200 мс.

    // Формирую пакет с путевой информацией и помещаю его в архив.
    PacketQueue.GetNewWayPacket();
    uiError = PacketQueue.InsertToArchiv();

    // Опрашиваю модем
    uiError = DC.ReceiveData();
    if(NO_ERROR != uiError){
     // Что-то надо делать
    }

    // Нет ТСР соединения
    const U32 MASK = STATEGSM_GPRSMISSING | STATEGSM_TCPMISSING;
    if(0 != (DC.uiStateGSM & MASK)){
      DC.SetStateLINK();
      // Сохраняю состояние "Нет ТСР соединения"
      StateHandler->ResetTCPStatus();      
    }

    // ТСР соединение установлено
    else
    {
      // Сохраняю состояние "Есть ТСР соединение"
      StateHandler->SetTCPStatus();    
      switch(DC.ucStateTask)
      {
        case STATE_LINK: 
            // Начинаю отсчет времени нахождения в состояния STATE_LOGIN
            uiTimeLOGIN = DiffTime(0);
            uiError = DC.Link();         
            break;
    
        case STATE_LOGIN:
            // Время ожидания истекло
            if(TIME_MAX_LOGIN < DiffTime(uiTimeLOGIN)) DC.SetStateLINK();
            // Вреемя ожидания не истекло
            else {
              // В ТСР буфере есть данные
              if(DC.boDataRedy){
                C_Msg* pMsg;
                uiError = DC.GetPacketFromTCPBufer(&pMsg);
                if(NO_ERROR == uiError) {
                  // Если в буфере нет полного пакета, то pMsg = 0
                  if(NULL != pMsg){
                    DC.Login(pMsg);
                    pMsg->Release();
                    DC.uiTimeNOOP = DiffTime(0);
                  }
                }
                else {
                  if(ERROR_MEMORY_ALLOC == uiError) Restart();
                  if(ERROR_DC_TCP == uiError) DC.SetStateLINK();
                }
              }
            }
            break;
    
        case STATE_WORK:
            // Время ожидания истекло
            if(TIME_MAX_NOOP < DiffTime(DC.uiTimeNOOP)) DC.SetStateLINK();
            // Время ожидания не истекло
            else{
              // В ТСР буфере есть данные
              if(DC.boDataRedy){
                C_Msg* pMsg;
                uiError = DC.GetPacketFromTCPBufer(&pMsg);
                if(NO_ERROR == uiError) {
                  // Если в буфере нет полного пакета, то pMsg = 0
                  if(NULL != pMsg){  
                    uiError = DC.PacketHandler(pMsg);
                    pMsg->Release();
                  }
                }
                else {
                  if(ERROR_MEMORY_ALLOC == uiError) Restart();
                  if(ERROR_DC_TCP == uiError) DC.SetStateLINK();
                }
              }

              // Помещаю новые данные и данные из архива в очередь
              uiError = PacketQueue.InsertToQueue(1);
              if(NO_ERROR != uiError) {

              }

              uiError = PacketQueue.InsertToQueueFromArchiv();
              if(NO_ERROR != uiError) {

              }
          
              // Отправляю в ДЦ пакеты с путевой информацией
              uiError = PacketQueue.Queue_Handler(); 
              if(NO_ERROR == uiError) {

              }
            }
            break;
    
        // На всякий случай
        default:
            DC.SetStateLINK();
            break;
      } // Конец switch(DC.ucStateTask)
    }  // Конец блока "ТСР соединение установлено"
  }
}
//----------------------------------------------------------------------------

//     if(0 == uiCount % 64){
//        ErrorHandler.ErrorToList((ID_DC << 24) + (5 << 16) + uiCount/4);
//     }
//    if(NO_ERROR != uiError)
//      ErrorHandler.ErrorToList((ID_DC << 24) + (1 << 16) + uiError);
