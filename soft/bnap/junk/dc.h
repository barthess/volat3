#ifndef DC_H
#define DC_H
//===========================================================================
// Обработка команд и подтверждений диспетчерского центра                                   
//============================================================================

#include <RTL.h>
#include "SysConfig.h"
#include "statehandler.h"
#include "tcpbuf.h"
#include "packets_dc.h"
#include "dc.h"
//----------------------------------------------------------------------------

class C_DC
{
  friend __task void Task_DC(void); 
private:
  // Слово состояние модема
  U32 uiStateGSM;            
  // Массив для временного хранения данных
  char pHEXData[2*BLOKSIZE];
  // Слово состояние задачи
  U8   ucStateTask;  
  // Данные помещены в ТСР буфер          
  bool boDataRedy;            
  // Время после последнего приема пакета PACKET_NOOP
  U32 uiTimeNOOP;
                            
  C_Msg* BuildCMD_ReceiveData(void);
  C_Msg* BuildCMD_SendData(const char* pData, U16 usSize);
  U32  GetPacketFromTCPBufer(C_Msg** pMsg);
  U32  GetResponse(void);
  U32  GetStateAndDataFromArg(char* szResponse);
  U32  ReceiveData(void);
  U32  SendData(const char* pData, U16 usSize);
  U32  SetDataToBuf(char* pArg);
  void SetStateLINK(void); 

  // Обработка принятых пакетов
  U32  CMDforLCD(C_Msg* pMsg);
  U32  Link(void);
  void Login(C_Msg* pMsg);
  U32  MasageForTerminal(C_Msg* pMsg);
  U32  NOOPPacketHandler(void);
  U32  SetPollingConfig(C_Msg* pMsg);
  U32  TextForLCD(C_Msg* pMsg);  
  U32  WayPointConfirm(C_Msg* pMsg);
  U32  PacketHandler(C_Msg* pMsg);
public:
  os_mbx_declare(MailBox, DC_MAILBOX_SIZE); 
  C_TCPBuf TCPBufer;

  C_DC() : uiStateGSM(0x500), ucStateTask(0), boDataRedy(false), uiTimeNOOP(0)
                                                {os_mbx_init(MailBox, sizeof(MailBox));}
  U32 SendPacketToDC(const char *pData, U16 usSize);
  void TCPbufClear(void){ TCPBufer.Clear(); boDataRedy = false; }

};
//----------------------------------------------------------------------------

extern C_DC DC;
extern OS_TID idtDC;
__task void Task_DC(void);
//----------------------------------------------------------------------------

#endif //#ifndef DC_H
