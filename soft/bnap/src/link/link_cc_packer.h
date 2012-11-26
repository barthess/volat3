#ifndef LINK_CC_PACKER_H_
#define LINK_CC_PACKER_H_

#include "wavecom.h"

#define cc_sdWrite(sdp, bp, n)  {UdpSdWrite((sdp), (bp), (n));}

void CcPackCycle(SerialDriver *sdp);

#endif /* LINK_CC_PACKER_H_ */
