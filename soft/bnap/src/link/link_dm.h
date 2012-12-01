#ifndef LINK_DM_H_
#define LINK_DM_H_

void spawn_dm_threads(SerialDriver *sdp);
void kill_dm_threads(void);
bool_t dm_port_ready(void);
void acquire_dm_out(void);
void release_dm_out(void);

#endif /* LINK_DM_H_ */
