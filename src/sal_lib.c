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

int sendSALFormattedMsgFD(int fd, char* fmt, ...){
  char *p, *sval;
  int ival;
  double dval;
  char numbuff[16];
  va_list ap;
  msg* m = makeMsg(MSG_BUFFSZ);
  char * bytesSend = NULL;
  msg * tmp = NULL;
  int length;
  
  if(m == NULL) return -1;
  va_start(ap, fmt);
  for(p = fmt; *p; p++){
    if(*p != '%'){
      addToMsg(m, 1, p);
      continue;
    }
    switch(*++p){
    case 's':
      sval = va_arg(ap, char *);
      addToMsg(m, strlen(sval), sval);
      break;
    case 'd':
      ival = va_arg(ap, int);
      sprintf(numbuff, "%d", ival);
      addToMsg(m, strlen(numbuff), numbuff);
      break;
    case 'f':
      ival = va_arg(ap, double);
      sprintf(numbuff, "%f", dval);
      addToMsg(m, strlen(numbuff), numbuff);
      break;
    default:
      addToMsg(m, 1, p);
      break;
    }
  }
  va_end(ap);
  m->data[m->bytesUsed] = '\0';
  
  if (bytesSend != NULL) {
    free(bytesSend);
    bytesSend = NULL;
  }
  bytesSend = (char *)calloc(SIZE,sizeof(char));
  if (bytesSend == NULL){
    fprintf(stderr,"\nCalloc failed in SalLib for bytesSend \n");
    return -1;
  }
  sprintf(bytesSend,"%d",m->bytesUsed);
  fprintf(stderr,"\n Bytes used: %d , Bytes Send: %s\n",m->bytesUsed,bytesSend);
 
	     
  if (tmp != NULL){
    freeMsg(tmp);
    tmp = NULL;
  }
  tmp = makeMsg(MSG_BUFFSZ);
  if(tmp == NULL){
    fprintf(stderr, "makeMsg in %d failed(SalLib)\n", getpid());
    return -1;
  }
  if(addToSALMsg(tmp, SIZE, bytesSend) != 0){
    fprintf(stderr, "addToSALMsg in %d failed(Sal Lib)\n", getpid());
    return -1;
  }
  length = parseString(tmp->data, tmp->bytesUsed);
  tmp->bytesUsed = length;
	     
  fprintf(stderr,"\n \n tmp->data is : %s\n\n ",tmp->data);
  /*writeInt(fd, m->bytesUsed);*/
  /*  write(fd, m->data, m->bytesUsed);*/
  
  writeMsg(fd,tmp);
  writeMsg(fd, m);
  return m->bytesUsed;
}




