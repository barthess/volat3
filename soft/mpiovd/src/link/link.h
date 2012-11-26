#ifndef LINK_H_
#define LINK_H_

#include "mavlink.h"

void LinkInit(void);
void KillMavlinkThreads(void);
void SpawnMavlinkThreads(void *sdp);

#endif /* LINK_H_ */
