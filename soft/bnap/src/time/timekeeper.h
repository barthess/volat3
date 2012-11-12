#ifndef TIMEKEEPER_H_
#define TIMEKEEPER_H_

void TimekeeperInit(void);
uint64_t pnsGetTimeUnixUsec(void);
systime_t GetTimeInterval(systime_t *last);

#endif /* TIMEKEEPER_H_ */
