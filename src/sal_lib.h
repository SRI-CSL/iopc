#include "types.h"

msg* readSALMsg(fdBundle* fdB);
void parseSalThenEcho(int from, int to);
msg* wrapper_readSalMsg(int fd);
void echo2Sal(int from, int to);
