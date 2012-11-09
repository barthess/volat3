#ifndef LINK_DM_PACKER_H_
#define LINK_DM_PACKER_H_

#include "ch.h"
#include "hal.h"
#include "mavlink.h"

#include "link.h"
#include "message.h"
#include "main.h"

uint16_t sort_output_mail(Mail *mailp, mavlink_message_t *mavlink_msgbuf);

#endif /* LINK_DM_PACKER_H_ */
