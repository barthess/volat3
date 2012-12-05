#ifndef LINK_H_
#define LINK_H_


void LinkInit(void);
bool_t traffic_limiter(systime_t *last, const systime_t *period);

void acquire_cc_out(void);
void release_cc_out(void);
void acquire_cc_in(void);
void release_cc_in(void);
void acquire_dm_out(void);
void release_dm_out(void);
void acquire_dm_in(void);
void release_dm_in(void);

#endif /* LINK_H_ */
