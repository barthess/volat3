#ifndef MESSAGE_H_
#define MESSAGE_H_

#include <mavlink.h>
#include <common.h>
#include <mpiovd.h>

#include "ch.h"
#include "hal.h"

/**
 * ��������� ���������� ������ ��������� ���� "������" ��� ������ �������.
 */
typedef struct Mail Mail;
struct Mail{
  /**
   * @brief ��������� �� ������� �����.
   * @details ����� ����, ��� ������ ������� �������
   * ��������, ��������� ��������������� � NULL. ��� ����� �������� ���
   * ������ �������.
   */
  void *payload;
  /**
   * ���� ����������� �� ���������� ����������, ����� ��������� ��� ������.
   */
  msg_t invoice;
  /**
   * ��������� �� �������� ����, � ������� ���� �������� �������������
   * ��������� ����������. ����� �������������� ��� ������������� �������������
   * ������. �������� ��������� ������ �� �������� ������.
   * ����� ���� NULL.
   */
  Mailbox *confirmbox;
};


void MsgInit(void);
void MavInit(void);


#endif /* MESSAGE_H_ */

