#ifndef LINK_CC_H_
#define LINK_CC_H_

/* special character for data flow control */
#define ETX  3  /* End of text */
#define DLE  16 /*  Data link escape */

void link_cc_up(SerialDriver *sdp);
bool_t cc_port_ready(void);

#endif /* LINK_CC_H_ */
