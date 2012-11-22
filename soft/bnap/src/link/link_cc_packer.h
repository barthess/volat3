#ifndef LINK_CC_PACKER_H_
#define LINK_CC_PACKER_H_

#define heartbeat_sendperiod              MS2ST(950)
#define gps_raw_int_sendperiod            MS2ST(10000)
#define global_position_int_sendperiod    MS2ST(950)
#define system_time_sendperiod            MS2ST(10000)
#define mpiovd_sensors_sendperiod         MS2ST(500)
#define sys_status_sendperiod             MS2ST(950)
#define statustext_sendperiod             MS2ST(1)

void CcPackCycle(SerialDriver *sdp);

#endif /* LINK_CC_PACKER_H_ */
