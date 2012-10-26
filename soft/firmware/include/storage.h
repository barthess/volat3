#ifndef _STORAGE_H_
#define _STORAGE_H_

#include "cli.h"

void StorageInit(void);
Thread* storage_clicmd(int argc, const char * const * argv, SerialDriver *sdp);


#endif /* _STORAGE_H_ */
