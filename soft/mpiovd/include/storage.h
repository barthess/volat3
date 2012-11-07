#ifndef _STORAGE_H_
#define _STORAGE_H_

#include "cli.h"

void StorageInit(void);
Thread* storage_clicmd(int argc, const char * const * argv, SerialDriver *sdp);

void update_trip(uint32_t delta);
uint32_t GetUptime(void);
uint32_t GetTrip(void);

#endif /* _STORAGE_H_ */
