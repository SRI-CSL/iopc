#include "cheaders.h"
#include "constants.h"
#include "msg.h"
#include "actor.h"
#include "dbugflags.h"

#define MSG_BUFFSZ 1024

msg* readSALMsg(int fd){
  int bytes = 0;
  char buff[MSG_BUFFSZ];
  msg* retval = makeMsg(MSG_BUFFSZ);
  if(retval == NULL){
    fprintf(stderr, "makeMsg in %d failed\n", getpid());
    goto fail;
  }
  while(1){
  restart: 
    /*    fprintf(stderr, "readSALMsg blocking on read.\n");*/
    if((bytes = read(fd, buff, MSG_BUFFSZ)) < 0){
      if(errno == EINTR){
	mannounce("readSALMsg  in %d restarting after being interrupted by a signal\n", getpid());
	goto restart;
      }
      if(errno == EBADF){
	fprintf(stderr, "readSALMsg  in %d failing because of a bad file descriptor\n", getpid());
	goto fail;
      }
      fprintf(stderr, "Read  in %d returned with nothing\n", getpid());
      return retval;
    }
    /*    fprintf(stderr, "readSALMsg read %d bytes\n", bytes);*/
    if(addToMsg(retval, bytes, buff) != 0){
      fprintf(stderr, "addToMsg in %d failed\n", getpid());
      goto fail;
    }
    else break;
  }
  return retval;
 fail:
  return NULL;
}



