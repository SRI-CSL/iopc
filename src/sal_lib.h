#include "types.h"

/*used by the sal actor to read the sal message*/
msg* readSALMsg(fdBundle* fdB);
/*used by the salenv wrapper to parse and echo the salenv message */
void parseSalThenEcho(int from, int to);
/*used by the salenv wrapper to read the salenv message */
msg* wrapper_readSalMsg(int fd);
