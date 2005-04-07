#include "types.h"

#define SAL_ACTOR_DEBUG 0
msg* readSALMsg(int fd);
int writeSALMsg(int fd, msg* m);
