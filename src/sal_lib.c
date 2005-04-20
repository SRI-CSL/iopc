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


int DEAD_SAL;

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
  
  if(retval == NULL){
    fprintf(stderr, "makeMsg in %d failed\n", getpid());
    goto fail;
  }
  while(1){
  restart: 
    if (SAL_ACTOR_DEBUG)
      fprintf(stderr, "readSALMsg blocking on read.\n");
    if (DEAD_SAL){
      if(  select(fd+1, &rfds, NULL, NULL, &tv) == -1){
	perror("select error:");
      }
      else if (!FD_ISSET(fd,&rfds)){
	if (SAL_ACTOR_DEBUG)
	  fprintf(stderr,"No data within 2 seconds\n");
	goto fail;
      }
    }
    if((bytes = read(fd, buff, MSG_BUFFSZ)) < 0){
      if(errno == EINTR){
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
    }
    if (SAL_ACTOR_DEBUG)
      fprintf(stderr, "readSALMsg read %d bytes\n", bytes);
    if(addToSALMsg(retval, bytes, buff) != 0){
      if (SAL_ACTOR_DEBUG)
	fprintf(stderr, "addToSALMsg in %d failed\n", getpid());
      goto fail;
    }
    else break;
  }
  return retval;
 fail:
  return NULL;
}

int addToSALMsg(msg* m, int bytes, char* buff){
  if((bytes <= 0) || (buff == NULL)){
    if (SAL_ACTOR_DEBUG)
      fprintf(stderr, "Bad arguments to addToSALMsg\n");
    return -1;
  }
  if (SAL_ACTOR_DEBUG){
    fprintf(stderr,"addToSALMsg: bytes = %d m->bytesLeft = %d\n", bytes, m->bytesLeft);
  }
  if(bytes < m->bytesLeft){
    memcpy(&m->data[m->bytesUsed], buff, bytes);
    m->bytesUsed += bytes;
    m->bytesLeft -= bytes;
    m->data[m->bytesUsed] = '\0';
    return 0;
  } else {
    int current, desired, new;
    char *tmp;
    current = m->bytesUsed + m->bytesLeft;
    desired = m->bytesUsed + bytes;
    new = 2 * current;
    while(new < desired) new *= 2;
    tmp = m->data;
    m->data = (char *)calloc(new, sizeof(char));
    if(m->data == NULL){
      fprintf(stderr, "Calloc failed in addToSALMsg\n");
      free(tmp);
      m->bytesUsed = 0;
      m->bytesLeft = 0;
      return -1;
    }
    memcpy(m->data, tmp, m->bytesUsed);
    memcpy(&m->data[m->bytesUsed], buff, bytes);
    m->bytesUsed += bytes;
    m->bytesLeft = new - desired;
    m->data[m->bytesUsed] = '\0';
    free(tmp);
    return 0;
  }
} 




