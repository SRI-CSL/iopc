#include "cheaders.h"
#include "constants.h"
#include "msg.h"
#include "actor.h"
#include "dbugflags.h"
#include "sal_lib.h"


msg* readSALMsg(fdBundle* fdB){
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
    if (SAL_ACTOR_DEBUG)fprintf(stderr, "readSALMsg blocking on read.\n");

    if(fdB->exit){
      if(select(fd+1, &rfds, NULL, NULL, &tv) == -1){
	perror("select error:");
	goto fail;
      } else if(!FD_ISSET(fd, &rfds)){
	if (SAL_ACTOR_DEBUG)fprintf(stderr,"No data within 2 seconds\n");
	return retval;
      }
      finished = 1;
    } /* child_died */

    if((bytes = read(fd, buff, BUFFSZ)) < 0){
      if(errno == EINTR){
	if (SAL_ACTOR_DEBUG)fprintf(stderr,"readSALMsg  in %d restarting after being interrupted by a signal\n", getpid());
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

void echoSAL(int from, int to){
  char buff[BUFFSZ];
  int bytesRead = 0;
  bytesRead = read(from, buff,BUFFSZ);
  if(bytesRead > 0){
    write(to,buff,bytesRead);
  }
}


