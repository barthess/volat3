//============================================================================
// ��������� ������ � ������������� �������������� ������                                   
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
// ������������ ������ ��������� ����  char � ������ 16-�������� �������� 
// ������ �������� �������� ������������� ��� �������
// usInSize - ����� ������� ���������. 
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
// ������������ ������ 16-�������� �������� � ������ ��������� ���� char
// ���� ������� ��������� ������������� ���� ��������
// usOutSize - ����� �������� ���������. 
// ������������ ��������: true - ��, false - ������, ������������ ������
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
// ��������� ��������� � �������� mgData.
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
// �������� ������ �� ������������� � ��� �����.
//---------------------------------------------------------------------------
U32 C_DC::SetDataToBuf(char* szArg)
{
  // � ������������� ��� ���������
  if(ARG_DELIMITER != szArg[0]) return NO_ERROR;
  
  // ������������ ��������� �� 1-� ������ ���������
  ++szArg;      
  // ������ ��������� ������������ ��������� ����������� 0
  szArg[strlen(szArg) - 1] = 0;   
            
  // �������� ������������ �������. �������� ������ ��������� ������ ����� ��������
  U16 usArgumentLength = strlen(szArg);
  if(0 != (usArgumentLength % 2)) return ERROR_DC_1;

  // �������� �������� �� ������� ����������� ��������
  for(int i = 0; i < usArgumentLength; ++i){
    if(0 == isxdigit(szArg[i])) return ERROR_DC_2;
  }

  // �������� ������� ����� � ��� ������. ( ��� HEX ������� �������������� � 1 char ) 
  usArgumentLength /= 2;
  if(TCPBufer.SizeFree() < usArgumentLength/2) return ERROR_DC_3;

  // ����������� ������ �� HEX ������� � �������� ������ � ������� �� � ����������� �����
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
// �������� �� ������������� ����� ��������� � ������.
// ����� ���������, ������ 8 ������ ������ ������������� (� HEX �������)
// �������� (� HEX �������) - ��� ��������� 
//---------------------------------------------------------------------------
U32 C_DC::GetStateAndDataFromArg(char* szResponse)
{
  U32 uiError;
  // ������� ����� ���������. ��������� ��������� ������������  
  // ����� ���������, ������� ���������� ��������
  szResponse += 1;
  uiError = ERROR_DC_4;
  if(HexToChar(reinterpret_cast<U8*>(&uiStateGSM) + 0, szResponse + 6, 1))  
    if(HexToChar(reinterpret_cast<U8*>(&uiStateGSM) + 1, szResponse + 4, 1))  
      if(HexToChar(reinterpret_cast<U8*>(&uiStateGSM) + 2, szResponse + 2, 1))  
        if(HexToChar(reinterpret_cast<U8*>(&uiStateGSM) + 3, szResponse + 0, 1)){  
          // ������� ��������.  pArg - ��������� �� ����������� ����� ������ ��������� � �������
          char *pArg = szResponse + 8;  
          uiError = SetDataToBuf(pArg);
        }

  return uiError;
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// �������� � ������������ �������������
//---------------------------------------------------------------------------
U32 C_DC::GetResponse(void)
{
  C_Msg* pResponse;
  U32 uiError = os_mbx_wait(MailBox, (void **)&(pResponse), TIMEOUT_RESPONSE_REDY);

  // ������������� �� ��������
  if(OS_R_TMO == uiError) return ERROR_DC_5;   
  // ������������� ��������. 
  else {                
    uiError = GetStateAndDataFromArg(pResponse->PointerStr());
    pResponse->Release();
    return uiError;
  }
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// ��������� ������� "�������� ������" �� ������.
//---------------------------------------------------------------------------
U32 C_DC::ReceiveData(void)
{
  U32 uiError;
  // ��������� ������� "�������� ������"
  C_Msg* pMsg = BuildCMD_ReceiveData();
  uiError = GSM.SendMsgToGSM(pMsg);
  if(NO_ERROR != uiError){
    uiError = 0;
  }
  pMsg->Release();

  // ������ �������������
  return GetResponse();
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// ��������� ������� ������� ������ � �����. ������ � HEX ������� 
// ������� ����� �� ��������� �������� �������
// pData  - ��������� �� ������
// usSize - ������ ������
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
// ���������� ������ � HEX ������� � �����. ��������� �������������. 
// � ������������� ����� ����� ��������� ����� ����������� ������ �� ������.
// pData  - ��������� �� ������
// usSize - ������ ������
//----------------------------------------------------------------------------
U32 C_DC::SendData(const char* pData, U16 usSize)
{
  // ��������� ������� "�������� ������"
  C_Msg* pMsg = BuildCMD_SendData(pData, usSize);
  GSM.SendMsgToGSM(pMsg);
  pMsg->Release();

  // ������ �������������
  return GetResponse();
}
//---------------------------------------------------------------------------

//----------------------------------------------------------------------------
// ���������� ����� � ������������� �����
// ������ ������������ ��������. ������������ ������ ���� ����� BLOKSIZE. 
// (����� �������������� � HEX 2*BLOKSIZE). 
// pData  - ��������� �� �����, ������������ �������������� ������
// usSize - ������ ������
//----------------------------------------------------------------------------
U32 C_DC::SendPacketToDC(const char *pData, U16 usSize)
{
  U32 uiError;
  while(BLOKSIZE < usSize){
    // ���������� � HEX
    CharToHex(pHEXData, pData, BLOKSIZE);
    // ��������� ���� ������
    uiError = SendData(pHEXData, 2*BLOKSIZE);
    if(NO_ERROR != uiError) return uiError;
    usSize -= BLOKSIZE;
    pData  += BLOKSIZE;
  }
  // ���������� � HEX
  CharToHex(pHEXData, pData, usSize);
  // ��������� ���� ������
  uiError = SendData(pHEXData, 2*usSize);
  return uiError;
}
//----------------------------------------------------------------------------

inline void C_DC::SetStateLINK(void) { ucStateTask = STATE_LINK; } 

//----------------------------------------------------------------------------
// ���������� ����� � ������� ���������
//----------------------------------------------------------------------------
U32 C_DC::Link(void)
{  
  S_TerminalInfoPacket TerminalInfoPacket(Parameters->GetTerminalNumber());
  U32 uiError = SendPacketToDC((char *)&TerminalInfoPacket, sizeof(S_TerminalInfoPacket));

  // ������� � ��������� STATE_LOGIN
  if(NO_ERROR == uiError) {
    ucStateTask = STATE_LOGIN; 
    TCPbufClear();
  }

  return uiError; 
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// ��������� ������. 
//----------------------------------------------------------------------------
void C_DC::Login(C_Msg* pMsg)
{
  S_AuthPacket* AuthPacket;
  pMsg->AddRef(); 

  S_DefaultPacket* DefaultPacket = reinterpret_cast<S_DefaultPacket*>(pMsg->PointerStr());

  // �������� ��� ����������� ������. �������� ��� ������
  if(TYPE_PACKET_AUTH_PASSWORD != DefaultPacket->ucType) goto END;

  // ��������� ������
  AuthPacket = reinterpret_cast<S_AuthPacket*>(pMsg->PointerStr());

  // ������ ������, �������� � ��������� STATE_WORK
  if(0 == strcmp(AuthPacket->pcGPSPassowrd, Parameters->GetGPSPassowrd()))
    ucStateTask = STATE_WORK; 
  // ������ �� ������, ����������� � ��������� STATE_LINK
  else SetStateLINK();     

END:
  pMsg->Release();    
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// ������������ ������������� ������ � �������� �����������
//----------------------------------------------------------------------------
U32 C_DC::WayPointConfirm(C_Msg* pMsg)
{
  U32 uiError;
  pMsg->AddRef(); 

  S_WayPointConfirmPacket * WayPointConfirmPacket = reinterpret_cast<S_WayPointConfirmPacket*>(pMsg->PointerStr());
  U16 usNumber = WayPointConfirmPacket->usNumberPoint;
  // ������ ����� �� ������� �������� ������������� �� ������� � ������
  uiError = PacketQueue.ResponseHandler(usNumber);

  pMsg->Release();    
  return uiError;
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// �������� ��������� ��������� �� ��������� � �������
//----------------------------------------------------------------------------
U32 C_DC::TextForLCD(C_Msg* pMsg){
  U32 uiError;
  pMsg->AddRef();
   
  // ������� ���������    
  Terminal.SetPointerpTextMsg(pMsg);
  // ��������� ������������� ��
  const S_SetPollingConfirmPacket SetPollingConfirmPacket;
  uiError = SendPacketToDC((char *)&SetPollingConfirmPacket, sizeof(S_SetPollingConfirmPacket));

  pMsg->Release();    
  return uiError;
}
//----------------------------------------------------------------------------

U32 C_DC::CMDforLCD(C_Msg* pMsg){
  U32 uiError;
  pMsg->AddRef(); 

  // ������� A, B, N, Z ������
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

  // ���� ������� ������� ��, ��������� ������������� ��
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
  // �������� ����� ��������� ������ PACKET_NOOP
  uiTimeNOOP = DiffTime(0);
  const S_NoopConfirmPacket  NoopConfirmPacket;
  return SendPacketToDC((char *)&NoopConfirmPacket, sizeof(S_NoopConfirmPacket));
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// ������������� ��������� ������ ������������ ������� ����������          
//----------------------------------------------------------------------------
U32 C_DC::SetPollingConfig(C_Msg* pMsg)
{
  pMsg->AddRef(); 

  S_ConfigPollingPacket *pConfigPollingPacket = (S_ConfigPollingPacket *)(pMsg->PointerStr());
  // �������� �������� ������ � ��������� Parameters 
  Parameters->SetWPTimeout(pConfigPollingPacket->usGPRSDataTimeout);
  Parameters->SetWPDistanse(pConfigPollingPacket->usGPRSDistanse);
  Parameters->SetWPAzimut(pConfigPollingPacket->ucGPRSAzimut);
  // ��������� ������������� ��
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
    // ������������� ������ � ������� �����������     
    case TYPE_PACKET_WAY_POINT_CONFIRM: 
        uiError = WayPointConfirm(pMsg);
        break;

    // ������� A, B, N, Z ������
    case TYPE_PACKET_CMD_SCRN: 
        uiError = CMDforLCD(pMsg);
        break;

    // C�������� � ������� �� ��
    case TYPE_PACKET_MESSAGE:
        uiError = TextForLCD(pMsg);          
        break;

    // ������ ����� ��� ��������� ����������     
    case TYPE_PACKET_NOOP:
        uiError = NOOPPacketHandler();
        break;
    // ���������, ������������� ����� ������������ ������� ����������          
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
// �������� ������ �� ������ ��� ������
//----------------------------------------------------------------------------
U32 C_DC::GetPacketFromTCPBufer(C_Msg** pMsg)
{
  U32 uiError;

  // ���� ������ ������
  if(BUFTCP_OK == DC.TCPBufer.CmpBufBeg("KMRT", 4))
  {
    U16 usPacketSize;
    DC.TCPBufer.Read(reinterpret_cast<U8 *>(&usPacketSize), 4, 2);  

    // ���� ������ �����.
    if(DC.TCPBufer.Size() >= usPacketSize){ 
      // ������� �������� �����
      *pMsg = NewMsg(ID_DC, ID_DC, NOMAILBOX, MSG_CMDWITHOUTRESPONSE, usPacketSize);
      DC.TCPBufer.Get(reinterpret_cast<U8*>((*pMsg)->PointerStr()), usPacketSize);
      if(DC.TCPBufer.Size() < MIN_PACKET_SIZE) boDataRedy = false;
      uiError = NO_ERROR;
    }           
    // � ������ ��� ������� ������. 
    else{ 
      boDataRedy = false;  
      *pMsg = NULL;
      uiError = NO_ERROR;
    } 
  }
  // ��� ������ ������
  else uiError = ERROR_DC_TCP;

  return uiError;
}
//----------------------------------------------------------------------------

//---------------------------------------------------------------------------
// !!! ������ ��� ������ ����� �������� ������ �� �� !!!
//---------------------------------------------------------------------------
__task void Task_DC(void) 
{
  U32 TIME_MAX_NOOP = 120000;  // � ����. 2 ������
  U32 TIME_MAX_LOGIN = 10000;  // � ����. 10 ���

  U32 uiError;
  // ����� ���������� Task_DC � ��������� STATE_LOGIN
  static U32 uiTimeLOGIN;

  for(;;)
  {
    os_dly_wait(20);   // 200 ��.

    // �������� ����� � ������� ����������� � ������� ��� � �����.
    PacketQueue.GetNewWayPacket();
    uiError = PacketQueue.InsertToArchiv();

    // ��������� �����
    uiError = DC.ReceiveData();
    if(NO_ERROR != uiError){
     // ���-�� ���� ������
    }

    // ��� ��� ����������
    const U32 MASK = STATEGSM_GPRSMISSING | STATEGSM_TCPMISSING;
    if(0 != (DC.uiStateGSM & MASK)){
      DC.SetStateLINK();
      // �������� ��������� "��� ��� ����������"
      StateHandler->ResetTCPStatus();      
    }

    // ��� ���������� �����������
    else
    {
      // �������� ��������� "���� ��� ����������"
      StateHandler->SetTCPStatus();    
      switch(DC.ucStateTask)
      {
        case STATE_LINK: 
            // ������� ������ ������� ���������� � ��������� STATE_LOGIN
            uiTimeLOGIN = DiffTime(0);
            uiError = DC.Link();         
            break;
    
        case STATE_LOGIN:
            // ����� �������� �������
            if(TIME_MAX_LOGIN < DiffTime(uiTimeLOGIN)) DC.SetStateLINK();
            // ������ �������� �� �������
            else {
              // � ��� ������ ���� ������
              if(DC.boDataRedy){
                C_Msg* pMsg;
                uiError = DC.GetPacketFromTCPBufer(&pMsg);
                if(NO_ERROR == uiError) {
                  // ���� � ������ ��� ������� ������, �� pMsg = 0
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
            // ����� �������� �������
            if(TIME_MAX_NOOP < DiffTime(DC.uiTimeNOOP)) DC.SetStateLINK();
            // ����� �������� �� �������
            else{
              // � ��� ������ ���� ������
              if(DC.boDataRedy){
                C_Msg* pMsg;
                uiError = DC.GetPacketFromTCPBufer(&pMsg);
                if(NO_ERROR == uiError) {
                  // ���� � ������ ��� ������� ������, �� pMsg = 0
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

              // ������� ����� ������ � ������ �� ������ � �������
              uiError = PacketQueue.InsertToQueue(1);
              if(NO_ERROR != uiError) {

              }

              uiError = PacketQueue.InsertToQueueFromArchiv();
              if(NO_ERROR != uiError) {

              }
          
              // ��������� � �� ������ � ������� �����������
              uiError = PacketQueue.Queue_Handler(); 
              if(NO_ERROR == uiError) {

              }
            }
            break;
    
        // �� ������ ������
        default:
            DC.SetStateLINK();
            break;
      } // ����� switch(DC.ucStateTask)
    }  // ����� ����� "��� ���������� �����������"
  }
}
//----------------------------------------------------------------------------

//     if(0 == uiCount % 64){
//        ErrorHandler.ErrorToList((ID_DC << 24) + (5 << 16) + uiCount/4);
//     }
//    if(NO_ERROR != uiError)
//      ErrorHandler.ErrorToList((ID_DC << 24) + (1 << 16) + uiError);
