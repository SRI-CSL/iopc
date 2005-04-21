#include "cheaders.h"
#include "constants.h"
#include "msg.h"
#include "actor.h"
#include "dbugflags.h"
#include "sal_lib.h"


int dead_sal = 0;

msg* readSALMsg(int fd){
  fd_set rfds;
  struct timeval tv;
  int bytes = 0, finished = 0;
  char buff[MSG_BUFFSZ];
  msg* retval = makeMsg(MSG_BUFFSZ);
  
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

    if(dead_sal){
      if(select(fd+1, &rfds, NULL, NULL, &tv) == -1){
	perror("select error:");
	goto fail;
      } else if(!FD_ISSET(fd, &rfds)){
	if (SAL_ACTOR_DEBUG)fprintf(stderr,"No data within 2 seconds\n");
	return retval;
      }
      finished = 1;
    } /* dead_sal */

    if((bytes = read(fd, buff, MSG_BUFFSZ)) < 0){
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
  char buff[MSG_BUFFSZ];
  int bytesRead = 0;
  bytesRead = read(from,buff,MSG_BUFFSZ);
  if(bytesRead > 0){
    write(to,buff,bytesRead);
  }
}

void *echoSALErrors(void *arg){
  int fd;
  sigset_t mask;
  if(arg == NULL){
    fprintf(stderr, "Bad arg to echoErrors\n");
    return NULL;
  }
  fd = *((int *)arg);
  if((sigemptyset(&mask) != 0) && (sigaddset(&mask, SIGCHLD) != 0)){
    fprintf(stderr, "futzing with sigsets failed in echoErrors\n");
  }
  if(pthread_sigmask(SIG_BLOCK, &mask, NULL) != 0){
    fprintf(stderr, "pthread_sigmask failed in echoErrors\n");
  }
  while(!dead_sal)
    echoSAL(fd, STDERR_FILENO);
  return NULL;
}

