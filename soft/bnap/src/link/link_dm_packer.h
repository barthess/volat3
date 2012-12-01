#ifndef LINK_DM_PACKER_H_
#define LINK_DM_PACKER_H_

#define dm_sdWrite(sdp, bp, n)  {sdWrite((sdp), (bp), (n));}
void DmPackCycle(SerialDriver *sdp);

#endif /* LINK_DM_PACKER_H_ */
