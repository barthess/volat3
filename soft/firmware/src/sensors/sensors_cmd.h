#ifndef SENSORS_CMD_H_
#define SENSORS_CMD_H_

#include "cli.h"


Thread* sensors_clicmd(int argc, const char * const * argv, SerialDriver *sdp);


#endif /* SENSORS_CMD_H_ */
