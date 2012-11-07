#ifndef TIMEKEEPING_H_
#define TIMEKEEPING_H_

void TimekeepingInit(void);
uint64_t pnsGetTimeUnixUsec(void);
systime_t GetTimeInterval(systime_t *last);

#endif /* TIMEKEEPING_H_ */
