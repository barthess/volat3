///===========================================================================
/// Драйвер 5-ти кнопочной клавиатуры   
/// При изменении состояния клавиатуры формируется сообщение с новым состоянием
/// состоянием и помещается в MailBox терминала  
/// Состояние клавиши описывается двумя разрядами переменной типа U32. 
/// Максимальное число клавиш - 16
//============================================================================
// Семенчик В.Г. 01.10.2009                                                  
//============================================================================

#include <AT91SAM7A3.H>
#include <RTL.h>
#include "errorhandler.h"
#include "kbd.h"
//----------------------------------------------------------------------------

// Количество клавиш в системе задается константой KBD_KEY_NUMBER. Максимальное
// число клавиш - 16. Клавиши могут быть подключены как к PIOA так и к PIOB
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

// Задает выводы, которые используются для подключения клавиатуры.  Максимум 16 выводов
//#define KBD_MASKA (KBDA_0 | KBDA_1 | KBDA_2 | KBDA_3 | KBDA_4)
#define KBD_MASKB (KBDB_0 | KBDB_1 | KBDB_2 | KBDB_3 | KBDB_4)
//----------------------------------------------------------------------------

C_KBD KBD;
OS_TID idtKBD;
//----------------------------------------------------------------------------

C_KBD::C_KBD(void) : uiState(0x3FF)
{
//  *AT91C_PIOA_PER = KBD_MASKA;  // Назначаю выводы для подключения клавиатуры
//  *AT91C_PIOA_ODR = KBD_MASKA;  // Определяю их ка входы
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
//    // Формирую слово состояния клавиатуры
//    KBD.uiState = (KBD.uiState << 1) & 0xAAAAAAAA; // Предыдущее состояние сохраняю в нечетных разрядах                                                 // Четные разряды подготавливаю для приема текущего состояния
//    
//    U32 uiMask = 1;   
//    // Тестирую клавиши подключенные к PIOA     
//    U32 uiInput = *AT91C_PIOA_PDSR;
//    KBD.KBDTest(uiInput, uiMask, KBD_MASKA); 
//    // Тестирую клавиши подключенные к PIOB 
//    uiInput = *AT91C_PIOB_PDSR;
//    KBD.KBDTest(uiInput, uiMask, KBD_MASKB);  
//
//    if(uiOldState == KBD.uiState) continue;
//     uiOldState = KBD.uiState;
//
//    // Выделяю память для сообщения
//    S_Msg4 * pMsg = (S_Msg4 *)AloocMsg(PoolMsg4, ERROR_FLAG_KBD);
//    if(NULL == pMsg) return;    // Переполнен пул сообщений
//    // Формирую и отправляю сообщение терминалу
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
  while(0 != uiInputMask){               // Опрашиваю все разряды входного слова
    if(0 != (uiInputMask & uiKBDMask)){  // Обрабатываю только те разряды, которые отображают состояние клавиш
      if(0 != (uiInputMask & uiInput))   // Если клавиша нажата, то устанавливаю 1 в соответствующем разряде
        KBD.uiState = KBD.uiState | uiKeyMask;
      uiKeyMask = uiKeyMask << 2;        // Сдвигаю на 2 бита так как каждой клавише соответствует 2 разряда
    }
    uiInputMask = uiInputMask << 1;      // Подготавлива маску к тестированию следующего бита входного слова
  }
}
//----------------------------------------------------------------------------

