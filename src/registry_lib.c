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
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "cheaders.h"
#include "constants.h"
#include "types.h"
#include "registry_lib.h"
#include "iop_lib.h"
#include "socket_lib.h"
#include "msg.h"
#include "dbugflags.h"
#include "argv.h"

static char javaErrorsFileName[PATH_MAX];
static char errorsFileName[PATH_MAX];
static FILE *errorsFile;
static char cr = '\n';
static char selectedActor[SIZE];
static int  selected = 0;

static pthread_mutex_t theRegistryMutex = PTHREAD_MUTEX_INITIALIZER;
static actor_id  **theRegistry;
static int theRegistrySize = REGISTRYSZ;
static pthread_mutex_t iop_errlog__mutex = PTHREAD_MUTEX_INITIALIZER;

extern int iop_no_windows_flag;
extern int iop_hardwired_actors_flag;
extern int iop_debug_flag;
extern char* registry_fifo_in;
extern char* registry_fifo_out;
extern char* iop_bin_dir;
extern pid_t iop_pid;
extern int reg2InPort;

/* statics */
static int wait4ReadyFromInputWindow(int);
static int sendMsg2Input(msg *, output_cmd_t);
static actor_id *makeActorId(actor_spec*);
static int initActorId(actor_id*);
static void outputRegistry(int);
static void outputRegistrySize(int);
static int registerActor(actor_spec*);
static int unregisterActor(int);
static void sendActor(actor_id*, char*);
static actor_id *getActorBySlot(int);
static actor_id *getActorByName(char*);
static int getActorsSlotByName(char *);
static void* errorLog(void*);
static void* echoOut(void*);
static void parseOut(msg*);
static char* cmd2str(registry_cmd_t);
static void shutdownActor(actor_id *);
static void deleteActor(actor_id*);
static void shutdownRegistry();
static void freeActor(actor_id*);

void log2File(const char *format, ...){
  va_list arg;
  va_start(arg, format);
  if(format == NULL){
    va_end(arg);
  } else {
    pthread_mutex_lock(&iop_errlog__mutex);
    errorsFile = fopen(errorsFileName, "a");
    if(errorsFile != NULL){
      vfprintf(errorsFile, format, arg);
      fflush(errorsFile);
      fclose(errorsFile);
    }
    pthread_mutex_unlock(&iop_errlog__mutex);
    va_end(arg);
  }
  return;
}

static void registry_sig_handler(int sig){
  announce("Got signal %d\n", sig);
  if((sig == SIGUSR1) || (sig == SIGSEGV)) {
    bail();
    exit(EXIT_FAILURE);
  } 
}

static void registry_sigchld_handler(int sig){
  int status;
  pid_t child = waitpid(-1, &status, WNOHANG);
  announce("waited on child with pid %d with exit status %d\n",  child, status);
}

int registry_installHandler(){
  struct sigaction sigactInt;
  struct sigaction sigactSegv;
  struct sigaction sigactChld;
  sigset_t sigmask;
  sigemptyset(&sigmask);
  sigaddset(&sigmask, SIGINT);
  sigprocmask(SIG_BLOCK, &sigmask, NULL);
  sigactInt.sa_handler = registry_sig_handler;
  sigactInt.sa_flags = 0;
  sigfillset(&sigactInt.sa_mask);
  sigaddset(&sigactInt.sa_mask, SIGINT);
  if(sigaction(SIGUSR1, &sigactInt, NULL) != 0)
    return -1;
  sigactSegv.sa_handler = registry_sig_handler;
  sigactSegv.sa_flags = 0;
  sigfillset(&sigactSegv.sa_mask);
  if(sigaction(SIGSEGV, &sigactSegv, NULL) != 0)
    return -1;
  sigactChld.sa_handler = registry_sigchld_handler;
  sigactChld.sa_flags = 0;
  sigfillset(&sigactChld.sa_mask);
  return sigaction(SIGCHLD, &sigactChld, NULL);
}


int makeRegistryFifos(){
  announce("Unlinking %s\n", registry_fifo_in);  
  /* try and clean up old copies */
  unlink(registry_fifo_in);
  announce("Creating %s\n", registry_fifo_in);  
  /* make new ones               */
  if(mkfifo(registry_fifo_in,  S_IRWXU) < 0)    
    goto fail;
  announce("Unlinking %s\n", registry_fifo_out);  
  /* try and clean up old copies */
  unlink(registry_fifo_out);
  announce("Creating %s\n", registry_fifo_out);  
  /* make new ones               */
  if(mkfifo(registry_fifo_out, S_IRWXU) < 0)    
    goto fail;
  
  return 0;
  
 fail:
  
  fprintf(stderr, "Failure in makeRegistryFifos: %s\n", strerror(errno));
  return -1;
}

int registryInit(int *fifo_in_fd, int *fifo_out_fd){

  announce("registryInit locking mutex\n");
  pthread_mutex_lock(&theRegistryMutex);
  announce("registryInit locked mutex\n");  
  announce("Opening %s\n", registry_fifo_in);  
  if((*fifo_in_fd = open(registry_fifo_in, O_RDWR)) < 0)  
    goto fail;
  announce("Opening %s\n", registry_fifo_out);  
  if((*fifo_out_fd = open(registry_fifo_out, O_RDWR)) < 0) 
    goto fail;
  announce("Callocing\n");  
  theRegistry = (actor_id**)calloc(theRegistrySize, sizeof(actor_id*));
  assert(theRegistry != NULL);
  announce("registryInit unlocking mutex\n");  
  pthread_mutex_unlock(&theRegistryMutex);
  announce("registryInit unlocked mutex\n");  
  if(theRegistry == NULL) goto fail;
  announce("Registry successfully initialized\n");  
  return 0;

 fail:

  fprintf(stderr, "Failure in registryInit: %s\n", strerror(errno));
  return -1;
}


int errorsInit(){
  sprintf(errorsFileName, "/tmp/iop_%d_c_errors", iop_pid);
  sprintf(javaErrorsFileName, "/tmp/iop_%d_java_errors", iop_pid);
  announce("IOP's error file is: %s\n", errorsFileName);
  errorsFile = fopen(errorsFileName, "w");
  if(errorsFile == NULL){
    perror("Error file couldn't be initialized");
    return -1;
  }
  fclose(errorsFile);
  return 0;
}


static actor_id *makeActorId(actor_spec *acts){
  int i;
  actor_id *retval;
  char *reason;
  if(acts == NULL){  
    reason = "acts == NULL";
    goto fail;
  }
  announce("makeActorId commencing for %s\n", acts->name);
  retval = (actor_id *)calloc(1, sizeof(actor_id));
  if(retval == NULL){  
    reason = "calloc failed";
    goto fail; 
  }

  retval->spec = acts;
  retval->exitFlag = 0;
  for(i = 0; i < 3; i++)
    retval->fds[i] = -1;
  
  announce("makeActorId finished  %s\n", acts->name);

  return retval;

 fail:
  fprintf(stderr, "Failure in makeActorId: %s\n", reason);
  if(acts != NULL)
    fprintf(stderr, "Actor name = %s\n", acts->name);
  return NULL;

}

static int initActorId(actor_id *actid){
  int i, flags[3] = { O_RDWR, O_RDWR,  O_RDWR };
  actor_spec *aspec = NULL;
  if(actid == NULL){  
    goto fail;
  }
  aspec = actid->spec;
  announce("initActorId commencing for %s\n", aspec->name);
  pthread_mutex_init(&(actid->mutex), NULL);
  for(i = 0; i < 3; i++){
    actid->fds[i] = open(aspec->fifos[i], flags[i]);
    if(actid->fds[i] == -1) goto fail;
  }
  announce("initActorId launching errorLog thread  for %s\n", aspec->name);
  if((pthread_create(&(actid->tids[0]), NULL, errorLog, actid) != 0)){
    goto fail;
  }
  announce("initActorId launching echoOut thread  for %s\n", aspec->name);
  if((pthread_create(&(actid->tids[1]), NULL, echoOut, actid) != 0)){
    goto fail;
  }
  announce("initActorId finished  %s\n", aspec->name);

  return 1;

 fail:
  fprintf(stderr, "Failure in initActorId: %s\n", strerror(errno));
  fprintf(stderr, "Actor name = %s\n", aspec->name);
  return -1;

}

static void outputRegistry(int fd){
  int i, count = 0;
  announce("outputRegistry locking mutex\n");
  pthread_mutex_lock(&theRegistryMutex);
  announce("outputRegistry locked mutex\n");


  for(i = 0; i < theRegistrySize; i++)
    if(theRegistry[i] != NULL){
      if(theRegistry[i]->spec->pid > 0){ count++; }
    }
  
  if(writeInt(fd, count) < 0){
    goto exit;
  }
  announce("sent the count %d\n", count);

  for(i = 0; i < theRegistrySize; i++)
    if(theRegistry[i] != NULL){
      if(theRegistry[i]->spec->pid > 0){
	char *name = theRegistry[i]->spec->name;
	int lenN = strlen(name);
	if(writeInt(fd, i) < 0) goto exit;
	announce("sent i =  %d\n", i);
	if(write(fd, name, lenN) != lenN)
	  goto exit;
	if(write(fd, &cr, sizeof(char)) != sizeof(char))
	  goto exit;
	announce("sent name =  %s\n", name);
      }
    }

 exit:
  
  announce("outputRegistry unlocking mutex\n");
  pthread_mutex_unlock(&theRegistryMutex);
  announce("outputRegistry unlocked mutex\n");
  return;
}

static void outputRegistrySize(int fd){
  int i, count = 0;
  announce("outputRegistrySize locking mutex\n");
  pthread_mutex_lock(&theRegistryMutex);
  announce("outputRegistrySize locked mutex\n");  

  for(i = 0; i < theRegistrySize; i++)
    if(theRegistry[i] != NULL){
      if(theRegistry[i]->spec->pid > 0){
	count++;
      }
    }
  
  if(writeInt(fd, count) < 0) goto exit;
  
 exit:
  
  announce("outputRegistrySize unlocking mutex\n");
  pthread_mutex_unlock(&theRegistryMutex);
  announce("outputRegistrySize unlocked mutex\n");
  return;
}

/* 
   Not the most efficient, but at least reliable. Make sure the new actor
   has a unique name. If it doesn't it adds a number as a suffix, till it
   becomes unique.
*/

static int _allocateUniqueName(actor_spec *acts){
  static int requestNo = -1;
  char *oldName = acts->name;
  int i, found = 0;
  requestNo++;
  announce("_allocateUniqueName(%d): nameIn  = %s\n", requestNo, oldName);

  for(i = 0; i < theRegistrySize; i++){
    if((theRegistry[i] != NULL) && (strcmp(theRegistry[i]->spec->name, oldName) == 0)){
      found = 1;
      break;
    }
  }
  if(!found){ 
    announce("_allocateUniqueName(%d): nameOut  = %s\n", requestNo, oldName);
    return 1;
  } else {
    int len = strlen(oldName) + 1;
    char *newName = (char *)calloc(len + SIZE, sizeof(char));
    int index = -1;
    if(newName == NULL){ return -1; }
    if(len + SIZE > PATH_MAX){ return -1; }
    while(found == 1){
      index++;
      sprintf(newName, "%s%d", oldName, index);
      found = 0;
      for(i = 0; i < theRegistrySize; i++){
	if((theRegistry[i] != NULL) && (strcmp(theRegistry[i]->spec->name, newName) == 0)){
	  found = 1;
	  break;
	}
      }
    }/* while(found == 1) */
    strcpy(acts->name, newName);
    announce("_allocateUniqueName(%d): nameOut  = %s\n", requestNo, newName);
    free(newName);
    return 1;
  }
}

static int _getEmptyRegistrySlot(){
  int retval = -1, i;
  for(i = 0; i < theRegistrySize; i++){
    if(theRegistry[i] == NULL){
      retval = i;
      return retval;
    }
  }
  if(retval < 0){
    int theNewRegistrySize = 2 * theRegistrySize;
    actor_id  **theNewRegistry =  (actor_id**)calloc(theNewRegistrySize, sizeof(actor_id*));
    if(theNewRegistry == NULL){ return -1; }
    for(i = 0; i < theRegistrySize; i++)
      theNewRegistry[i] = theRegistry[i];
    retval = theRegistrySize;
    free(theRegistry);
    theRegistry = theNewRegistry;
    theRegistrySize = theNewRegistrySize;
  }
  return retval;
}

static int registerActor(actor_spec *acts){
  int retval = -1, slot, errcode;
  actor_id* actid;
  
  announce("Inside registerActor\n");

  announce("Calling makeActorId\n");
  actid = makeActorId(acts);
  announce("Called makeActorId\n");

  if(actid == NULL){
    fprintf(stderr, "registerActor failed,  makeActorId returned NULL\n");
    return retval;
  }

  announce("Calling initActorId\n");
  errcode = initActorId(actid);
  announce("Called initActorId\n");

  if(errcode == -1){
    fprintf(stderr, "registerActor failed,  initActorId returned -1\n");
    return retval;
  }
  

  announce("registerActor locking mutex\n");
  pthread_mutex_lock(&theRegistryMutex);
  announce("registerActor locked mutex\n");
  
  announce("allocating unique name\n");
  
  if(_allocateUniqueName(acts) < 0){
    fprintf(stderr, "_allocateUniqueName failed (name = %s)!\n", acts->name);
    goto exit;
  }
  announce("allocated unique name\n");

  slot = _getEmptyRegistrySlot();

  announce("got empty slot\n");
  
  if(slot < 0){ 
    goto exit;    
  } else {
    theRegistry[slot] = actid;
    retval = slot;
    goto exit;
  }

 exit:
  
  announce("registerActor unlocking mutex\n");
  pthread_mutex_unlock(&theRegistryMutex);
  announce("registerActor unlocked mutex\n");
  return retval;
}



static int unregisterActor(int slot){
  int retval = -1;
  actor_id *act = NULL;
  if((slot < 0) || (slot >= theRegistrySize)) return retval;
  announce("unregisterActor locking mutex\n");  
  pthread_mutex_lock(&theRegistryMutex);
  announce("unregisterActor locked mutex\n");  
  
  act = theRegistry[slot];
  theRegistry[slot] = NULL;
  retval = slot;
  announce("unregisterActor unlocking mutex\n");  
  pthread_mutex_unlock(&theRegistryMutex);
  announce("unregisterActor unlocked mutex\n");  
  return retval;
}


static actor_id *getActorBySlot(int slot){
  actor_id *retval = NULL;
  announce("getActorBySlot locking mutex\n");  
  pthread_mutex_lock(&theRegistryMutex);
  announce("getActorBySlot locked mutex\n");  
  retval = theRegistry[slot];
  announce("getActorBySlot unlocking mutex\n");  
  pthread_mutex_unlock(&theRegistryMutex);
  announce("getActorBySlot unlocked mutex\n");  
  return retval;
}

static actor_id *getActorByName(char *name){
  actor_id *retval = NULL;
  int i;
  if(name == NULL) return NULL;
  announce("getActorByName locking mutex\n");  
  pthread_mutex_lock(&theRegistryMutex);
  announce("getActorByName locked mutex\n");  
  for(i = 0; i < theRegistrySize; i++){
    if(theRegistry[i] == NULL) continue;
    if(theRegistry[i]->spec->name == NULL) continue;
    if(!strcmp(theRegistry[i]->spec->name, name)){ 
      retval =  theRegistry[i];
      goto exit;
    }
  }

 exit:
  announce("getActorByName unlocking mutex\n");  
  pthread_mutex_unlock(&theRegistryMutex);
  announce("getActorByName unlocked mutex\n");  
  return retval;
}

static int getActorsSlotByName(char *name){
  int retval = -1, i;
  if(name == NULL) return retval;
  announce("getActorsSlotByName locking mutex\n");  
  pthread_mutex_lock(&theRegistryMutex);
  announce("getActorsSlotByName locked mutex\n");  
  for(i = 0; i < theRegistrySize; i++){
    if(theRegistry[i] == NULL) continue;
    if(theRegistry[i]->spec->name == NULL) continue;
    if(!strcmp(theRegistry[i]->spec->name, name)){ 
      retval =  i;
      goto exit;
    }
  }

 exit:
  announce("getActorsSlotByName unlocking mutex\n");  
  pthread_mutex_unlock(&theRegistryMutex);
  announce("getActorsSlotByName unlocked mutex\n");  

  return retval;
  
}

static void sendActor(actor_id *rcv, char* buff){
  if(rcv == NULL){
    return;
  } else {
    int bytes = strlen(buff);
    pthread_mutex_lock(&(rcv->mutex));
    if(writeInt((rcv->fds)[IN], bytes) != 1){
      perror("WriteInt is sendActor failed\n");
      goto exit;
    }
    if(write((rcv->fds)[IN], buff, bytes) != bytes){
      perror("Write is sendActor failed\n");
      goto exit;
    }
    if(VERBOSE)
      fprintf(stderr, "Sent %s to %s\n", buff, rcv->spec->name);
  }
 exit:
  pthread_mutex_unlock(&(rcv->mutex));

}

static void* errorLog(void *arg){
  actor_id *act = (actor_id *)arg;
  msg *errmsg = NULL;
  int fd, failures = 0;
  if(act == NULL){
    pthread_exit(NULL);
    return NULL; 
  }
  fd = act->fds[ERR];
  while(1){

    if(act->exitFlag){
      announce("errorLog thread of %s exiting gracefully\n", act->spec->name);
      pthread_exit(NULL);
    }
 
    if((errmsg = readMsgVolatile(fd, &act->exitFlag)) == NULL){
      announce("readMsgVolatile in errorLog returned NULL\n");
      if(++failures > 5){
	announce("errorLog giving up!\n");
	pthread_exit(NULL); 
      };
      continue;
    }

    failures = 0;

    if(!iop_no_windows_flag){
      sendMsg2Input(errmsg, ERROR);
    } else {
      fprintf(stderr, "%s\n", errmsg->data);
    }
    
    log2File("\n%s wrote %d bytes to STDERR_FILENO\n", act->spec->name, errmsg->bytesUsed);
    freeMsg(errmsg);

  } /* while(1) */

  pthread_exit(NULL);
#ifdef _MAC
  return NULL;
#endif
}



static void* echoOut(void *arg){
  actor_id *act = (actor_id *)arg;
  msg *outmsg;
  int fd = (act->fds)[OUT];  
  int buffno = 1;
  
  announce("echoOut for %s commencing\n", act->spec->name);


  while(1){
    if(act->exitFlag){
      announce("echoOut thread of %s exiting gracefully\n", act->spec->name);
      pthread_exit(NULL);
    }
    announce("echoOut for %s waiting\n", act->spec->name);

    if((outmsg = acceptMsgVolatile(fd, &act->exitFlag)) == NULL){
      announce("acceptMsgVolatile in echoOut for %s returned NULL\n", act->spec->name);
      continue;
    }
    announce("\nbuffer no of %s: %d\n", act->spec->name, buffno++);
    announce("calling parseOut outmsg size = %d\n", outmsg->bytesUsed);
    announce("calling parseOut outmsg = %s\n", outmsg->data);
    parseOut(outmsg);
    announce("called parseOut\n");
    freeMsg(outmsg);
    announce("freed outmsg\n");
  }
}

static void parseOut(msg* outmsg){
  char *copy = NULL;
  char *parsedmsg = NULL;
  char *target, *sender, *rest;
  actor_id *recipient;
  announce("commencing parseOut\n");
  if((copy = (char *)calloc(outmsg->bytesUsed + 1, sizeof(char))) == NULL)
    goto fail;
  strcpy(copy, outmsg->data);
  if(getNextToken(copy, &target, &rest) != 1)
    goto echo;
  announce("target = \"%s\"\n", target);
  announce("calling getActorByName\n");
  recipient = getActorByName(target);
  announce("recipient == NULL: %d\n", recipient == NULL);
  if(recipient == NULL)
    goto echo;
  if(recipient->spec->pid <= 0)
    goto echo;
  if(getNextToken(rest, &sender, &rest) != 1)
    goto echo;
  announce("sender = \"%s\"\n", sender);
  if(rest == NULL)
    goto echo;
  if((parsedmsg = (char *)calloc((outmsg->bytesUsed + 4), sizeof(char))) == NULL)
    goto fail;

  sprintf(parsedmsg, "(%s %s)", sender, rest);


  announce("parsedmsg = \"%s\"\n", parsedmsg);
  sendActor(recipient, parsedmsg);
  free(copy);
  free(parsedmsg);
  return;
  
 echo:
  
  if(!iop_no_windows_flag){
    sendMsg2Input(outmsg, OUTPUT);
  } else {
    /* used to be to stdout, but now that the registry is an actor....*/
    fprintf(stderr, "%s\n", outmsg->data);
    fflush(stderr);
  }

  free(copy);
  free(parsedmsg);
  return;
  
 fail:
  fprintf(stderr, "parseOut failed");
  free(copy);
  free(parsedmsg);
  return;
}

static char* cmd2str(registry_cmd_t cmd){
  switch(cmd){
  case SEND:       return "SEND";
  case REGISTER:   return "REGISTER";
  case UNREGISTER: return "UNREGISTER";
  case DUMP:       return "DUMP";
  case KILL:       return "KILL";
  case NAME:       return "NAME";
  case RSIZE:      return "RSIZE";
  default:         return "UNKNOWN";
  }
}


static void shutdownActor(actor_id *act){
  int i;
  for(i = 0; i < 2; i++){
    /*    turing LINUX bug                     */
    /*    pthread_kill(act->tids[i], SIGKILL); */

  }
  if(act->spec->pid > 0){
    for(i = 0; i < 3; i++){
      if(act->fds[i] > 0){ close(act->fds[i]); }
      if(unlink(act->spec->fifos[i]) < 0){
	perror("Unlink in shutdownActor failed");
      }
    }
    kill(act->spec->pid, SIGINT);
  }
}

void deleteActor(actor_id *act){
  int i;
  act->exitFlag = 1;
  if(act->spec->pid > 0){
    for(i = 0; i < 3; i++){
      if(act->fds[i] > 0){ close(act->fds[i]); }
      if(unlink(act->spec->fifos[i]) < 0){
	fprintf(stderr, 
		"Unlink of %s in deleteActor failed: %s\n", 
		act->spec->fifos[i], strerror(errno));
      }
    }
  }
}

static void shutdownRegistry(){
  announce("shutdownRegistry locking mutex\n");  
  pthread_mutex_lock(&theRegistryMutex);
  announce("shutdownRegistry locked mutex\n");  

  bail();
  announce("shutdownRegistry unlocking mutex\n");  
  pthread_mutex_unlock(&theRegistryMutex);
  announce("shutdownRegistry unlocked mutex\n");  
  exit(EXIT_SUCCESS);
}

void bail(){
  int i;
  for(i = 0; i < theRegistrySize; i++){
    if(theRegistry[i] != NULL)
      shutdownActor(theRegistry[i]);
  }
  if(unlink(registry_fifo_in) < 0){
    perror("Unlink of REGISTRY_FIFO_IN failed");
  }
  if(unlink(registry_fifo_out) < 0){
    perror("Unlink of REGISTRY_FIFO_IN failed");
  }
  
  if(unlink(errorsFileName) < 0){
    perror("Unlink of errorsFileName failed");
  }

  if(!iop_no_windows_flag){
    if(unlink(javaErrorsFileName) < 0){
      perror("Unlink of javaErrorsFileName failed");
    }
  } else {
    /* no windows, better kill chatter */
    kill(iop_pid, SIGKILL);
  }
}


static void freeActor(actor_id* act){
  if(act == NULL) return;
  free(act->spec);
  free(act);
}

void processSendCommand(int inFd, int outFd){
  int bytesin, actorId;
  char *buffin;
  actor_id* target;
  if(readInt(inFd, &actorId) < 0)
    return;
  announce("SEND clause, actorId = %d\n", actorId);
  if(readInt(inFd, &bytesin) < 0)
    return;
  announce("SEND clause, bytesin = %d\n", bytesin);
  if((buffin = (char *)calloc(bytesin + 1, sizeof(char))) == NULL){
    fprintf(stderr, "SEND clause, calloc of buffin failed\n");
    return;
  }
  if(read(inFd, buffin, bytesin) != bytesin){
    fprintf(stderr, "SEND clause, read into buffin failed\n");
    free(buffin);
    return;
  }
  announce("buffin[bytesin - 1] == NULL : %d\n", buffin[bytesin - 1] == '\0');
  buffin[bytesin] = '\0';
  announce("SEND clause, buffin = \"%s\"\n", buffin);
  if((target = getActorBySlot(actorId)) == NULL){
    fprintf(stderr, "SEND clause, getActorBySlot failed\n");
    free(buffin);
    return;
  }
  announce("SEND clause, target = %s\n", target->spec->name);
  if((writeInt((target->fds)[IN], bytesin)) != 1){
    fprintf(stderr, "SEND clause, writeInt failed\n");
    free(buffin);
    return;
  }
  if((write((target->fds)[IN], buffin, bytesin)) != bytesin){ 
    fprintf(stderr, "SEND clause, write failed\n");
    free(buffin);
    return;
  }
  announce("SEND clause, send OK\n");
  free(buffin);
  return;
}

void processRegisterCommand(int inFd, int outFd, int notifyGUI){
  int slotNumber;
  actor_spec *acts = NULL;
  announce("Reading actor spec\n");
  if((acts =  readActorSpec(inFd)) == NULL){
    fprintf(stderr, "Reading actor spec failed\n");
    return; 
  }
  announce("Registering actor spec\n");
  if((slotNumber = registerActor(acts)) == -1)
    return;
  
  announce("Replying with slot %d\n", slotNumber);

  if(writeInt(outFd, slotNumber) < 0){
    fprintf(stderr, "REGISTER clause, write of slotNumber failed\n");
    return;
  }
  announce("sent slotNumber = %d\n", slotNumber);
  
  if(notifyGUI){
    /* 
       the system is the first actor now, lots of race conditions here!  
       simplest way to fix this is have the system actor get registered 
       first, and not try and notify the no-existent GUI.
    */
    if(!iop_no_windows_flag){
      announce("notifying GUI\n");
      sendMsg2Input(NULL, UPDATE);
      announce("notified GUI\n");
    }
  }


  announce("breaking\n");
  return;
}

void processUnregisterCommand(int inFd, int outFd){
  char name[PATH_MAX + 1];
  int len, slotNumber;
  actor_id* victim;
  announce("UNREGISTER clause, Reading actor name\n");
  if((len = read(inFd, name, PATH_MAX)) <= 0)
    return; 
  name[len] ='\0';
  announce("UNREGISTER clause, Registering actor name: %s\n", name);
  if((slotNumber = getActorsSlotByName(name)) == -1)
    return;
  if((victim = getActorBySlot(slotNumber)) == NULL){
    fprintf(stderr, "UNREGISTER clause, getActorBySlot failed\n");
    return;
  }
  if(unregisterActor(slotNumber) == -1)
    return;
  announce("UNREGISTER clause, found actor %s in slotNumber = %d\n", name, slotNumber);
  if(writeInt(outFd, slotNumber) < 0){
    fprintf(stderr, "UNREGISTER clause, write of slotNumber failed\n");
    return;
  }
  announce("UNREGISTER clause, sent slotNumber = %d\n", slotNumber);
  /* give victim time to exit by relinquishing CPU */
  usleep(1);
  if(victim != NULL){
    deleteActor(victim);
    freeActor(victim);
  }

  if(!iop_no_windows_flag){
    announce("notifying GUI\n");
    sendMsg2Input(NULL, UPDATE);
    announce("notified GUI\n");
  }
  announce("UNREGISTER clause, breaking\n");
  return;

}

void processNameCommand(int cmd, int inFd, int outFd){
  int actorId, len;
  actor_id* target;
  char unk[] = UNKNOWNNAME;
  if(readInt(inFd, &actorId) < 0) goto fail;
  if((target = getActorBySlot(actorId)) == NULL){
    announce("NAME clause, getActorBySlot failed\n");
    goto fail;
  }
  write(outFd, target->spec->name, strlen(target->spec->name));
  return;

 fail:
  len = strlen(unk);
  if(write(outFd, unk, len) != len){
    fprintf(stderr, "write(outFd, unk, strlen(unk)) failed\n");
  };
  announce("command index = %d\n", cmd);
  return;
}

void processRegistryCommand(int inFd, int outFd, int notifyGUI){
  int cmd = -1;
  announce("Awaiting a command \n");
  if(readInt(inFd, &cmd) < 0){
    fprintf(stderr, "Read of cmd failed\n");
    return;
  }
  announce("Processing %s command (cmd = %d)\n", cmd2str(cmd), cmd);
  switch(cmd){
  case SEND : {
    processSendCommand(inFd, outFd);
    break;
  }
  case REGISTER : {
    processRegisterCommand(inFd, outFd, notifyGUI);
    break;
  }
  case UNREGISTER : {
    processUnregisterCommand(inFd, outFd);
    break;
  }
  case DUMP : {
    outputRegistry(outFd);
    break;
  }
  case KILL : {
    shutdownRegistry();
    break;
  }
  case NAME : {
    processNameCommand(cmd, inFd, outFd);
    break; 
  }
  case RSIZE : {
    outputRegistrySize(outFd);
    break;
  }
  default: return;
  }
}

void *monitorInSocket(void *arg){
  int listeningSocket = *((int *)arg);

  announce("Waiting notification of reg2InPort\n");
  if((reg2InPort = wait4ReadyFromInputWindow(listeningSocket)) < 0){
    fprintf(stderr, "wait4ReadyFromInputWindow failed\n");
    bail();
  }
  announce("reg2InPort = %d\n", reg2InPort);


  while(1){
    int  *msgsock;
    char *description = NULL;
    announce("monitorInSocket blocking on acceptSocket\n");
    msgsock = acceptSocket(listeningSocket, &description);
    if (*msgsock == INVALID_SOCKET){
      fprintf(stderr, description);
      socketCleanUp();
      free(description); 
      pthread_exit(NULL);
}
    announce("%s -- *msgsock = %d\n", description, *msgsock);
    processRegistryCommand(*msgsock, *msgsock, 1);
    free(description); 
    closeSocket(*msgsock);
  }
}

static int wait4ReadyFromInputWindow(int in2regSocket){
  int  *msgsock, bytesread;
  char *description = NULL, buff[SIZE];
  announce("wait4ReadyFromInputWindow blocking on acceptSocket\n");
  msgsock = acceptSocket(in2regSocket, &description);
  if (*msgsock == INVALID_SOCKET){
    fprintf(stderr, description);
    socketCleanUp();
    free(description);
    return -1;
  }
  if((bytesread = read(*msgsock, buff, SIZE)) < 0){
    fprintf(stderr, "read failed in wait4ReadyFromInputWindow\n");
    return -1;
  }
  buff[bytesread] = '\0';
  free(description);
  return atoi(buff);
}

/* 
   WARNING: Putting active debug messages in this routine, and any routine
   it calls can cause LOOPING. Better to log it out to the C errors file!
*/
static int sendMsg2Input(msg *message, output_cmd_t type){
  int socket, retval = 0;
  while(reg2InPort < 0) sleep(1);
  if(allocateSocket(reg2InPort, "localhost", &socket) != 1){
    fprintf(stderr, "sendMsg2Input: allocateSocket failed\n");
    return retval;
  }
  /*
    type: one of:
    7 for error
    8 for output
    9  for update
    though this is not used at present
  */
  if(writeInt(socket, type) < 0){
    fprintf(stderr, "sendMsg2Input: writeInt failed\n");
    goto exit;
  }
  if(message != NULL){
    if(write(socket, message->data, message->bytesUsed) != message->bytesUsed){
      fprintf(stderr, "sendMsg2Input: truncated write to socket\n");      
      goto exit;
    }
  }
  retval = 1;
 exit:
  closeSocket(socket);
  return retval;
}


char* registryLaunchActor(char* name, int argc, char** argv){
  int slot, i, errcode;
  char* retval = NULL;
  char* executable;
  actor_id* actid;
  actor_spec *spec;
  announce("registryLaunchActor calling makeActorSpec for %s\n", name);  
  spec = makeActorSpec(name);
  announce("registryLaunchActor called makeActorSpec for %s\n", name);  
  announce("registryLaunchActor locking mutex\n");  
  pthread_mutex_lock(&theRegistryMutex);
  announce("registryLaunchActor locked mutex\n");  

  announce("registryLaunchActor calling _allocateUniqueName\n");  


  if(_allocateUniqueName(spec) < 0){
    fprintf(stderr, "_allocateUniqueName failed (name = %s)!\n", name);
    goto exit;
  }

  announce("registryLaunchActor calling _getEmptyRegistrySlot\n");  


  if((slot = _getEmptyRegistrySlot()) < 0){
    fprintf(stderr, "_getEmptyRegistrySlot() failed (name = %s)!\n", name);
    goto exit;
  }

  /* create the actor  -- careful with names etc */
  executable = argv[0];
  argv[0] = spec->name;

  for(i = 0; i < argc; i++){
    if(!strcmp(argv[i], FIFO_IN)){
      strcpy(argv[i], registry_fifo_in);
    } else if(!strcmp(argv[i], FIFO_OUT)){
      strcpy(argv[i], registry_fifo_out);
    } else if(!strcmp(argv[i], IOP_BIN_DIR)){
      strcpy(argv[i], iop_bin_dir);
    }
  }

  announce("registryLaunchActor calling newActor\n");  
  
  spec = newActor(0, executable,  argv);
  argv[0] = executable;
  if(spec == NULL){
    fprintf(stderr, "registryLaunchActor: newActor failed (name = %s)!\n", name);
    goto exit;
  }

  /*
    fprintf(stderr, "%s had pid_t %d\n", spec->name, spec->pid);
  */

  announce("registryLaunchActor calling makeActorId\n");  
  actid = makeActorId(spec);
  announce("registryLaunchActor called makeActorId\n");  

  if(actid == NULL){
    fprintf(stderr, "registryLaunchActor: makeActorId failed (name = %s)!\n", name);
    goto exit;
  }

  announce("Calling initActorId\n");
  errcode = initActorId(actid);
  announce("Called initActorId\n");

  if(errcode == -1){
    fprintf(stderr, "registryLaunchActor: initActorId returned -1\n");
    goto exit;
  }

  theRegistry[slot] = actid;
  retval = spec->name;

 exit:

  announce("registryLaunchActor unlocking mutex\n");  
  pthread_mutex_unlock(&theRegistryMutex);
  announce("registryLaunchActor unlocked mutex\n");  
  announce("registryLaunchActor complete!\n");  
  return retval;

}


void  processRegistryStartMessage(char *sender, char *rest, int notify){
  char *name, *args;
  if(getNextToken(rest, &name, &args) != 1){
    fprintf(stderr, "processRegistryStartMessage: didn't understand: (cmd)\n\t \"%s\" \n", rest);
    return;
  } else {
    int argc;
    char**argv;
    char* actorName;
    argc = makeArgv(args, " \t\n", &argv);
    /*
      printArgv(stderr, argc, argv);
    */

    if(argc > 0){
      actorName = registryLaunchActor(name, argc, argv);
      if(actorName != NULL){
	if(sender != NULL){
	  announce("%s\n%s\nstartOK %s\n", sender, REGISTRY_ACTOR, actorName);
	  sendFormattedMsgFP(stdout, "%s\n%s\nstartOK %s\n", sender, REGISTRY_ACTOR, actorName);
	}
      } else {
	if(sender != NULL){
	  announce("%s\n%s\nstartFAILED %s\n", sender, REGISTRY_ACTOR, name);
	  sendFormattedMsgFP(stdout, "%s\n%s\nstartFAILED %s\n", sender, REGISTRY_ACTOR, name);
	}
      }
      freeArgv(argc, argv);

      if(notify && !iop_no_windows_flag){
	announce("processRegistryStartMessage notifying GUI\n");  
	sendMsg2Input(NULL, UPDATE);
	announce("processRegistryStartMessage notified GUI\n");  
      }

    }
  }
}

void  processRegistryStopMessage(char *sender, char *rest){
  char *name, *args;
  if(getNextToken(rest, &name, &args) != 1){
    fprintf(stderr, "processRegistryStopMessage: didn't understand: (cmd)\n\t \"%s\" \n", rest);
    return;
  } else {
    if(name == NULL){ 
      fprintf(stderr, "processRegistryStopMessage: name is NULL\n");
      return; 
    } else if(!strcmp(name, REGISTRY_ACTOR)){
      shutdownRegistry();
    } else {
      actor_id *actid = getActorByName(name);
      if(actid != NULL){
	int slot = getActorsSlotByName(name);
	if(slot >= 0){
	  announce("processRegistryStopMessage locking mutex\n");  
	  pthread_mutex_lock(&theRegistryMutex);
	  announce("processRegistryStopMessage locked mutex\n");  
	  theRegistry[slot] = NULL;
	  announce("processRegistryStopMessage unlocking mutex\n");  
	  pthread_mutex_unlock(&theRegistryMutex);
	  announce("processRegistryStopMessage unlocked mutex\n");  
	}
	shutdownActor(actid);  
	if(!iop_no_windows_flag){
	  sendMsg2Input(NULL, UPDATE);
	}
	announce("%s\n%s\nstopOK %s\n", sender, REGISTRY_ACTOR, name);
	sendFormattedMsgFP(stdout, "%s\n%s\nstopOK %s\n", sender, REGISTRY_ACTOR, name);
      } else {
	fprintf(stderr, "processRegistryStopMessage: actid of %s is NULL\n", name);
	announce("%s\n%s\nstopFAILED %s\n", sender, REGISTRY_ACTOR, name);
	sendFormattedMsgFP(stdout, "%s\n%s\nstopFAILED %s\n", sender, REGISTRY_ACTOR, name);
      }
    }
  }
}

void  processRegistrySelectMessage(char *sender, char *rest){
  char *name, *args;
  if(getNextToken(rest, &name, &args) != 1){
    fprintf(stderr, "processRegistrySelectMessage: didn't understand: (cmd)\n\t \"%s\" \n", rest);
    return;
  } else {
    if(name == NULL){ 
      fprintf(stderr, "processRegistrySelectMessage: name is NULL\n");
      return; 
    } else {
      int alen = strlen(name);
      msg *amsg = makeMsg(alen);
      addToMsg(amsg, alen, name);
      sendMsg2Input(amsg, SELECT);
      freeMsg(amsg);
    }
  }
}

void  processRegistryNameMessage(char *sender, char *rest){
  char *name, *args;
  int slot = -1;
  if(getNextToken(rest, &name, &args) != 1){
    fprintf(stderr, "processRegistryNameMessage: didn't understand: (cmd)\n\t \"%s\" \n", rest);
    goto exit; 
  } else {
    if(name == NULL){ 
      fprintf(stderr, "processRegistryNameMessage: name is NULL\n");
      goto exit; 
    } else {
      if(strlen(name) + 1 > PATH_MAX){
	fprintf(stderr, "processRegistryNameMessage: name is too long\n");
	goto exit; 
      } else {
	actor_id* actid;
	actor_spec *aspec = (actor_spec *)calloc(1, sizeof(actor_spec));
	if(aspec == NULL){
	  fprintf(stderr, "processRegistryNameMessage: callocing aspec failed!\n");
	  goto exit; 
	}  
	strcpy(aspec->name, name);
	aspec->pid = 0;
	announce("processRegistryNameMessage: calling makeActorId\n");
	actid = makeActorId(aspec);
	announce("processRegistryNameMessage: called makeActorId\n");
	
	if(actid == NULL){
	  fprintf(stderr, "processRegistryNameMessage:  makeActorId returned NULL\n");
	  goto exit;
	}
	
        announce("processRegistryNameMessage: locking mutex\n");
	pthread_mutex_lock(&theRegistryMutex);
	announce("processRegistryNameMessage: locked mutex\n");
	
	announce("processRegistryNameMessage: allocating unique name\n");
	
	if(_allocateUniqueName(aspec) < 0){
	  fprintf(stderr, "_allocateUniqueName failed (name = %s)!\n", aspec->name);
	  goto unlock;
	}
	announce("allocated unique name\n");
	
	slot = _getEmptyRegistrySlot();
	
	announce("got empty slot\n");
	
	if(slot >= 0){ 
	  theRegistry[slot] = actid;
	}
	
	
      unlock:
	
	announce("processRegistryNameMessage: unlocking mutex\n");
	pthread_mutex_unlock(&theRegistryMutex);
	announce("processRegistryNameMessage: unlocked mutex\n");
	
      exit:

	if(slot >= 0){
	  sendFormattedMsgFP(stdout, "%s\n%s\nnameOK %s %s %d\n", 
			     sender, REGISTRY_ACTOR, name, aspec->name, iop_pid);
	} else {
	  sendFormattedMsgFP(stdout, "%s\n%s\nnameFAILED %s\n", sender, REGISTRY_ACTOR, name);
	}

	return;

      }
    }
  }
}


int parseActorSpec(actor_spec* acts, char* args){
  int retval = -1;
  if(acts->pid > 0){
    fprintf(stderr, "parseActorSpec: acts already complete\n");
    goto exit;
  } else {
    int errcode = sscanf(args, 
			 "%d %s %s %s", 
			 &(acts->pid), 
			 acts->fifos[IN],
			 acts->fifos[OUT],
			 acts->fifos[ERR]);
    if(errcode != 4){
      fprintf(stderr, "parseActorId: args doesn't parse \"%s\"\n", args);
      goto exit;
    }
    
    retval = 1;

  }


 exit:
  return retval;
}

void  processRegistryEnrollMessage(char *sender, char *rest){
  char *name, *args;
  actor_id* subject = NULL;
  int slot = -1, errcode = -1;
  if(getNextToken(rest, &name, &args) != 1){
    fprintf(stderr, "processRegistryEnrollMessage: didn't understand: (cmd)\n\t \"%s\" \n", rest);
    goto exit;
  } else {
    if(name == NULL){ 
      fprintf(stderr, "processRegistryEnrollMessage: name is NULL\n");
      goto exit; 
    } else {
      if(strlen(name) + 1 > PATH_MAX){
	fprintf(stderr, "processRegistryEnrollMessage: name is too long\n");
	goto exit; 
      } else {
	if((slot = getActorsSlotByName(name)) == -1){
	  fprintf(stderr, "processRegistryEnrollMessage: getActorsSlotByName failed\n");
	  goto exit;
	}
	if((subject = getActorBySlot(slot)) == NULL){
	  fprintf(stderr, "processRegistryEnrollMessage: getActorBySlot failed\n");
	  goto exit;
	}
	if((errcode = parseActorSpec(subject->spec, args)) == -1){
	  fprintf(stderr, "processRegistryEnrollMessage: parseActorSpec failed\n");
	  goto exit;
	}
	if((errcode = initActorId(subject)) == -1){
	  fprintf(stderr, "processRegistryEnrollMessage: initActorId failed\n");
	  goto exit;
	}

	
      }
    }
  }

 exit:

  if((slot >= 0) && (subject != NULL) && (errcode != -1)){
    sendFormattedMsgFP(stdout, "%s\n%s\nenrollOK %s\n", sender, REGISTRY_ACTOR, name);
  } else {
    sendFormattedMsgFP(stdout, "%s\n%s\nenrollFAILED %s\n", sender, REGISTRY_ACTOR, name);
  }


}

void  processRegistryUnenrollMessage(char *sender, char *rest){
  char *name, *args;
  actor_id* victim = NULL;
  int slot = -1, errcode = -1;
  if(getNextToken(rest, &name, &args) != 1){
    fprintf(stderr, "processRegistryUnenrollMessage: didn't understand: (cmd)\n\t \"%s\" \n", rest);
    goto exit;
  } else {
    if(name == NULL){ 
      fprintf(stderr, "processRegistryUnenrollMessage: name is NULL\n");
      goto exit; 
    } else {
      if(strlen(name) + 1 > PATH_MAX){
	fprintf(stderr, "processRegistryUnenrollMessage: name is too long\n");
	goto exit; 
      } else {
	if((slot = getActorsSlotByName(name)) == -1)
	  goto exit;
	if((victim = getActorBySlot(slot)) == NULL){
	  fprintf(stderr, "processRegistryUnenrollMessage: getActorBySlot failed\n");
	  goto exit;
	}
	if((errcode = unregisterActor(slot)) == -1){
	  fprintf(stderr, "processRegistryUnenrollMessage: unregisterActor failed\n");
	  goto exit;
	}
	announce("processRegistryUnenrollMessage: found actor %s in slotNumber = %d\n", name, slot);
      }
    }
  }
  
 exit:

  if((slot >= 0) && (victim != NULL) && (errcode != -1)){
    fprintf(stderr, "%s\n%s\nunenrollOK %s\n", sender, REGISTRY_ACTOR, name);

    if(!iop_no_windows_flag){
      sendMsg2Input(NULL, UPDATE);
    }

  } else {
    fprintf(stderr, "%s\n%s\nunenrollFAILED %s\n", sender, REGISTRY_ACTOR, name);
  }
  
  if((victim != NULL) && (errcode != -1)){
    deleteActor(victim);
    freeActor(victim);
  }

  return;
}


void processRegistryMessage(char *sender, char *body){
  char *cmd, *rest;
  if(getNextToken(body, &cmd, &rest) != 1){
    fprintf(stderr, "processRegistryMessage: didn't understand: (cmd)\n\t \"%s\" \n", body);
    return;
  }
  if(!strcmp(cmd, REGISTRY_STATUS)){
    fprintf(stderr, "STATUS\n");
  } else if (!strcmp(cmd, REGISTRY_START)){
    processRegistryStartMessage(sender, rest, 1);
  } else if (!strcmp(cmd, REGISTRY_STOP)){
    processRegistryStopMessage(sender, rest);
  } else if (!strcmp(cmd, REGISTRY_SELECT)){
    processRegistrySelectMessage(sender, rest);
  } else if (!strcmp(cmd, REGISTRY_NAME)){
    processRegistryNameMessage(sender, rest);
  } else if (!strcmp(cmd, REGISTRY_ENROLL)){
    processRegistryEnrollMessage(sender, rest);
  } else if (!strcmp(cmd, REGISTRY_UNENROLL)){
    processRegistryUnenrollMessage(sender, rest);
  }
}



int registryProcessConfigFile(){
  char ioprc[BUFFSZ];
  char* homedir = getenv(HOME);
  if(homedir == NULL){
    fprintf(stderr, "Configuration file not found, getenv returned NULL.\n");
    return -1;
  } else {
    FILE* rcfile;
    int len = strlen(homedir);
    strncpy(ioprc, homedir, len + 1);
    strncat(ioprc, "/", 2);
    strncat(ioprc, IOPRC, BUFFSZ - len - 3);
    rcfile = fopen(ioprc, "r");
    if(rcfile == NULL){
      announce("Configuration file not opened: %s", strerror(errno));
      return -1;
    } else {
      char line[BUFFSZ];
      int counter = 0;
      while(fgets(line, BUFFSZ, rcfile) != NULL){
	int len;
	counter++;
	if((len = strlen(line)) == 0) continue;
	if(line[0] == '#') continue;
	if(line[len - 1] == '\n'){ line[len - 1] = '\0'; }
	if(strncmp(line, REGISTRY_START, strlen(REGISTRY_START)) == 0){
	  /* its probably a start command */
	  char *cmd, *rest;
	  if(getNextToken(line, &cmd, &rest) != 1){
	    fprintf(stderr, "registryProcessConfigFile: parsing %s failed\n", line);
	    continue;
	  }
	  if(!(strcmp(cmd, REGISTRY_START) == 0)){ 
	    fprintf(stderr, "registryProcessConfigFile: cmd %s not recognized\n", cmd);
	    continue;
	  } 
	  processRegistryStartMessage(NULL, rest, 0);
	} else if(strncmp(line, REGISTRY_SELECT, strlen(REGISTRY_SELECT)) == 0){
	  /* its probably a select command */
	  char *cmd, *name, *rest;
	  if(getNextToken(line, &cmd, &rest) != 1){
	    fprintf(stderr, "registryProcessConfigFile: parsing %s failed\n", line);
	    continue;
	  }
	  if(!(strcmp(cmd, REGISTRY_SELECT) == 0)){ 
	    fprintf(stderr, "registryProcessConfigFile: cmd %s not recognized\n", cmd);
	    continue;
	  } 
	  if(getNextToken(rest, &name, &rest) != 1){
	    fprintf(stderr, "registryProcessConfigFile: parsing %s failed\n", line);
	    continue;
	  }
	  announce("registryProcessConfigFile: selected actor = %s\n", name);
	  strncpy(selectedActor, name, SIZE);
	  selected = 1;
	}
      }
      
      if(!iop_no_windows_flag){
	announce("registryProcessConfigFile notifying GUI\n");  
	if(selected){
	  int alen = strlen(selectedActor);
	  msg *amsg = makeMsg(alen);
	  addToMsg(amsg, alen, selectedActor);
	  sendMsg2Input(amsg, SELECT);
	  freeMsg(amsg);
	  selected = 0;
	} else {
	  sendMsg2Input(NULL, UPDATE);
	}
	announce("registryProcessConfigFile notified GUI\n");  
      }
      return counter;
    }
  }
}
