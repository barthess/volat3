#include <mavlink.h>
#include <common.h>


#ifndef LINK_H_
#define LINK_H_

void LinkInit(void);
void KillMavlinkThreads(void);
void SpawnMavlinkThreads(void *sdp);

#endif /* LINK_H_ */
