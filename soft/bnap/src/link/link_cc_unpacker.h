#ifndef LINK_CC_UNPACKER_H_
#define LINK_CC_UNPACKER_H_

#include "ch.h"
#include "hal.h"
#include "mavlink.h"

#include "link.h"
#include "message.h"
#include "main.h"

void CcUnpackCycle(SerialDriver *sdp);

#endif /* LINK_CC_UNPACKER_H_ */
