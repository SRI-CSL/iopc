#include "cheaders.h"
#include "constants.h"
#include "msg.h"
#include "actor.h"
#include "dbugflags.h"
#include "sal_lib.h"

/* This is Ian's version */

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

/* This is buggy, see comment in code */

msg* readSALMsgRomulus(fdBundle* fdB){
  fd_set rfds;
  struct timeval tv;
  int bytes = 0, finished = 0;
  char buff[BUFFSZ];
  msg* retval = makeMsg(BUFFSZ);
  int fd = fdB->fd;

  FD_ZERO(&rfds);
  FD_SET(fd, &rfds);
  
  tv.tv_sec = 2;
  tv.tv_usec = 0;
  
  if(retval == NULL){
    fprintf(stderr, "makeMsg in %d failed\n", getpid());
    goto fail;
  }
  while(1){
  restart: 

    /* The previous select call can change both tv and rfds */

    if (SAL_ACTOR_DEBUG)fprintf(stderr, "readSALMsg blocking on read.\n");
    if(*(fdB->exit)){
      if(1)fprintf(stderr, "sal is dead, blocking on select\n");
      if(select(fd+1, &rfds, NULL, NULL, &tv) == -1){
	perror("select error:");
	goto fail;
      }
      if(!FD_ISSET(fd, &rfds)){
	if (SAL_ACTOR_DEBUG)fprintf(stderr,"No data within 2 seconds\n");
	return retval;
      }
      finished = 1;
    } /* *(fdB->exit) */

    /* WHAT HAPPENS IF THE SIGCHLD ARRIVES HERE?? */

    if((bytes = read(fd, buff, BUFFSZ)) < 0){
      if(errno == EINTR){
	if(1)fprintf(stderr, "interrupting the read\n");
	if (SAL_ACTOR_DEBUG)
	  fprintf(stderr,"readSALMsg  in %d restarting after being interrupted by a signal\n", getpid());
	goto restart;
      }
      
      if(errno == EBADF){
	fprintf(stderr, "readSALMsg  in %d failing because of a bad file descriptor\n", getpid());
	goto fail;
      }
      fprintf(stderr, "Read  in %d returned with nothing\n", getpid());
      return retval;
    } /* read failed */
    if (SAL_ACTOR_DEBUG)fprintf(stderr, "readSALMsg read %d bytes\n", bytes);
    
    if(bytes > 0){
     
      if(addToMsg(retval, bytes, buff) != 0){
	if (SAL_ACTOR_DEBUG)
	  fprintf(stderr, "addToMsg in %d failed\n", getpid());
	goto fail;
      }
    }
    if(finished){ break; }
  }
  return retval;
 fail:
  return NULL;
}

