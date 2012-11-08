#ifndef MESSAGE_H_
#define MESSAGE_H_

#include <mavlink.h>
#include <common.h>
#include <mpiovd.h>

#include "ch.h"
#include "hal.h"

/**
 * структура определяет формат сообщений типа "письмо" для обмена данными.
 */
typedef struct Mail Mail;
struct Mail{
  /**
   * @brief указатель на внешний буфер.
   * @details После того, как данные забраны берущей
   * стороной, указатель устанавливается в NULL. Это будет сигналом для
   * дающей стороны.
   */
  void *payload;
  /**
   * Поле заполняется по усмотрению приложения, может содержать что угодно.
   */
  msg_t invoice;
  /**
   * Указатель на почтовый ящик, в который надо прислать подтверждение
   * успешного выполнения. Может использоваться для синхронизации отправляющего
   * потока. Аналогия обратного адреса на конверте письма.
   * Может быть NULL.
   */
  Mailbox *confirmbox;
};


void MsgInit(void);
void MavInit(void);


#endif /* MESSAGE_H_ */
