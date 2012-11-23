#ifndef UTILS_H_
#define UTILS_H_


#define putinrange(v, vmin, vmax){                                            \
  chDbgCheck(vmin <= vmax, "");                                               \
  if (v <= vmin)                                                              \
    v = vmin;                                                                 \
  else if (v >= vmax)                                                         \
    v = vmax;                                                                 \
}


#include "memfunc.h"
#include "softdebounce.h"

#endif /* UTILS_H_ */
