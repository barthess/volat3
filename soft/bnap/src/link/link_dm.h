#ifndef LINK_DM_H_
#define LINK_DM_H_

void spawn_dm_threads(SerialDriver *sdp);
void kill_dm_threads(void);
bool_t dm_port_ready(void);

#endif /* LINK_DM_H_ */
