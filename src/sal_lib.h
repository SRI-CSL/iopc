#include "types.h"

#define MSG_BUFFSZ 1024
#define SAL_ACTOR_DEBUG 0
msg* readSALMsg(int fd);
void *echoSALErrors(void *arg);
