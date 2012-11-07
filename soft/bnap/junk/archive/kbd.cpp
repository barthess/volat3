///===========================================================================
/// ������� 5-�� ��������� ����������   
/// ��� ��������� ��������� ���������� ����������� ��������� � ����� ����������
/// ���������� � ���������� � MailBox ���������  
/// ��������� ������� ����������� ����� ��������� ���������� ���� U32. 
/// ������������ ����� ������ - 16
//============================================================================
// �������� �.�. 01.10.2009                                                  
//============================================================================

#include <AT91SAM7A3.H>
#include <RTL.h>
#include "errorhandler.h"
#include "kbd.h"
//----------------------------------------------------------------------------

// ���������� ������ � ������� �������� ���������� KBD_KEY_NUMBER. ������������
// ����� ������ - 16. ������� ����� ���� ���������� ��� � PIOA ��� � � PIOB
#define KBDA_0 0
#define KBDA_1 0
#define KBDA_2 0
#define KBDA_3 0
#define KBDA_4 0
#define KBDB_0 AT91C_PIO_PB21
#define KBDB_1 AT91C_PIO_PB22
#define KBDB_2 AT91C_PIO_PB23
#define KBDB_3 AT91C_PIO_PB24
#define KBDB_4 AT91C_PIO_PB25

// ������ ������, ������� ������������ ��� ����������� ����������.  �������� 16 �������
//#define KBD_MASKA (KBDA_0 | KBDA_1 | KBDA_2 | KBDA_3 | KBDA_4)
#define KBD_MASKB (KBDB_0 | KBDB_1 | KBDB_2 | KBDB_3 | KBDB_4)
//----------------------------------------------------------------------------

C_KBD KBD;
OS_TID idtKBD;
//----------------------------------------------------------------------------

C_KBD::C_KBD(void) : uiState(0x3FF)
{
//  *AT91C_PIOA_PER = KBD_MASKA;  // �������� ������ ��� ����������� ����������
//  *AT91C_PIOA_ODR = KBD_MASKA;  // ��������� �� �� �����
  *AT91C_PIOB_PER = KBD_MASKB;  
  *AT91C_PIOB_ODR = KBD_MASKB;  
//  _init_box(PoolMsg, sizeof (PoolMsg), sizeof(S_KBDMsg));
}
//----------------------------------------------------------------------------

__task void Task_KBD(void)
{
//  U32 uiOldState = 0;
  for(;;){
//    os_dly_wait(KBD_DLY); 
//    // �������� ����� ��������� ����������
//    KBD.uiState = (KBD.uiState << 1) & 0xAAAAAAAA; // ���������� ��������� �������� � �������� ��������                                                 // ������ ������� ������������� ��� ������ �������� ���������
//    
//    U32 uiMask = 1;   
//    // �������� ������� ������������ � PIOA     
//    U32 uiInput = *AT91C_PIOA_PDSR;
//    KBD.KBDTest(uiInput, uiMask, KBD_MASKA); 
//    // �������� ������� ������������ � PIOB 
//    uiInput = *AT91C_PIOB_PDSR;
//    KBD.KBDTest(uiInput, uiMask, KBD_MASKB);  
//
//    if(uiOldState == KBD.uiState) continue;
//     uiOldState = KBD.uiState;
//
//    // ������� ������ ��� ���������
//    S_Msg4 * pMsg = (S_Msg4 *)AloocMsg(PoolMsg4, ERROR_FLAG_KBD);
//    if(NULL == pMsg) return;    // ���������� ��� ���������
//    // �������� � ��������� ��������� ���������
//    pMsg->ucIDSource = ID_MSG_KBD;
//    pMsg->uiLenght   = 4;
//    pMsg->pPoolMsg   = PoolMsg4;
//    pMsg->pMaiBoxSource    = NULL;
//    *((U32 *)(pMsg->str4)) = KBD.uiState;
//    if(OS_R_OK != os_mbx_send(Terminal.MailBox, pMsg, Timeout_KBD_SEND))
//      os_evt_set (ERROR_FLAG_KBD, idtErrorHandler);
  }   
}
//----------------------------------------------------------------------------

void C_KBD::KBDTest(U32 uiInput, U32 &uiKeyMask, U32 uiKBDMask)
{
  U32 uiInputMask = 1;          
  while(0 != uiInputMask){               // ��������� ��� ������� �������� �����
    if(0 != (uiInputMask & uiKBDMask)){  // ����������� ������ �� �������, ������� ���������� ��������� ������
      if(0 != (uiInputMask & uiInput))   // ���� ������� ������, �� ������������ 1 � ��������������� �������
        KBD.uiState = KBD.uiState | uiKeyMask;
      uiKeyMask = uiKeyMask << 2;        // ������� �� 2 ���� ��� ��� ������ ������� ������������� 2 �������
    }
    uiInputMask = uiInputMask << 1;      // ������������ ����� � ������������ ���������� ���� �������� �����
  }
}
//----------------------------------------------------------------------------

