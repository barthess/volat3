#ifndef LINK_DM_PACKER_H_
#define LINK_DM_PACKER_H_

#include "ch.h"
#include "hal.h"
#include "mavlink.h"

#include "link.h"
#include "message.h"
#include "main.h"

void DmPackCycle(SerialDriver *sdp);

#endif /* LINK_DM_PACKER_H_ */
