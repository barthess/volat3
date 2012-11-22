#ifndef LINK_DM_PACKER_H_
#define LINK_DM_PACKER_H_

#define heartbeat_sendperiod              MS2ST(1)
#define gps_raw_int_sendperiod            MS2ST(1)
#define global_position_int_sendperiod    MS2ST(1)
#define system_time_sendperiod            MS2ST(1)
#define mpiovd_sensors_sendperiod         MS2ST(1)
#define sys_status_sendperiod             MS2ST(1)
#define statustext_sendperiod             MS2ST(1)

#define dm_sdWrite(sdp, bp, n)  {sdWrite((sdp), (bp), (n));}

void DmPackCycle(SerialDriver *sdp);

#endif /* LINK_DM_PACKER_H_ */
