#include "cheaders.h"
#include "constants.h"
#include "msg.h"
#include "actor.h"
#include "dbugflags.h"
#include "sal_lib.h"


msg* readSALMsg(fdBundle* fdB){
  fd_set set;
  struct timeval tv;
  int bytes = 0;
  char buff[BUFFSZ];
  msg* retval = makeMsg(BUFFSZ);
  int errcode;
  int fd = fdB->fd;

  if(retval == NULL){
    fprintf(stderr, "makeMsg in %d failed\n", getpid());
    goto fail;
  }
  
  while(1){
  restart: 
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    FD_ZERO(&set); 
    FD_SET(fd, &set); 
    errcode = select(fd + 1, &set, NULL, NULL, &tv);
    if(errcode == -1){
      if(errno == EINTR){ continue; }
      perror("select error:");
      goto fail;
    }
    if(FD_ISSET(fd, &set)){
      if((bytes = read(fd, buff, BUFFSZ)) < 0){
	if(errno == EINTR){ 
	  goto restart; 
	} else { 
	  fprintf(stderr, "read in readSALMsg failed\n");
	  goto fail;
	}
      }
      if(bytes > 0){
	if(addToMsg(retval, bytes, buff) != 0){
	  goto fail;
	}
      } else {
	/* no bytes there      */
	if(*(fdB->exit)){ break; } else { continue; }
      }
    } else {
      /* nothing to read of fd */
      if(*(fdB->exit)){ break; } else { continue; }
    }
  }

  return retval;
  
 fail:
  return NULL;
}

