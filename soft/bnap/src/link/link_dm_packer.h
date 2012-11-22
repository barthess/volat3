#ifndef LINK_DM_PACKER_H_
#define LINK_DM_PACKER_H_

#define heartbeat_sendperiod              MS2ST(1000)
#define gps_raw_int_sendperiod            MS2ST(1000)
#define global_position_int_sendperiod    MS2ST(1000)
#define system_time_sendperiod            MS2ST(1000)
#define mpiovd_sensors_sendperiod         MS2ST(1000)
#define sys_status_sendperiod             MS2ST(1000)
#define statustext_sendperiod             MS2ST(1000)

void DmPackCycle(SerialDriver *sdp);

#endif /* LINK_DM_PACKER_H_ */
