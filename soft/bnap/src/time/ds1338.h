#ifndef DS1338_H_
#define DS1338_H_

int64_t ds1338GetTimeUnixUsec(void);
time_t ds1338GetTimeUnixSec(void);
void ds1338GetTimeTm(struct tm *timp);
void ds1338_set_time(struct tm *timp);
void ds1338Init(void);

#endif /* DS1338_H_ */
