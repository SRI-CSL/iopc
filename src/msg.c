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
#include "iop_lib.h"
#include "iop_utils.h"
#include "actor.h"
#include "dbugflags.h"
#include "ec.h"

/* statics */
static int setFlag(int fd, int flags);
static int clearFlag(int fd, int flags);

/* externs  */
extern int   self_debug_flag;
extern char* self;

/* local error logging */
static pthread_mutex_t iop_err_mutex = PTHREAD_MUTEX_INITIALIZER;
static void eM(const char *format, ...){
  va_list arg;
  va_start(arg, format);
  if(format != NULL){
    if(MSG_DEBUG && self_debug_flag){
      ec_rv( pthread_mutex_lock(&iop_err_mutex) );
#if defined(_LINUX)
      fprintf(stderr, "MSG(%lu)\t:\t", (unsigned long)pthread_self());
#elif defined(_MAC)
      fprintf(stderr, "MSG(%p)\t:\t", (void *)pthread_self());
#endif
      vfprintf(stderr, format, arg);
      ec_rv( pthread_mutex_unlock(&iop_err_mutex) );
    }
  }
  va_end(arg);
  return;
EC_CLEANUP_BGN
  va_end(arg);
  return;
EC_CLEANUP_END
}


int mywrite(int fd, char *buff, int count, int verbose){
  int bytesLeft = 0, bytesWritten = 0, index = 0;
  bytesLeft = count;
  while(bytesLeft > 0){
    bytesWritten = write(fd, buff + index, bytesLeft);
    if(bytesWritten < 0){
      if(errno != EINTR){ 
	if(verbose)perror("write in mywrite failed:");
	return -1; 
      } else { 
	continue;
      }
    }
    bytesLeft -= bytesWritten;
    index += bytesWritten;
  }
  return count;
}


msg* makeMsg(int bytes){
  msg* retval = NULL;
  ec_null( retval = calloc(1, sizeof(msg)) );
  ec_null( retval->data = calloc(bytes, sizeof(char)) );
  retval->bytesUsed = 0;
  retval->bytesLeft = bytes;
  return retval;
EC_CLEANUP_BGN
  free(retval);
  return NULL;
EC_CLEANUP_END
}  

void freeMsg(msg* m){
  if(m == NULL){ return; }
  free(m->data);
  free(m);
}

int addToMsg(msg* m, int bytes, char* buff){
  if(bytes <= 0){
    fprintf(stderr, "Bad arguments to addToMsg (bytes <= 0)\n");
    return -1;
  }
  if(buff == NULL){
    fprintf(stderr, "Bad arguments to addToMsg (buff == NULL)\n");
    return -1;
  }
  eM("addToMsg: bytes = %d m->bytesLeft = %d\n", bytes, m->bytesLeft);
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

static msg* abbreviateMsg(msg* m){
  msg* retval;
  int msgleni, msgleno;
  if(m == NULL){ return m; }
  msgleni = m->bytesUsed;
  msgleni = msgleni < 100 ? msgleni : 100;
  retval = makeMsg(msgleni);
  msgleno =  addToMsg(retval, msgleni, m->data);
  return retval;
}

int setFlag(int fd, int flags){
  int val;
  if((val = fcntl(fd, F_GETFL, 0)) == -1){ return -1; }
  val |= flags;
  if(fcntl(fd, F_SETFL, val) == -1 ){ return -1; }
  return 0;
}

int clearFlag(int fd, int flags){
  int val;
  if((val = fcntl(fd, F_GETFL, 0)) == -1){ return -1; }
  val &= ~flags;
  if(fcntl(fd, F_SETFL, val)== -1){ return -1; }
  return 0;
}

msg* readMaudeMsg(int fd){
  char prompt[] = "Maude> ";
  char *promptPointer = NULL;
  int bytes = 0, iteration = 0, continuation = 0;
  msg* retval;
  static msg* next = NULL;
  if(next == NULL){ 
    retval = makeMsg(BUFFSZ);
  } else {
    continuation = 1;
    retval = next;
  }
  
  next = NULL;

  if(retval == NULL){
    fprintf(stderr, "makeMsg in %d failed\n", getpid());
    goto fail;
  }

  while(1){
    char buff[BUFFSZ];
    
    if(continuation){
      continuation = 0;
      goto checkprompt;
    }
    
  restart:

    eM("readMaudeMsg\t:\tcommencing a read\n");
    if((bytes = read(fd, buff, BUFFSZ)) < 0){
      eM("readMaudeMsg\t:\tread error read returned %d bytes\n", bytes);
      if(errno == EINTR){
	eM("readMsg  in %d restarting after being interrupted by a signal\n", getpid());
	goto restart;
      }
      if(errno == EBADF){
	fprintf(stderr, "readMsg  in %d failing because of a bad file descriptor\n", getpid());
	goto fail;
      }
      fprintf(stderr, "Read  in %d returned with nothing\n", getpid());
      return retval;
    } /* if((bytes = read(fd, buff, BUFFSZ)) < 0) */

    eM("readMaudeMsg\t:\tread read %d bytes\n", bytes);

    if(addToMsg(retval, bytes, buff) != 0){
      fprintf(stderr, "addToMsg in %d failed\n", getpid());
      goto fail;
    }


  checkprompt:
    
    iteration++;
    
    if((promptPointer = strstr(retval->data, prompt)) != NULL){  break; }

  }/* while */

  if(retval != NULL){
    int msgLength = (promptPointer - retval->data) ;
    int datLength = msgLength + strlen(prompt) ;
    int total = retval->bytesUsed;
    if(datLength < total) {
      next = makeMsg(BUFFSZ);
      addToMsg(next,total - datLength,&(retval->data[datLength]));
      promptPointer[0] = '\0';             /* chomp the prompt I  */
      retval->bytesUsed = msgLength;       /* chomp the prompt II */
    } else {
      promptPointer[0] = '\0';             /* chomp the prompt I  */
      retval->bytesUsed -= strlen(prompt); /* chomp the prompt II */
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
  char buff[BUFFSZ];
  msg* retval = makeMsg(BUFFSZ);
  if(retval == NULL){
    fprintf(stderr, "makeMsg in %d failed\n", getpid());
    goto fail;
  }
  while(1){
  restart:
    if((bytes = read(fd, buff, BUFFSZ)) < 0){
      if(errno == EINTR){
	eM("readPVSMsg  in %d restarting after being interrupted by a signal\n", getpid());
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
  freeMsg(retval);
  return NULL;
}



msg* readMsg(int fd){
  int bytes = 0;
  char buff[BUFFSZ];
  msg* retval = NULL;
 restart:
  eM("readMsg  in %d starting (or restarting)\n", getpid());
  if((bytes = read(fd, buff, BUFFSZ)) < 0){
    if(errno == EINTR){
      eM("readMsg  in %d restarting after being interrupted by a signal\n", getpid());
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
    eM("readMsg in %d going into non blocking mode (bytes = %d)\n", getpid(), bytes);
    for(i = 0; i < bytes; i++)
      fprintf(stderr, "%c", buff[i]);
    fprintf(stderr, "\n\t%d\n", getpid());
  }
  if(setFlag(fd, O_NONBLOCK) < 0){ goto fail; }
  eM("readMsg in %d setFlag to O_NONBLOCK\n", getpid());
  if((retval = makeMsg(BUFFSZ)) == NULL){
    fprintf(stderr, 
	    "makeMsg in %d failed\n", getpid());
    goto exit;
  }
  eM("readMsg in %d made Msg\n", getpid());
  if(addToMsg(retval, bytes, buff) != 0){
    fprintf(stderr, "addToMsg in %d failed\n", getpid());
    retval = NULL;
    goto exit;
  }
  eM("readMsg in %d added  buff to Msg\n", getpid());
  while(iop_usleep(1), (bytes = read(fd, buff, BUFFSZ)) > 0){
    eM("readMsg in %d read in non-blocking mode (bytes = %d)\n", getpid(), bytes);
    if(addToMsg(retval, bytes, buff) != 0){
      fprintf(stderr, "addToMsg  in %d failed\n", getpid());
      freeMsg(retval);
      retval = NULL;
      goto exit;
    }
  }
 exit:

  eM("readMsg in %d read exiting non-blocking mode (bytes = %d)\n", getpid(), bytes);
  clearFlag(fd, O_NONBLOCK);
  eM("readMsg in %d cleared non-blocking flag\n", getpid());
  /* retval->data is a C string, and retval->bytesUsed is its C string length */
  if(retval != NULL){
    if(addToMsg(retval, 1, "\0") != 0){
      fprintf(stderr, "addToMsg  in %d failed\n", getpid());
      freeMsg(retval);
      retval = NULL;
      return retval;
    }
    retval->bytesUsed--;
    eM("readMsg in %d read exiting non-blocking mode (retval->bytesUsed = %d)\n", getpid(), retval->bytesUsed);
  }
  return retval;
 fail:
  return retval;
}

msg* readMsgVolatile(int fd, volatile int* exitFlag){
  int bytes = 0;
  char buff[BUFFSZ];
  msg* retval = NULL;
 restart:
  if(*exitFlag){
    eM("readMsgVolatile in %d (exitFlag = %d)\n", getpid(), *exitFlag);
    goto fail;
  }
  eM("readMsgVolatile  in %d (exitFlag = %d) starting (or restarting)\n", getpid(), *exitFlag);
  if((bytes = read(fd, buff, BUFFSZ)) < 0){
    if(*exitFlag){ 
      eM("readMsgVolatile in %d (exitFlag = %d)\n", getpid(), *exitFlag);
      goto fail; 
    }
    if(errno == EINTR){
      eM("readMsgVolatile  in %d (exitFlag = %d) restarting after being interrupted by a signal\n", getpid(), *exitFlag);
      goto restart;
    }
    if(errno == EBADF){
      eM("readMsgVolatile  in %d  (exitFlag = %d) failing because of a bad file descriptor\n", getpid(), *exitFlag);
      goto fail;
    }
    eM("Read in %d (exitFlag = %d) returned with nothing\n", getpid(), *exitFlag);
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
    eM("readMsgVolatile in %d (exitFlag = %d)\n", getpid(), *exitFlag);
    goto fail; 
  }    
  if(setFlag(fd, O_NONBLOCK) < 0){  goto fail; }
  eM("readMsgVolatile in %d (exitFlag = %d) setFlag to O_NONBLOCK\n", getpid(), *exitFlag);

  if(*exitFlag){ 
    eM("readMsgVolatile in %d (exitFlag = %d)\n", getpid(), *exitFlag);
    goto fail; 
  }
  if((retval = makeMsg(BUFFSZ)) == NULL){
    fprintf(stderr, "makeMsg in %d failed\n", getpid());
    goto exit;
  }
  eM("readMsgVolatile in %d (exitFlag = %d) made Msg\n", getpid(), *exitFlag);
  if(*exitFlag){ 
    eM("readMsgVolatile in %d (exitFlag = %d)\n", getpid(), *exitFlag);
    goto fail; 
  }
  if(addToMsg(retval, bytes, buff) != 0){
    fprintf(stderr, "addToMsg in %d failed\n", getpid());
    goto fail; 
  }
  eM("readMsgVolatile in %d (exitFlag = %d) added  buff to Msg\n", getpid(), *exitFlag);
  if(*exitFlag){ 
    eM("readMsgVolatile in %d (exitFlag = %d)\n", getpid(), *exitFlag);
    goto fail;
  }
  while(iop_usleep(1), 
	(bytes = read(fd, buff, BUFFSZ)) > 0){
    if(*exitFlag){ 
      eM("readMsgVolatile in %d (exitFlag = %d)\n", getpid(), *exitFlag);
      goto fail; 
    }
    eM("readMsgVolatile in %d (exitFlag = %d) read in non-blocking mode (bytes = %d)\n", getpid(), *exitFlag, bytes);
    
    if(*exitFlag){ 
      eM("readMsgVolatile in %d (exitFlag = %d)\n", getpid(), *exitFlag);
      goto fail; 
    }
    if(addToMsg(retval, bytes, buff) != 0){
      fprintf(stderr, "addToMsg  in %d failed\n", getpid());
      goto fail;
    }
  }
 exit:
  eM("readMsgVolatile in %d (exitFlag = %d) read exiting non-blocking mode (bytes = %d)\n", getpid(), *exitFlag, bytes);
  if(*exitFlag){ 
    eM("readMsgVolatile in %d (exitFlag = %d)\n", getpid(), *exitFlag);
    goto fail; 
  } 
  
  eM("readMsgVolatile in %d (exitFlag = %d) cleared non-blocking flag\n", getpid(), *exitFlag);
  /* retval->data is a C string, and retval->bytesUsed is its C string length */
  if(retval != NULL){
    if(addToMsg(retval, 1, "\0") != 0){
      fprintf(stderr, "addToMsg  in %d failed\n", getpid());
      goto fail;
    }
    retval->bytesUsed--;

    eM("readMsgVolatile in %d (exitFlag = %d) read exiting non-blocking mode (retval->bytesUsed = %d)\n", getpid(), *exitFlag, retval->bytesUsed);

  } else {

    eM("readMsgVolatile in %d (exitFlag = %d) read exiting non-blocking mode (retval = NULL)\n", getpid(), *exitFlag);
  }

  clearFlag(fd, O_NONBLOCK);
  
  return retval;
  
 fail:

  clearFlag(fd, O_NONBLOCK); 
  freeMsg(retval);
  retval = NULL;
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
  restart:
    if((bytesWritten = write(fd, 
			     buff, 
			     ((bytesRemaining < blksz) ? 
			      bytesRemaining : 
			      blksz))) < 0){
      if(errno == EINTR){
	goto restart; 
      } else {
	perror("Write failed in writeMsg");
      }
      return -1;
    }
    bytesRemaining -= bytesWritten;
    if(bytesRemaining > 0) buff = &buff[bytesWritten];
  }
  return 1;
}

int logMsg(char* from, char* filename, msg* message){
  FILE* fp; 
  int fno;
  if(filename != NULL){
    fp = fopen(filename,"a");
    if(fp != NULL){
      fno = fileno(fp) ;
      write(fno, "start ", strlen("start "));
      write(fno, from, strlen(from));
      write(fno, "\n", strlen("\n"));
      //writeMsg(fno, message);
      writeMsg(fno, abbreviateMsg(message));      
      write(fno, "\nstop\n", strlen("\nstop\n"));
      fclose(fp);
      return 1;
    }
  }
  return 0;
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

int readInt(int fd, int* nump, const char* caller){
  ssize_t retval;
  if(nump == NULL){
    return -1;
  } else {
    int i = 0, bad = 0;
    char buff[SIZE];
    
  squareone:
    i = 0;
    while(i < SIZE){
    restart:
      errno = 0;
      if((retval = read(fd, &buff[i], sizeof(char))) != sizeof(char)){
	if(retval == 0){
	  buff[i] = '\0';
	  break;
	} else {
	  if(errno == EINTR){
	    eM("readInt: restarting (errno == EINTR)\n");
	    goto restart;
	  }
	  fprintf(stderr, "readInt@%s: read failed: %s\n", caller, strerror(errno));
	  return -1;
	}
      }
      if(buff[i] == '\n'){
	buff[i] = '\0';
	break;
      }
      if(bad){ continue; }
      if(!isdigit(buff[i])){
	bad = 1;
	/* fprintf(stderr, "readInt saw a non-digit: %c (bad = %d)\n", buff[i], bad); */
	/* read and discard everything upto the next '\n'; */
	goto squareone;
      }
      i++;
    } /* while  */
    if(bad || (i == SIZE)){ return -1; }
    *nump = atoi(buff);
    /*    fprintf(stderr, "readInt returning: %d (bad = %d, i = %d)\n", *nump, bad, i); */
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
	  fprintf(stderr, "read failed in readIntVolatile: %s\n", strerror(errno));
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
  len = strlen(act->name);
  if(write(fd, act->name, len) != len) return -1;
  if(write(fd, "\n", sizeof(char)) !=  sizeof(char)) return -1;
  if(writeInt(fd, act->pid) != 1) return -1;
  for(i = 0; i < 3; i++){
    len = strlen(act->fifos[i]);
    if(write(fd, act->fifos[i], len) != len) return -1;
    if(write(fd, "\n", sizeof(char)) != sizeof(char)) return -1;
  }
  return 1;
}

actor_spec *readActorSpec(int fd){
  char *tempLine;
  int i, linelen;
  actor_spec *retval = (actor_spec *)calloc(1, sizeof(actor_spec));
  if(retval ==  NULL){  return retval; }
  tempLine = readline(fd);
  if(tempLine == NULL){
    goto fail;
  }
  linelen = strlen(tempLine) + 1;
  if(linelen >= PATH_MAX){
    free(tempLine);
    goto fail;
  }
  strncpy(retval->name, tempLine, linelen);
  free(tempLine);
  if(readInt(fd, &retval->pid, "readActorSpec") != 1){ goto fail; }
  for(i = 0; i < 3; i++){
    tempLine = readline(fd);
    if(tempLine == NULL){ goto fail; }
    linelen = strlen(tempLine) + 1;
    if(linelen >= PATH_MAX){ goto fail; }
    strncpy(retval->fifos[i], tempLine, linelen);
    free(tempLine);
  }
  return retval;

 fail:
  fprintf(stderr, "Failing\n");
  free(retval);
  return NULL;
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
  char buff[BUFFSZ];
  int bytes, bytesIncoming, bytesRemaining, errcode;
  msg* retval;
  errcode = readInt(fd, &bytesIncoming, "acceptMsg");
  if((errcode < 0) || (bytesIncoming <= 0)) return NULL;
  eM("acceptMsg: expecting %d bytes\n", bytesIncoming);
  retval = makeMsg(bytesIncoming + 1);
  if(retval == NULL){  goto fail; }
  bytesRemaining = bytesIncoming;
  while(1){
  restart:
    if((bytes = read(fd, buff, (bytesRemaining < BUFFSZ ? bytesRemaining : BUFFSZ))) < 0){
      if(errno == EINTR){
	eM("acceptMsg: restarting after being interrupted by a signal\n");
	goto restart;
      }
      if(errno == EBADF){
	fprintf(stderr, "acceptMsg: failing because of a bad file descriptor\n");
	goto fail;
      }
      fprintf(stderr, "acceptMsg: read  returned with nothing\n");
      return NULL;
    }
    eM("acceptMsg: got %d bytes\n", bytes);
    if(addToMsg(retval, bytes, buff) != 0){
      fprintf(stderr, "acceptMsg: addToMsg failed\n");
      goto fail;
    }
    /* iam 05/04/22 hopefully stop message collisions */
    bytesRemaining -= bytes;
    if(bytesRemaining <= 0){
      eM("acceptMsg: retval->bytesUsed =  %d\n", retval->bytesUsed);
      if(bytesRemaining < 0){
	fprintf(stderr, 
		"acceptMsg: got %d more bytes than expected\n",
		retval->bytesUsed - bytesIncoming);
      }
      retval->data[retval->bytesUsed] = '\0';
      return retval;
    }
  }
 fail:
  freeMsg(retval);
  return NULL;
}

msg* acceptMsgVolatile(int fd, volatile int* exitFlag){
  char buff[BUFFSZ];
  int bytes, bytesIncoming, bytesRemaining, errcode;
  msg* retval;
  eM("acceptMsgVolatile: calling readIntVolatile\n");
  errcode =  readIntVolatile(fd, &bytesIncoming, exitFlag);
  if(*exitFlag || (errcode < 0) || (bytesIncoming <= 0)){
    eM("acceptMsgVolatile: (*exitFlag||(errcode < 0)||(bytesIncoming <= 0)) is true!\n");
    eM("\t*exitFlag = %d\n", *exitFlag);
    eM("\t*errcode = %d\n", errcode);
    eM("\t*bytesIncoming = %d\n", bytesIncoming);
    return NULL;
  }
  eM("acceptMsgVolatile: expecting %d bytes\n", bytesIncoming);
  retval = makeMsg(bytesIncoming + 1);
  if(retval == NULL) goto fail;
  bytesRemaining = bytesIncoming;
  while(1){
  restart:
    if(*exitFlag){ goto fail; }
    if((bytes = read(fd, buff, (bytesRemaining < BUFFSZ ? bytesRemaining : BUFFSZ))) < 0){
      if(errno == EINTR){
	eM("acceptMsgVolatile: restarting after being interrupted by a signal\n");
	goto restart;
      }
      if(errno == EBADF){
	fprintf(stderr, "acceptMsgVolatile: failing because of a bad file descriptor\n");
	goto fail;
      }
      fprintf(stderr, "acceptMsgVolatile: read  returned with nothing\n");
       goto fail;
    }
    eM("acceptMsgVolatile: got %d bytes\n", bytes);
    if(*exitFlag){
      eM("acceptMsgVolatile: bailing with NULL,  *exitFlag is true\n");
      goto fail;
    }
    if(addToMsg(retval, bytes, buff) != 0){
      fprintf(stderr, "acceptMsgVolatile: addToMsg failed\n");
      goto fail;
    }
    /* iam 05/04/22 hopefully stop message collisions */
    bytesRemaining -= bytes;
    if(bytesRemaining <= 0){
      if(bytesRemaining < 0){
	fprintf(stderr, 
		"acceptMsgVolatile: got %d more bytes than expected\n",
		retval->bytesUsed - bytesIncoming);
      }
      retval->data[retval->bytesUsed] = '\0';
      return retval;
    }
  }
 fail:
  freeMsg(retval);
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
  int retval;
  msg* m = makeMsg(BUFFSZ);
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
      dval = va_arg(ap, double);
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
  retval = m->bytesUsed;
  freeMsg(m);
  return retval;
}

int sendFormattedMsgFD(int fd, char* fmt, ...){
  char *p, *sval;
  int ival;
  double dval;
  char numbuff[16];
  int retval;
  va_list ap;
  msg* m = makeMsg(BUFFSZ);
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
      dval = va_arg(ap, double);
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
  
  if(writeInt(fd, m->bytesUsed) < 0){
    fprintf(stderr, "sendFormattedMsgFD: writeInt failed\n");
  }

  if(write(fd, m->data, m->bytesUsed) < 0){
    fprintf(stderr, "sendFormattedMsgFD: write failed\n");
  }

  retval = m->bytesUsed;
  freeMsg(m);
  return retval;
}

void echoMsgVolatile(int from, int to, volatile int* exitFlag){
  msg* message;
  message = acceptMsgVolatile(from, exitFlag);
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
      if(MSG_DEBUG ||  G2D_REMOTE_ACTOR_DEBUG){
	sendMsg(STDERR_FILENO, message);
	fprintf(stderr, "\nechoMsg: echo wrote %d bytes\n", message->bytesUsed);
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
      if(MSG_DEBUG || G2D_REMOTE_ACTOR_DEBUG){
	sendMsg(STDERR_FILENO, message);
	eM("\nechoMsg: echo wrote %d bytes\n", message->bytesUsed);
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
    if(WATCH_MAUDE)logMsg("notmaude", MAUDE_LOGFILE, message);
    writeMsg(to, message);
    if(MSG_DEBUG){
      writeMsg(STDERR_FILENO, message);
      eM("echo2Maude: wrote %d bytes\n", message->bytesUsed);
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
      eM("echo2Pvs: wrote %d bytes\n", message->bytesUsed);
    }
    freeMsg(message);
  }
}

static void errDump(char* buff, int bytes){
  int i;
  fprintf(stderr, "errDump\t:\t %d bytes read:\n", bytes);
  for(i = 0; i < bytes; i++)
    fprintf(stderr, "[%d]", buff[i]);
  fprintf(stderr, "\n");
}

static void echoChunk(int from, int to){
  char buff[BUFFSZ];
  int bytesI, bytesO;
  if((bytesI = read(from, buff, BUFFSZ)) <= 0){
    announce("echo(%d,%d)\t:\terror bytesI = %d\n", from, to, bytesI);
    return;
  }
  if(self_debug_flag)errDump(buff, bytesI);
  if((bytesO = write(to, buff, bytesI)) != bytesI){
    announce("echo(%d,%d)\t:\terror bytes0 != bytesI (%d != %d)\n", from, to, bytesO, bytesI);
    return;
  }
}

static void reverberate(int from, int to){
  fd_set readset;
  struct timeval delay;
  int retval, iteration = 0;
  while(1){
    FD_ZERO(&readset);
    FD_SET(from, &readset);
    delay.tv_sec = 1;
    delay.tv_usec = 0;
    retval = select(from + 1, &readset, NULL, NULL, &delay);
    if(retval <= 0){
      announce("reverberate(%d,%d)\t:\tbreaking retval = %d\n", from, to, retval);
      break;
    } else {
      announce("reverberate(%d,%d)\t:\titeration = %d\n", from, to, iteration);
      echoChunk(from, to);
      iteration++;
    }
  }/* while */
}

void wait4IO(int fdout, int fderr, void (*fp)(int , int)){
  int maxfd = (fderr < fdout) ? fdout + 1 : fderr + 1;
  fd_set readset;
  int retval;
  announce("entering wait4IO\n"); 
  FD_ZERO(&readset);
  FD_SET(fdout, &readset);
  FD_SET(fderr, &readset);
  retval = select(maxfd, &readset, NULL, NULL, NULL);
  announce("wait4IO\t:\tselect returned %d (out: %d) (err: %d)\n", 
	   retval, FD_ISSET(fdout, &readset), FD_ISSET(fderr, &readset));
  if(retval < 0){
    fprintf(stderr, "wait4IO\t:\tselect error\n");
  } else if(retval == 0){
    fprintf(stderr, "wait4IO\t:\tselect returned 0\n");
  } else {
    if(FD_ISSET(fderr, &readset)){
      struct timeval delay;
      reverberate(fderr, STDERR_FILENO);
      FD_ZERO(&readset);
      FD_SET(fdout, &readset);
      FD_SET(fderr, &readset);
      delay.tv_sec = 1;
      delay.tv_usec = 0;
      retval = select(maxfd, &readset, NULL, NULL, &delay);
      if(retval <= 0) 
	goto exit;
      else if((retval == 2) || FD_ISSET(fderr, &readset)){
	fprintf(stderr, "wait4IO\t:\tthis shouldn't happen\n");
	wait4IO(fdout, fderr, fp);
	goto exit;
      } else {
	/* OK */
      }
    }
    if(FD_ISSET(fdout, &readset)){ fp(fdout, STDOUT_FILENO); }
  }

 exit:
  announce("exiting wait4IO(pout[0], perr[0],fp(fdout,STDOUT_FILENO));\n"); 
  return;
}

void echo2Input(int from, int to){
  msg* message;
  message = acceptMsg(from);
  if(message != NULL){
    writeMsg(to, message);
    if(self_debug_flag){
      writeMsg(STDERR_FILENO, message);
      announce("echo2Input: wrote %d bytes\n", message->bytesUsed);
    }
    freeMsg(message);
  }
}
