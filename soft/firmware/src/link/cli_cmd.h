#ifndef CLI_CMD_H_
#define CLI_CMD_H_

#include "cli.h"


Thread* ps_clicmd(int argc, const char * const * argv, SerialDriver *sdp);
Thread* uname_clicmd(int argc, const char * const * argv, SerialDriver *sdp);
Thread* help_clicmd(int argc, const char * const * argv, SerialDriver *sdp);
Thread* clear_clicmd(int argc, const char * const * argv, SerialDriver *sdp);
Thread* list_clicmd(int argc, const char * const * argv, SerialDriver *sdp);
Thread* loop_cmd(int argc, const char * const * argv, SerialDriver *sdp);
Thread* reboot_cmd(int argc, const char * const * argv, SerialDriver *sdp);
Thread* sleep_cmd(int argc, const char * const * argv, SerialDriver *sdp);
Thread* selftest_clicmd(int argc, const char * const * argv, SerialDriver *sdp);
Thread* sensor_cmd(int argc, const char * const * argv, SerialDriver *sdp);


#endif /* CLI_CMD_H_ */
