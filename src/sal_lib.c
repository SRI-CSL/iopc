#include "cheaders.h"
#include "constants.h"
#include "msg.h"
#include "actor.h"
#include "dbugflags.h"
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include "sal_lib.h"

#define MSG_BUFFSZ 1024

msg* readSALMsg(int fd){
  fd_set rfds;
  struct timeval tv;
  int bytes = 0;
  char buff[MSG_BUFFSZ];
  msg* retval = makeMsg(MSG_BUFFSZ);
  
  FD_ZERO(&rfds);
  FD_SET(fd, &rfds);
  
  tv.tv_sec = 2;
  tv.tv_usec = 0;
  if(  select(fd+1, &rfds, NULL, NULL, &tv) == -1){
    perror("select error:");
  }
  else if (!FD_ISSET(fd,&rfds)){
    if (SAL_ACTOR_DEBUG)
      fprintf(stderr,"No data within 2 seconds\n");
    goto fail;
  }
  if(retval == NULL){
    fprintf(stderr, "makeMsg in %d failed\n", getpid());
    goto fail;
  }
  while(1){
  restart: 
    if (SAL_ACTOR_DEBUG)
      fprintf(stderr, "readSALMsg blocking on read.\n");
    
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
    if (SAL_ACTOR_DEBUG)
      fprintf(stderr, "readSALMsg read %d bytes\n", bytes);
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



