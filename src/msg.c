/*
    The InterOperability Platform: IOP
    Copyright (C) 2004 Ian A. Mason
    School of Mathematics, Statistics, and Computer Science   
    University of New England, Armidale, NSW 2351, Australia
    iam@turing.une.edu.au           Phone:  +61 (0)2 6773 2327 
    http://mcs.une.edu.au/~iam/     Fax:    +61 (0)2 6773 3312 


    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "cheaders.h"
#include "constants.h"
#include "types.h"
#include "msg.h"
#include "actor.h"
#include "dbugflags.h"

#define MSG_BUFFSZ 1024

/* statics */
static int setFlag(int fd, int flags);
static int clearFlag(int fd, int flags);

static pthread_mutex_t iop_err_mutex = PTHREAD_MUTEX_INITIALIZER;

void mannounce(const char *format, ...){
  va_list arg;
  va_start(arg, format);
  if(format == NULL){
    va_end(arg);
  } else {
    if(MSG_DEBUG){
      pthread_mutex_lock(&iop_err_mutex);
      fprintf(stderr, "MSG(%ld)\t:\t", (long)pthread_self());
      vfprintf(stderr, format, arg);
      pthread_mutex_unlock(&iop_err_mutex);
    }
    va_end(arg);
  }
  return;
}

msg* makeMsg(int bytes){
  msg* retval = (msg *)calloc(1, sizeof(msg));
  if(retval == NULL)  goto fail;
  retval->data = (char *)calloc(bytes, sizeof(char));
  if(retval->data == NULL) goto fail;
  retval->bytesUsed = 0;
  retval->bytesLeft = bytes;
  return retval;
 fail:
  fprintf(stderr, "makeMsg failed\n");
  free(retval);
  return NULL;
}

void freeMsg(msg* m){
  if(m == NULL) return;
  free(m->data);
  free(m);
}

int addToMsg(msg* m, int bytes, char* buff){
  if((bytes <= 0) || (buff == NULL)){
    fprintf(stderr, "Bad arguments to addToMsg\n");
    return -1;
  }
  mannounce("addToMsg: bytes = %d m->bytesLeft = %d\n", bytes, m->bytesLeft);
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
      fprintf(stderr, "Calloc failed in addToMsg\n");
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

void echo(int from, int to){
  msg* message;
  message = readMsg(from);
  if(message != NULL){
    writeMsg(to, message);
    if(MSG_DEBUG){
      writeMsg(STDERR_FILENO, message);
      mannounce("echo wrote %d bytes\n", message->bytesUsed);
    }
    freeMsg(message);
  }
}

int setFlag(int fd, int flags){
  int val;
  if((val = fcntl(fd, F_GETFL, 0)) < 0){
    perror("fcntl(fd, F_GETFL, 0) failed");
    fprintf(stderr, "pid = %d fd = %d\n", getpid(), fd);
    return -1;
  }
  val |= flags;
  if(fcntl(fd, F_SETFL, val) < 0){
    perror("fcntl(fd, F F_SETFL, val) failed");
    return -1;
  }
  return 0;
}

int clearFlag(int fd, int flags){
  int val;
  if((val = fcntl(fd, F_GETFL, 0)) < 0){
    perror("fcntl(fd, F_GETFL, 0) failed");
    fprintf(stderr, "pid = %d fd = %d\n", getpid(), fd);
    return -1;
  }
  val &= ~flags;
  if(fcntl(fd, F_SETFL, val) < 0){
    mannounce("fcntl(fd, F F_SETFL, val) failed: %s\n", strerror(errno));
    return -1;
  }
  return 0;
}

msg* readMaudeMsg(int fd){
  char prompt[] = "Maude> ";
  char *promptPointer = NULL;
  int bytes = 0, iteration = 0;
  msg* retval = makeMsg(MSG_BUFFSZ);
  if(retval == NULL){
    fprintf(stderr, "makeMsg in %d failed\n", getpid());
    goto fail;
  }

  while(1){
    char buff[MSG_BUFFSZ];
    mannounce("readMaudeMsg\t:\tcommencing a read\n");
  restart:
    if((bytes = read(fd, buff, MSG_BUFFSZ)) < 0){
      mannounce("readMaudeMsg\t:\tread error read returned %d bytes\n", bytes);
      if(errno == EINTR){
	mannounce("readMsg  in %d restarting after being interrupted by a signal\n", getpid());
	goto restart;
      }
      if(errno == EBADF){
	fprintf(stderr, "readMsg  in %d failing because of a bad file descriptor\n", getpid());
	goto fail;
      }
      fprintf(stderr, "Read  in %d returned with nothing\n", getpid());
      return retval;
    } /* if((bytes = read(fd, buff, MSG_BUFFSZ)) < 0) */

    mannounce("readMaudeMsg\t:\tread read %d bytes\n", bytes);

    /*
    if((iteration == 0) && 
       (bytes == 2) &&
       (strcmp(buff, "> ") == 0)){
      fprintf(stderr, "Warning: maude is in multiline mode\n");
      goto fail;
    }
    */

    if(addToMsg(retval, bytes, buff) != 0){
      fprintf(stderr, "addToMsg in %d failed\n", getpid());
      goto fail;
    }

    iteration++;

    if((promptPointer = strstr(retval->data, prompt)) != NULL){
      fd_set readset;
      struct timeval delay;
      int sret;
      mannounce("readMaudeMsg\t:\tsaw the prompt, making sure!\n");
      FD_ZERO(&readset);
      FD_SET(fd, &readset);
      delay.tv_sec = 0;
      delay.tv_usec = 0;
      sret = select(fd + 1, &readset, NULL, NULL, &delay);
      if(sret < 0){
	fprintf(stderr, "readMaudeMsg\t:\tselect error\n");
	goto fail;
      } else if(sret == 0){
	mannounce("readMaudeMsg\t:\tdefinitely the prompt!\n");
	break;
      } else {
	mannounce("readMaudeMsg\t:\tsret = %d more coming! TOO CONFUSING\n", sret);
	goto fail;
      }
    }

  }/* while */

  if(retval != NULL){
    if(MSG_DEBUG){
      mannounce("readMaudeMsg\t:\tretval->bytesUsed = %d\n", retval->bytesUsed);
      mannounce("readMaudeMsg\t:\tretval->data = \n\"%s\"\n", retval->data);
      mannounce("==================================================\n");
    }
    promptPointer[0] = '\0';                   /* chomp the prompt I           */
    retval->bytesUsed -= strlen(prompt)    ;   /* chomp the prompt II          */
    if(MSG_DEBUG){
      mannounce("readMaudeMsg\t:\tretval->bytesUsed = %d\n", retval->bytesUsed);
      mannounce("readMaudeMsg\t:\tretval->data = \n\"%s\"\n", retval->data);
      mannounce("==================================================\n");
    }
    if((retval->bytesUsed == 0) || 
       ((retval->bytesUsed == 1) && (retval->data[0] == '\n'))){
      sprintf(retval->data, "OK\n");
      retval->bytesUsed = strlen("OK\n");
    }
  }
  return retval;

 fail:
  freeMsg(retval);
  retval = NULL;
  return retval;
}

msg* readPVSMsg(char *prompt, int fd){
  /*  char prompt[] = "\ncl-user(";  */
  char *promptPointer = NULL;
  int bytes = 0;
  char buff[MSG_BUFFSZ];
  msg* retval = makeMsg(MSG_BUFFSZ);
  if(retval == NULL){
    fprintf(stderr, "makeMsg in %d failed\n", getpid());
    goto fail;
  }
  while(1){
  restart:
    if((bytes = read(fd, buff, MSG_BUFFSZ)) < 0){
      if(errno == EINTR){
	mannounce("readPVSMsg  in %d restarting after being interrupted by a signal\n", getpid());
	goto restart;
      }
      if(errno == EBADF){
	fprintf(stderr, "readPVSMsg  in %d failing because of a bad file descriptor\n", getpid());
	goto fail;
      }
      fprintf(stderr, "Read  in %d returned with nothing\n", getpid());
      return retval;
    }
    if(addToMsg(retval, bytes, buff) != 0){
      fprintf(stderr, "addToMsg in %d failed\n", getpid());
      /*      retval = NULL; */
      goto fail;
    }
    if((promptPointer = strstr(retval->data, prompt)) != NULL) break;
  }
  if(retval != NULL){
    int len = strlen(promptPointer) - 1;
    promptPointer[1] = '\0';                   /* leave the \n                */
    retval->bytesUsed -= len;                  /* omit the rest of the prompt */
  }
  return retval;
 fail:
  return NULL;
}



msg* readMsg(int fd){
  int bytes = 0;
  char buff[MSG_BUFFSZ];
  msg* retval = NULL;
 restart:
  mannounce("readMsg  in %d starting (or restarting)\n", getpid());
  if((bytes = read(fd, buff, MSG_BUFFSZ)) < 0){
    if(errno == EINTR){
      mannounce("readMsg  in %d restarting after being interrupted by a signal\n", getpid());
      goto restart;
    }
    if(errno == EBADF){
      fprintf(stderr, "readMsg  in %d failing because of a bad file descriptor\n", getpid());
      goto fail;
    }
    fprintf(stderr, "Read  in %d returned with nothing\n", getpid());
    return retval;
  }
  if(MSG_DEBUG){
    int i;
    mannounce("readMsg in %d going into non blocking mode (bytes = %d)\n", getpid(), bytes);
    for(i = 0; i < bytes; i++)
      fprintf(stderr, "%c", buff[i]);
    fprintf(stderr, "\n\t%d\n", getpid());
  }
  if(setFlag(fd, O_NONBLOCK) < 0) goto fail;
  mannounce("readMsg in %d setFlag to O_NONBLOCK\n", getpid());
  if((retval = makeMsg(MSG_BUFFSZ)) == NULL){
    fprintf(stderr, 
	    "makeMsg in %d failed\n", getpid());
    goto exit;
  }
  mannounce("readMsg in %d made Msg\n", getpid());
  if(addToMsg(retval, bytes, buff) != 0){
    fprintf(stderr, "addToMsg in %d failed\n", getpid());
    retval = NULL;
    goto exit;
  }
  mannounce("readMsg in %d added  buff to Msg\n", getpid());
  while(usleep(1), (bytes = read(fd, buff, MSG_BUFFSZ)) > 0){
    mannounce("readMsg in %d read in non-blocking mode (bytes = %d)\n", getpid(), bytes);
    if(addToMsg(retval, bytes, buff) != 0){
      fprintf(stderr, "addToMsg  in %d failed\n", getpid());
      retval = NULL;
      goto exit;
    }
  }
 exit:
  mannounce("readMsg in %d read exiting non-blocking mode (bytes = %d)\n", getpid(), bytes);
  clearFlag(fd, O_NONBLOCK);
  mannounce("readMsg in %d cleared non-blocking flag\n", getpid());
  /* retval->data is a C string, and retval->bytesUsed is its C string length */
  if(retval != NULL){
    if(addToMsg(retval, 1, "\0") != 0){
      fprintf(stderr, "addToMsg  in %d failed\n", getpid());
      freeMsg(retval);
      retval = NULL;
      return retval;
    }
    retval->bytesUsed--;
  }
  mannounce("readMsg in %d read exiting non-blocking mode (retval->bytesUsed = %d)\n", 
	    getpid(), retval->bytesUsed);
  return retval;
 fail:
  return retval;
}

msg* readMsgVolatile(int fd, volatile int* exitFlag){
  int bytes = 0;
  char buff[MSG_BUFFSZ];
  msg* retval = NULL;
 restart:
  if(*exitFlag){
    mannounce("readMsgVolatile in %d (exitFlag = %d)\n", getpid(), *exitFlag);
    return NULL;
  }
  mannounce("readMsgVolatile  in %d (exitFlag = %d) starting (or restarting)\n", 
	    getpid(), *exitFlag);
  if((bytes = read(fd, buff, MSG_BUFFSZ)) < 0){
    if(*exitFlag){ 
      mannounce("readMsgVolatile in %d (exitFlag = %d)\n", getpid(), *exitFlag);
      return NULL; 
    }
    if(errno == EINTR){
      mannounce("readMsgVolatile  in %d (exitFlag = %d) restarting after being interrupted by a signal\n", 
		getpid(), *exitFlag);
      goto restart;
    }
    if(errno == EBADF){
      mannounce("readMsgVolatile  in %d  (exitFlag = %d) failing because of a bad file descriptor\n", getpid(), *exitFlag);
      goto fail;
    }
    mannounce("Read in %d (exitFlag = %d) returned with nothing\n", getpid(), *exitFlag);
    return retval;
  }
  if(MSG_DEBUG){
    int i;
    fprintf(stderr, 
	    "readMsgVolatile in %d (exitFlag = %d) going into non blocking mode (bytes = %d)\n", 
	    getpid(), *exitFlag, bytes);
    for(i = 0; i < bytes; i++)
      fprintf(stderr, "%c", buff[i]);
    fprintf(stderr, "\n\t%d\n", getpid());
  }
  if(*exitFlag){ 
    mannounce("readMsgVolatile in %d (exitFlag = %d)\n", getpid(), *exitFlag);
    return NULL; 
  }    
  if(setFlag(fd, O_NONBLOCK) < 0) goto fail;
  mannounce("readMsgVolatile in %d (exitFlag = %d) setFlag to O_NONBLOCK\n", getpid(), *exitFlag);

  if(*exitFlag){ 
    mannounce("readMsgVolatile in %d (exitFlag = %d)\n", getpid(), *exitFlag);
    return NULL; 
  }
  if((retval = makeMsg(MSG_BUFFSZ)) == NULL){
    fprintf(stderr, "makeMsg in %d failed\n", getpid());
    goto exit;
  }
  mannounce("readMsgVolatile in %d (exitFlag = %d) made Msg\n", getpid(), *exitFlag);
  if(*exitFlag){ 
    mannounce("readMsgVolatile in %d (exitFlag = %d)\n", getpid(), *exitFlag);
    return NULL; 
  }
  if(addToMsg(retval, bytes, buff) != 0){
    fprintf(stderr, "addToMsg in %d failed\n", getpid());
    retval = NULL;
    goto exit;
  }
  mannounce("readMsgVolatile in %d (exitFlag = %d) added  buff to Msg\n", getpid(), *exitFlag);
  if(*exitFlag){ 
    mannounce("readMsgVolatile in %d (exitFlag = %d)\n", getpid(), *exitFlag);
    return NULL;
  }
  while(usleep(1), 
	(bytes = read(fd, buff, MSG_BUFFSZ)) > 0){
    if(*exitFlag){ 
      mannounce("readMsgVolatile in %d (exitFlag = %d)\n", getpid(), *exitFlag);
      return NULL; 
    }
    mannounce("readMsgVolatile in %d (exitFlag = %d) read in non-blocking mode (bytes = %d)\n", getpid(), *exitFlag, bytes);
    
    if(*exitFlag){ 
      mannounce("readMsgVolatile in %d (exitFlag = %d)\n", getpid(), *exitFlag);
      return NULL; 
    }
    if(addToMsg(retval, bytes, buff) != 0){
      fprintf(stderr, "addToMsg  in %d failed\n", getpid());
      retval = NULL;
      goto exit;
    }
  }
 exit:
  mannounce("readMsgVolatile in %d (exitFlag = %d) read exiting non-blocking mode (bytes = %d)\n", 
	    getpid(), *exitFlag, bytes);
  if(*exitFlag){ 
    mannounce("readMsgVolatile in %d (exitFlag = %d)\n", getpid(), *exitFlag);
    return NULL; 
  } 
  clearFlag(fd, O_NONBLOCK);
  
  mannounce("readMsgVolatile in %d (exitFlag = %d) cleared non-blocking flag\n", getpid(), *exitFlag);
  /* retval->data is a C string, and retval->bytesUsed is its C string length */
  if(retval != NULL){
    if(addToMsg(retval, 1, "\0") != 0){
      fprintf(stderr, "addToMsg  in %d failed\n", getpid());
      freeMsg(retval);
      retval = NULL;
      return retval;
    }
    retval->bytesUsed--;
  }
  mannounce("readMsgVolatile in %d (exitFlag = %d) read exiting non-blocking mode (retval->bytesUsed = %d)\n", 
	    getpid(), *exitFlag, retval->bytesUsed);
  return retval;
  
 fail:
  return retval;
}

int writeMsg(int fd, msg* m){
  int blksz = 1024;
  int bytesRemaining, bytesWritten;
  char *buff;
  if(m == NULL) return 0;
  bytesRemaining = m->bytesUsed;
  buff = m->data;
  while(bytesRemaining > 0){
    if((bytesWritten = write(fd, 
			     buff, 
			     ((bytesRemaining < blksz) ? 
			      bytesRemaining : 
			      blksz))) < 0){
      perror("Write failed in writeMsg");
      return -1;
    }
    bytesRemaining -= bytesWritten;
    if(bytesRemaining > 0) buff = &buff[bytesWritten];
  }
  return 1;
}


int writeInt(int fd, int number){
  int len;
  char buff[SIZE];
  sprintf(buff, "%d\n", number);
  len = strlen(buff);
  if(write(fd, buff, len) != len){
    fprintf(stderr, "write failed in writeInt\n");
    return -1;
  }
  return 1;
}

int readInt(int fd, int* nump){
  ssize_t retval;
  if(nump == NULL){
    return -1;
  } else {
    int i = 0;
    char buff[SIZE];
    while(i < SIZE){
    restart:
      errno = 0;
      if((retval = read(fd, &buff[i], sizeof(char))) != sizeof(char)){
	if(retval == 0){
	  buff[i] = '\0';
	  break;
	} else {
	  if(errno == EINTR){
	    mannounce("readInt: restarting (errno == EINTR)\n");
	    goto restart;
	  }
	  fprintf(stderr, "readInt: read failed: %s\n", strerror(errno));
	  return -1;
	  if(errno == EBADF){
	    fprintf(stderr, "readInt: read failed because of a bad file descriptor\n");
	    return -1;
	  }
	}
      }
      if(!isdigit(buff[i])){
	buff[i] = '\0';
	break;
      }
      i++;
    }
    if(i == SIZE) return -1;
    *nump = atoi(buff);
    return 1;
  }
}

int readIntVolatile(int fd, int* nump, volatile int* exitFlag){
  ssize_t retval;
  if(nump == NULL){
    return -1;
  } else {
    int i = 0;
    char buff[SIZE];
    while(i < SIZE){
      if(*exitFlag) return -1;
      if((retval = read(fd, &buff[i], sizeof(char))) != sizeof(char)){
	if(*exitFlag) return -1;
	if(retval == 0){
	  buff[i] = '\0';
	  break;
	} else {
	  fprintf(stderr, "read failed in readInt: %s\n", strerror(errno));
	  return -1;
	}
      }
      if(!isdigit(buff[i])){
	buff[i] = '\0';
	break;
      }
      i++;
    }
    if(i == SIZE) return -1;
    *nump = atoi(buff);
    return 1;
  }
}

int writeActorSpec(int fd, actor_spec *act){
  int i, len;
  if(act == NULL) return -1;
  if(act->name == NULL)  return -1;
  len = strlen(act->name);
  if(write(fd, act->name, len) != len) return -1;
  if(write(fd, "\n", sizeof(char)) !=  sizeof(char)) return -1;
  if(writeInt(fd, act->pid) != 1) return -1;
  for(i = 0; i < 3; i++){
    if(act->fifos[i] == NULL) return -1;
    len = strlen(act->fifos[i]);
    if(write(fd, act->fifos[i], len) != len) return -1;
    if(write(fd, "\n", sizeof(char)) != sizeof(char)) return -1;
  }
  return 1;
}

actor_spec *readActorSpec(int fd){
  char *tempLine;
  int i;
  actor_spec *retval = (actor_spec *)calloc(1, sizeof(actor_spec));
  if(retval ==  NULL) return retval;
  tempLine = readline(fd);
  if(tempLine == NULL) return NULL;
  strcpy(retval->name, tempLine);
  free(tempLine);
  if(readInt(fd, &retval->pid) != 1) return NULL; 
  for(i = 0; i < 3; i++){
    tempLine = readline(fd);
    if(tempLine == NULL) return NULL;
    strcpy(retval->fifos[i], tempLine);
    free(tempLine);
  }
  return retval;
}


char* readline(int fd){
  char *retval = (char *)calloc(PATH_MAX, sizeof(char));
  int i = 0;
  if(retval == NULL) return NULL;
  while(read(fd, &retval[i], sizeof(char)) == sizeof(char)){
    if(retval[i] == '\n'){
      retval[i] = '\0';
      return retval;
    }
    i++;
  }
  free(retval);
  return NULL;
}


msg* acceptMsg(int fd){
  char buff[MSG_BUFFSZ];
  int bytes, bytesIncoming, errcode;
  msg* retval;
  errcode = readInt(fd, &bytesIncoming);
  if((errcode < 0) || (bytesIncoming <= 0)) return NULL;
  mannounce("acceptMsg: expecting %d bytes\n", bytesIncoming);
  retval = makeMsg(bytesIncoming + 1);
  if(retval == NULL) goto fail;
  while(1){
  restart:
    if((bytes = read(fd, buff, (bytesIncoming < MSG_BUFFSZ ? bytesIncoming : MSG_BUFFSZ))) < 0){
      if(errno == EINTR){
	mannounce("acceptMsg: restarting after being interrupted by a signal\n");
	goto restart;
      }
      if(errno == EBADF){
	fprintf(stderr, "acceptMsg: failing because of a bad file descriptor\n");
	goto fail;
      }
      fprintf(stderr, "acceptMsg: read  returned with nothing\n");
      return NULL;
    }
    mannounce("acceptMsg: got %d bytes\n", bytes);
    if(addToMsg(retval, bytes, buff) != 0){
      fprintf(stderr, "acceptMsg: addToMsg failed\n");
      freeMsg(retval);
      retval = NULL;
      goto fail;
    }
    if(retval->bytesUsed >= bytesIncoming){
      mannounce("acceptMsg: retval->bytesUsed =  %d\n", retval->bytesUsed);
      if(retval->bytesUsed > bytesIncoming){
	fprintf(stderr, 
		"acceptMsg: got %d more bytes than expected\n",
		retval->bytesUsed - bytesIncoming);
      }
      retval->data[retval->bytesUsed] = '\0';
      return retval;
    }
  }
 fail:
  return NULL;
}

msg* acceptMsgVolatile(int fd, volatile int* exitFlag){
  char buff[MSG_BUFFSZ];
  int bytes, bytesIncoming, errcode;
  msg* retval;
  mannounce("acceptMsgVolatile: calling readInt\n");
  errcode = readInt(fd, &bytesIncoming);
  if(*exitFlag || (errcode < 0) || (bytesIncoming <= 0)){
    if(MSG_DEBUG){   
      mannounce("acceptMsgVolatile: (*exitFlag||(errcode < 0)||(bytesIncoming <= 0)) is true!\n");
      mannounce("\t*exitFlag = %d\n", *exitFlag);
      mannounce("\t*errcode = %d\n", errcode);
      mannounce("\t*bytesIncoming = %d\n", bytesIncoming);
    }
    return NULL;
  }
  mannounce("acceptMsgVolatile: expecting %d bytes\n", bytesIncoming);
  retval = makeMsg(bytesIncoming + 1);
  if(retval == NULL) goto fail;
  while(1){
  restart:
    if(*exitFlag) return NULL;
    if((bytes = read(fd, buff, (bytesIncoming < MSG_BUFFSZ ? bytesIncoming : MSG_BUFFSZ))) < 0){
      if(errno == EINTR){
	mannounce("acceptMsgVolatile: restarting after being interrupted by a signal\n");
	goto restart;
      }
      if(errno == EBADF){
	fprintf(stderr, "acceptMsgVolatile: failing because of a bad file descriptor\n");
	goto fail;
      }
      fprintf(stderr, "acceptMsgVolatile: read  returned with nothing\n");
      return NULL;
    }

    mannounce("acceptMsgVolatile: got %d bytes\n", bytes);

    if(*exitFlag){
      mannounce("acceptMsgVolatile: bailing with NULL,  *exitFlag is true\n");
     return NULL;
    }
    if(addToMsg(retval, bytes, buff) != 0){
      fprintf(stderr, "acceptMsgVolatile: addToMsg failed\n");
      freeMsg(retval);
      retval = NULL;
      goto fail;
    }
    if(retval->bytesUsed >= bytesIncoming){
      if(retval->bytesUsed > bytesIncoming){
	fprintf(stderr, 
		"acceptMsgVolatile: got %d more bytes than expected\n",
		retval->bytesUsed - bytesIncoming);
      }
      retval->data[retval->bytesUsed] = '\0';
      return retval;
    }
  }
 fail:
  return NULL;
}

int sendMsg(int fd, msg* m){
  errno = 0;
  if(m == NULL) goto fail;
  if(writeInt(fd, m->bytesUsed) < 0) goto fail;
  if(MSG_DEBUG)
    if(writeInt(STDERR_FILENO, m->bytesUsed) < 0) goto fail;
  if(write(fd, m->data, m->bytesUsed) != m->bytesUsed) goto fail;
  if(MSG_DEBUG)
    if(write(STDERR_FILENO, m->data, m->bytesUsed) != m->bytesUsed) goto fail;
  return m->bytesUsed;
 fail:
  if(errno != 0)
    fprintf(stderr, "sendMsg: failed -- %s\n", strerror(errno));
  else 
    fprintf(stderr, "sendMsg: failed\n");
  return -1;
}


int sendFormattedMsgFP(FILE* fp, char* fmt, ...){
  char *p, *sval;
  int ival;
  double dval;
  char numbuff[16];
  va_list ap;
  msg* m = makeMsg(MSG_BUFFSZ);
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
  fprintf(fp, "%d\n%s", m->bytesUsed, m->data);
  fflush(fp);
  return m->bytesUsed;
}

int sendFormattedMsgFD(int fd, char* fmt, ...){
  char *p, *sval;
  int ival;
  double dval;
  char numbuff[16];
  va_list ap;
  msg* m = makeMsg(MSG_BUFFSZ);
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
  writeInt(fd, m->bytesUsed);
  write(fd, m->data, m->bytesUsed);
  return m->bytesUsed;
}

void echoMsg(int from, int to){
  msg* message;
  message = acceptMsg(from);
  if(message != NULL){
    sendMsg(to, message);
    if(MSG_DEBUG){
      sendMsg(STDERR_FILENO, message);
      fprintf(stderr, "echoMsg: echo wrote %d bytes\n", message->bytesUsed);
    }
    freeMsg(message);
  }
}

void* echoLoopDieOnFail(void* args){
  int from, to;
  echofds* fds = (echofds*)args;
  msg* message;
  if(fds == NULL) goto fail;
  from = fds->from;
  to = fds->to;
  while(1){
    message = acceptMsg(from);
    if(message != NULL){
      sendMsg(to, message);
      if(MSG_DEBUG){
	sendMsg(STDERR_FILENO, message);
	fprintf(stderr, "echoMsg: echo wrote %d bytes\n", message->bytesUsed);
      }
      freeMsg(message);
    } else break;
  }
 fail:
  terminateIOP();
  return NULL;
}

void* echoLoop(void* args){
  int from, to;
  echofds* fds = (echofds*)args;
  msg* message;
  if(fds == NULL) goto fail;
  from = fds->from;
  to = fds->to;
  while(1){
    message = acceptMsg(from);
    if(message != NULL){
      sendMsg(to, message);
      if(MSG_DEBUG){
	sendMsg(STDERR_FILENO, message);
	mannounce("echoMsg: echo wrote %d bytes\n", message->bytesUsed);
      }
      freeMsg(message);
    }
  }
 fail:
  return NULL;
}


void echo2Maude(int from, int to){
  msg* message;
  message = acceptMsg(from);
  if(message != NULL){
    writeMsg(to, message);
    if(MSG_DEBUG){
      writeMsg(STDERR_FILENO, message);
      mannounce("echo2Maude: wrote %d bytes\n", message->bytesUsed);
    }
    freeMsg(message);
  }
}

void echo2PVS(int from, int to){
  msg* message;
  message = acceptMsg(from);
  if(message != NULL){
    writeMsg(to, message);
    if(MSG_DEBUG){
      writeMsg(STDERR_FILENO, message);
      mannounce("echo2Pvs: wrote %d bytes\n", message->bytesUsed);
    }
    freeMsg(message);
  }
}


