#ifndef _STORAGE_H_
#define _STORAGE_H_

#include "cli.h"

void StorageInit(void);
Thread* storage_cmd(int argc, const char * const * argv, const ShellCmd_t *cmdarray);


#endif /* _STORAGE_H_ */
