#ifndef TIMEKEEPER_H_
#define TIMEKEEPER_H_

void TimekeeperInit(void);
int64_t fastGetTimeUnixUsec(void);
systime_t GetTimeInterval(systime_t *last);
Thread* date_clicmd(int argc, const char * const * argv, SerialDriver *sdp);
void exti_pps_cb(EXTDriver *extp, expchannel_t channel);

#endif /* TIMEKEEPER_H_ */
