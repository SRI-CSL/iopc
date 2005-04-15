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
static pthread_mutex_t iop_err_mutex = PTHREAD_MUTEX_INITIALIZER;

extern int iop_no_windows_flag;
extern int iop_hardwired_actors_flag;
extern int iop_debug_flag;
extern char* registry_fifo_in;
extern char* registry_fifo_out;
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


void rannounce(const char *format, ...){
  va_list arg;
  va_start(arg, format);
  if(format == NULL){
    va_end(arg);
  } else {
    if(REGISTRY_DEBUG || iop_debug_flag){
      pthread_mutex_lock(&iop_err_mutex);
      fprintf(stderr, "REGISTRY(%ld)\t:\t" , (long)pthread_self());
      vfprintf(stderr, format, arg);
      pthread_mutex_unlock(&iop_err_mutex);
    }
    va_end(arg);
  }
  return;
}

void log2File(const char *format, ...){
  va_list arg;
  va_start(arg, format);
  if(format == NULL){
    va_end(arg);
  } else {
    pthread_mutex_lock(&iop_err_mutex);
    errorsFile = fopen(errorsFileName, "a");
    if(errorsFile != NULL){
      vfprintf(errorsFile, format, arg);
      fflush(errorsFile);
      fclose(errorsFile);
    }
    pthread_mutex_unlock(&iop_err_mutex);
    va_end(arg);
  }
  return;
}

static void registry_sig_handler(int sig){
  rannounce("Got signal %d\n", sig);
  if((sig == SIGUSR1) || (sig == SIGSEGV)) {
    bail();
    exit(EXIT_FAILURE);
  } 
}

static void registry_sigchld_handler(int sig){
  int status;
  pid_t child = waitpid(-1, &status, WNOHANG);
  rannounce("waited on child with pid %d with exit status %d\n", 
	    child, status);
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
  rannounce("Unlinking %s\n", registry_fifo_in);  
  /* try and clean up old copies */
  unlink(registry_fifo_in);
  rannounce("Creating %s\n", registry_fifo_in);  
  /* make new ones               */
  if(mkfifo(registry_fifo_in,  S_IRWXU) < 0)    
    goto fail;
  rannounce("Unlinking %s\n", registry_fifo_out);  
  /* try and clean up old copies */
  unlink(registry_fifo_out);
  rannounce("Creating %s\n", registry_fifo_out);  
  /* make new ones               */
  if(mkfifo(registry_fifo_out, S_IRWXU) < 0)    
    goto fail;
  
  return 0;
  
 fail:
  
  fprintf(stderr, "Failure in makeRegistryFifos: %s\n", strerror(errno));
  return -1;
}

int registryInit(int *fifo_in_fd, int *fifo_out_fd){

  rannounce("registryInit locking mutex\n");
  pthread_mutex_lock(&theRegistryMutex);
  rannounce("registryInit locked mutex\n");  
  rannounce("Opening %s\n", registry_fifo_in);  
  if((*fifo_in_fd = open(registry_fifo_in, O_RDWR)) < 0)  
    goto fail;
  rannounce("Opening %s\n", registry_fifo_out);  
  if((*fifo_out_fd = open(registry_fifo_out, O_RDWR)) < 0) 
    goto fail;
  rannounce("Duping\n");  
  /*
    if(dup2(in,  STDIN_FILENO) < 0)
    goto fail;
    if(REGISTRY_LIB_DEBUG || iop_debug_flag)
    fprintf(stderr, "Closing\n");  
    if(close(in) !=  0)  
    goto fail;
  */
  rannounce("Callocing\n");  
  theRegistry = (actor_id**)calloc(theRegistrySize, sizeof(actor_id*));
  assert(theRegistry != NULL);
  rannounce("registryInit unlocking mutex\n");  
  pthread_mutex_unlock(&theRegistryMutex);
  rannounce("registryInit unlocked mutex\n");  
  if(theRegistry == NULL) goto fail;
  rannounce("Registry successfully initialized\n");  
  return 0;

 fail:

  fprintf(stderr, "Failure in registryInit: %s\n", strerror(errno));
  return -1;
}


int errorsInit(){
  sprintf(errorsFileName, "/tmp/iop_%d_c_errors", iop_pid);
  sprintf(javaErrorsFileName, "/tmp/iop_%d_java_errors", iop_pid);
  rannounce("IOP's error file is: %s\n", errorsFileName);
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
  rannounce("makeActorId commencing for %s\n", acts->name);
  retval = (actor_id *)calloc(1, sizeof(actor_id));
  if(retval == NULL){  
    reason = "calloc failed";
    goto fail; 
  }

  retval->spec = acts;
  retval->exitFlag = 0;
  for(i = 0; i < 3; i++)
    retval->fds[i] = -1;
  
  rannounce("makeActorId finished  %s\n", acts->name);

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
  rannounce("initActorId commencing for %s\n", aspec->name);
  pthread_mutex_init(&(actid->mutex), NULL);
  for(i = 0; i < 3; i++){
    actid->fds[i] = open(aspec->fifos[i], flags[i]);
    if(actid->fds[i] == -1) goto fail;
  }
  rannounce("initActorId launching errorLog thread  for %s\n", aspec->name);
  if((pthread_create(&(actid->tids[0]), NULL, errorLog, actid) != 0)){
    goto fail;
  }
  rannounce("initActorId launching echoOut thread  for %s\n", aspec->name);
  if((pthread_create(&(actid->tids[1]), NULL, echoOut, actid) != 0)){
    goto fail;
  }
  rannounce("initActorId finished  %s\n", aspec->name);

  return 1;

 fail:
  fprintf(stderr, "Failure in initActorId: %s\n", strerror(errno));
  fprintf(stderr, "Actor name = %s\n", aspec->name);
  return -1;

}

static void outputRegistry(int fd){
  int i, count = 0;
  rannounce("outputRegistry locking mutex\n");
  pthread_mutex_lock(&theRegistryMutex);
  rannounce("outputRegistry locked mutex\n");


  for(i = 0; i < theRegistrySize; i++)
    if(theRegistry[i] != NULL){
      if(theRegistry[i]->spec->pid > 0){ count++; }
    }
  
  if(writeInt(fd, count) < 0){
    goto exit;
  }
  rannounce("sent the count %d\n", count);

  for(i = 0; i < theRegistrySize; i++)
    if(theRegistry[i] != NULL){
      if(theRegistry[i]->spec->pid > 0){
	char *name = theRegistry[i]->spec->name;
	int lenN = strlen(name);
	if(writeInt(fd, i) < 0) goto exit;
	rannounce("sent i =  %d\n", i);
	if(write(fd, name, lenN) != lenN)
	  goto exit;
	if(write(fd, &cr, sizeof(char)) != sizeof(char))
	  goto exit;
	rannounce("sent name =  %s\n", name);
      }
    }

 exit:
  
  rannounce("outputRegistry unlocking mutex\n");
  pthread_mutex_unlock(&theRegistryMutex);
  rannounce("outputRegistry unlocked mutex\n");
  return;
}

static void outputRegistrySize(int fd){
  int i, count = 0;
  rannounce("outputRegistrySize locking mutex\n");
  pthread_mutex_lock(&theRegistryMutex);
  rannounce("outputRegistrySize locked mutex\n");  

  for(i = 0; i < theRegistrySize; i++)
    if(theRegistry[i] != NULL){
      if(theRegistry[i]->spec->pid > 0){
	count++;
      }
    }
  
  if(writeInt(fd, count) < 0) goto exit;
  
 exit:
  
  rannounce("outputRegistrySize unlocking mutex\n");
  pthread_mutex_unlock(&theRegistryMutex);
  rannounce("outputRegistrySize unlocked mutex\n");
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
  rannounce("_allocateUniqueName(%d): nameIn  = %s\n", requestNo, oldName);

  for(i = 0; i < theRegistrySize; i++){
    if((theRegistry[i] != NULL) && (strcmp(theRegistry[i]->spec->name, oldName) == 0)){
      found = 1;
      break;
    }
  }
  if(!found){ 
    rannounce("_allocateUniqueName(%d): nameOut  = %s\n", requestNo, oldName);
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
    rannounce("_allocateUniqueName(%d): nameOut  = %s\n", requestNo, newName);
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
  
  rannounce("Inside registerActor\n");

  rannounce("Calling makeActorId\n");
  actid = makeActorId(acts);
  rannounce("Called makeActorId\n");

  if(actid == NULL){
    fprintf(stderr, "registerActor failed,  makeActorId returned NULL\n");
    return retval;
  }

  rannounce("Calling initActorId\n");
  errcode = initActorId(actid);
  rannounce("Called initActorId\n");

  if(errcode == -1){
    fprintf(stderr, "registerActor failed,  initActorId returned -1\n");
    return retval;
  }
  

  rannounce("registerActor locking mutex\n");
  pthread_mutex_lock(&theRegistryMutex);
  rannounce("registerActor locked mutex\n");
  
  rannounce("allocating unique name\n");
  
  if(_allocateUniqueName(acts) < 0){
    fprintf(stderr, "_allocateUniqueName failed (name = %s)!\n", acts->name);
    goto exit;
  }
  rannounce("allocated unique name\n");

  slot = _getEmptyRegistrySlot();

  rannounce("got empty slot\n");
  
  if(slot < 0){ 
    goto exit;    
  } else {
    theRegistry[slot] = actid;
    retval = slot;
    goto exit;
  }

 exit:
  
  rannounce("registerActor unlocking mutex\n");
  pthread_mutex_unlock(&theRegistryMutex);
  rannounce("registerActor unlocked mutex\n");
  return retval;
}



static int unregisterActor(int slot){
  int retval = -1;
  actor_id *act = NULL;
  if((slot < 0) || (slot >= theRegistrySize)) return retval;
  rannounce("unregisterActor locking mutex\n");  
  pthread_mutex_lock(&theRegistryMutex);
  rannounce("unregisterActor locked mutex\n");  
  
  act = theRegistry[slot];
  theRegistry[slot] = NULL;
  retval = slot;
  rannounce("unregisterActor unlocking mutex\n");  
  pthread_mutex_unlock(&theRegistryMutex);
  rannounce("unregisterActor unlocked mutex\n");  
  return retval;
}


static actor_id *getActorBySlot(int slot){
  actor_id *retval = NULL;
  rannounce("getActorBySlot locking mutex\n");  
  pthread_mutex_lock(&theRegistryMutex);
  rannounce("getActorBySlot locked mutex\n");  
  retval = theRegistry[slot];
  rannounce("getActorBySlot unlocking mutex\n");  
  pthread_mutex_unlock(&theRegistryMutex);
  rannounce("getActorBySlot unlocked mutex\n");  
  return retval;
}

static actor_id *getActorByName(char *name){
  actor_id *retval = NULL;
  int i;
  if(name == NULL) return NULL;
  rannounce("getActorByName locking mutex\n");  
  pthread_mutex_lock(&theRegistryMutex);
  rannounce("getActorByName locked mutex\n");  
  for(i = 0; i < theRegistrySize; i++){
    if(theRegistry[i] == NULL) continue;
    if(theRegistry[i]->spec->name == NULL) continue;
    if(!strcmp(theRegistry[i]->spec->name, name)){ 
      retval =  theRegistry[i];
      goto exit;
    }
  }

 exit:
  rannounce("getActorByName unlocking mutex\n");  
  pthread_mutex_unlock(&theRegistryMutex);
  rannounce("getActorByName unlocked mutex\n");  
  return retval;
}

static int getActorsSlotByName(char *name){
  int retval = -1, i;
  if(name == NULL) return retval;
  rannounce("getActorsSlotByName locking mutex\n");  
  pthread_mutex_lock(&theRegistryMutex);
  rannounce("getActorsSlotByName locked mutex\n");  
  for(i = 0; i < theRegistrySize; i++){
    if(theRegistry[i] == NULL) continue;
    if(theRegistry[i]->spec->name == NULL) continue;
    if(!strcmp(theRegistry[i]->spec->name, name)){ 
      retval =  i;
      goto exit;
    }
  }

 exit:
  rannounce("getActorsSlotByName unlocking mutex\n");  
  pthread_mutex_unlock(&theRegistryMutex);
  rannounce("getActorsSlotByName unlocked mutex\n");  

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
      rannounce("errorLog thread of %s exiting gracefully\n", act->spec->name);
      pthread_exit(NULL);
    }
 
    if((errmsg = readMsgVolatile(fd, &act->exitFlag)) == NULL){
      fprintf(stderr, "readMsgVolatile in errorLog returned NULL\n");
      if(++failures > 5){
	fprintf(stderr, "errorLog giving up!\n");
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
    
    log2File("\n%s wrote the following %d bytes:\n", act->spec->name, errmsg->bytesUsed);
    freeMsg(errmsg);
  }

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
  
  rannounce("echoOut for %s commencing\n", act->spec->name);


  while(1){
    if(act->exitFlag){
      rannounce("echoOut thread of %s exiting gracefully\n", act->spec->name);
      pthread_exit(NULL);
    }
    rannounce("echoOut for %s waiting\n", act->spec->name);

    if((outmsg = acceptMsgVolatile(fd, &act->exitFlag)) == NULL){
      rannounce("acceptMsgVolatile in echoOut for %s returned NULL\n", act->spec->name);
      continue;
    }
    rannounce("\nbuffer no of %s: %d\n", act->spec->name, buffno++);
    rannounce("calling parseOut outmsg size = %d\n", outmsg->bytesUsed);
    rannounce("calling parseOut outmsg = %s\n", outmsg->data);
    parseOut(outmsg);
    rannounce("called parseOut\n");
    freeMsg(outmsg);
    rannounce("freed outmsg\n");
  }
}

static void parseOut(msg* outmsg){
  char *copy = NULL;
  char *parsedmsg = NULL;
  char *target, *sender, *rest;
  actor_id *recipient;
  rannounce("commencing parseOut\n");
  if((copy = (char *)calloc(outmsg->bytesUsed + 1, sizeof(char))) == NULL)
    goto fail;
  strcpy(copy, outmsg->data);
  if(getNextToken(copy, &target, &rest) != 1)
    goto echo;
  rannounce("target = \"%s\"\n", target);
  rannounce("calling getActorByName\n");
  recipient = getActorByName(target);
  rannounce("recipient == NULL: %d\n", recipient == NULL);
  if(recipient == NULL)
    goto echo;
  if(recipient->spec->pid <= 0)
    goto echo;
  if(getNextToken(rest, &sender, &rest) != 1)
    goto echo;
  rannounce("sender = \"%s\"\n", sender);
  if(rest == NULL)
    goto echo;
  if((parsedmsg = (char *)calloc((outmsg->bytesUsed + 4), sizeof(char))) == NULL)
    goto fail;

  sprintf(parsedmsg, "(%s %s)", sender, rest);


  rannounce("parsedmsg = \"%s\"\n", parsedmsg);
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
  rannounce("shutdownRegistry locking mutex\n");  
  pthread_mutex_lock(&theRegistryMutex);
  rannounce("shutdownRegistry locked mutex\n");  

  bail();
  rannounce("shutdownRegistry unlocking mutex\n");  
  pthread_mutex_unlock(&theRegistryMutex);
  rannounce("shutdownRegistry unlocked mutex\n");  
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
  rannounce("SEND clause, actorId = %d\n", actorId);
  if(readInt(inFd, &bytesin) < 0)
    return;
  rannounce("SEND clause, bytesin = %d\n", bytesin);
  if((buffin = (char *)calloc(bytesin + 1, sizeof(char))) == NULL){
    fprintf(stderr, "SEND clause, calloc of buffin failed\n");
    return;
  }
  if(read(inFd, buffin, bytesin) != bytesin){
    fprintf(stderr, "SEND clause, read into buffin failed\n");
    free(buffin);
    return;
  }
  rannounce("buffin[bytesin - 1] == NULL : %d\n", buffin[bytesin - 1] == '\0');
  buffin[bytesin] = '\0';
  rannounce("SEND clause, buffin = \"%s\"\n", buffin);
  if((target = getActorBySlot(actorId)) == NULL){
    fprintf(stderr, "SEND clause, getActorBySlot failed\n");
    free(buffin);
    return;
  }
  rannounce("SEND clause, target = %s\n", target->spec->name);
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
  /*
    if(write((target->fds)[IN], &cr, sizeof(char)) != sizeof(char)){
    fprintf(stderr, "SEND clause, write of cr failed\n");
    return;
    }
  */

  rannounce("SEND clause, send OK\n");
  if(REGISTRY_LIB_DEBUG || iop_debug_flag){
    write(STDOUT_FILENO, buffin, bytesin);
    write(STDOUT_FILENO, &cr, sizeof(char));
  }
  free(buffin);
  return;
}

void processRegisterCommand(int inFd, int outFd, int notifyGUI){
  int slotNumber;
  actor_spec *acts = NULL;
  rannounce("Reading actor spec\n");
  if((acts =  readActorSpec(inFd)) == NULL){
    fprintf(stderr, "Reading actor spec failed\n");
    return; 
  }
  rannounce("Registering actor spec\n");
  if((slotNumber = registerActor(acts)) == -1)
    return;
  
  rannounce("Replying with slot %d\n", slotNumber);

  if(writeInt(outFd, slotNumber) < 0){
    fprintf(stderr, "REGISTER clause, write of slotNumber failed\n");
    return;
  }
  rannounce("sent slotNumber = %d\n", slotNumber);
  
  if(notifyGUI){
    /* 
       the system is the first actor now, lots of race conditions here!  
       simplest way to fix this is have the system actor get registered 
       first, and not try and notify the no-existent GUI.
    */
    if(!iop_no_windows_flag){
      rannounce("notifying GUI\n");
      sendMsg2Input(NULL, UPDATE);
      rannounce("notified GUI\n");
    }
  }


  rannounce("breaking\n");
  return;
}

void processUnregisterCommand(int inFd, int outFd){
  char name[PATH_MAX + 1];
  int len, slotNumber;
  actor_id* victim;
  rannounce("UNREGISTER clause, Reading actor name\n");
  if((len = read(inFd, name, PATH_MAX)) <= 0)
    return; 
  name[len] ='\0';
  rannounce("UNREGISTER clause, Registering actor name: %s\n", name);
  if((slotNumber = getActorsSlotByName(name)) == -1)
    return;
  if((victim = getActorBySlot(slotNumber)) == NULL){
    fprintf(stderr, "UNREGISTER clause, getActorBySlot failed\n");
    return;
  }
  if(unregisterActor(slotNumber) == -1)
    return;
  rannounce("UNREGISTER clause, found actor %s in slotNumber = %d\n", 
	    name, slotNumber);
  if(writeInt(outFd, slotNumber) < 0){
    fprintf(stderr, "UNREGISTER clause, write of slotNumber failed\n");
    return;
  }
  rannounce("UNREGISTER clause, sent slotNumber = %d\n", slotNumber);
  /* give victim time to exit by relinquishing CPU */
  usleep(1);
  if(victim != NULL){
    deleteActor(victim);
    freeActor(victim);
  }

  if(!iop_no_windows_flag){
    rannounce("notifying GUI\n");
    sendMsg2Input(NULL, UPDATE);
    rannounce("notified GUI\n");
  }
  rannounce("UNREGISTER clause, breaking\n");
  return;

}

void processNameCommand(int cmd, int inFd, int outFd){
  int actorId, len;
  actor_id* target;
  char unk[] = UNKNOWNNAME;
  if(readInt(inFd, &actorId) < 0) goto fail;
  if((target = getActorBySlot(actorId)) == NULL){
    rannounce("NAME clause, getActorBySlot failed\n");
    goto fail;
  }
  write(outFd, target->spec->name, strlen(target->spec->name));
  return;

 fail:
  len = strlen(unk);
  if(write(outFd, unk, len) != len){
    fprintf(stderr, "write(outFd, unk, strlen(unk)) failed\n");
  };
  rannounce("command index = %d\n", cmd);
  return;
}

void processRegistryCommand(int inFd, int outFd, int notifyGUI){
  int cmd = -1;
  rannounce("Awaiting a command \n");
  if(readInt(inFd, &cmd) < 0){
    fprintf(stderr, "Read of cmd failed\n");
    return;
  }
  rannounce("Processing %s command (cmd = %d)\n", cmd2str(cmd), cmd);
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

  rannounce("Waiting notification of reg2InPort\n");
  if((reg2InPort = wait4ReadyFromInputWindow(listeningSocket)) < 0){
    fprintf(stderr, "wait4ReadyFromInputWindow failed\n");
    bail();
  }
  rannounce("reg2InPort = %d\n", reg2InPort);


  while(1){
    int  *msgsock;
    char *description = NULL;
    rannounce("monitorInSocket blocking on acceptSocket\n");
    msgsock = acceptSocket(listeningSocket, &description);
    if (*msgsock == INVALID_SOCKET){
      fprintf(stderr, description);
      socketCleanUp();
      free(description); 
      pthread_exit(NULL);
}
    rannounce("%s -- *msgsock = %d\n", description, *msgsock);
    processRegistryCommand(*msgsock, *msgsock, 1);
    free(description); 
    closeSocket(*msgsock);
  }
}

static int wait4ReadyFromInputWindow(int in2regSocket){
  int  *msgsock, bytesread;
  char *description = NULL, buff[SIZE];
  rannounce("wait4ReadyFromInputWindow blocking on acceptSocket\n");
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


static int sendMsg2Input(msg *message, output_cmd_t type){
  int socket, retval = 0;


  rannounce("sendMsg2Input:(\ttype = %d\tmsg = %s%s)\n",
	    type, 
	    (message != NULL) ? "\n" : "",
	    (message != NULL) ? message->data : "NULL");

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
  rannounce("sendMsg2Input: closing socket\n");
  closeSocket(socket);
  rannounce("sendMsg2Input: closed socket\n");
  return retval;
}


char* registryLaunchActor(char* name, int argc, char** argv){
  int slot, i, errcode;
  char* retval = NULL;
  char* executable;
  actor_id* actid;
  actor_spec *spec;
  rannounce("registryLaunchActor calling makeActorSpec for %s\n", name);  
  spec = makeActorSpec(name);
  rannounce("registryLaunchActor called makeActorSpec for %s\n", name);  
  rannounce("registryLaunchActor locking mutex\n");  
  pthread_mutex_lock(&theRegistryMutex);
  rannounce("registryLaunchActor locked mutex\n");  

  rannounce("registryLaunchActor calling _allocateUniqueName\n");  


  if(_allocateUniqueName(spec) < 0){
    fprintf(stderr, "_allocateUniqueName failed (name = %s)!\n", name);
    goto exit;
  }

  rannounce("registryLaunchActor calling _getEmptyRegistrySlot\n");  


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
    } 
  }

  rannounce("registryLaunchActor calling newActor\n");  
  
  spec = newActor(0, executable,  argv);
  argv[0] = executable;
  if(spec == NULL){
    fprintf(stderr, "registryLaunchActor: newActor failed (name = %s)!\n", name);
    goto exit;
  }

  /*
    fprintf(stderr, "%s had pid_t %d\n", spec->name, spec->pid);
  */

  rannounce("registryLaunchActor calling makeActorId\n");  
  actid = makeActorId(spec);
  rannounce("registryLaunchActor called makeActorId\n");  

  if(actid == NULL){
    fprintf(stderr, "registryLaunchActor: makeActorId failed (name = %s)!\n", name);
    goto exit;
  }

  rannounce("Calling initActorId\n");
  errcode = initActorId(actid);
  rannounce("Called initActorId\n");

  if(errcode == -1){
    fprintf(stderr, "registryLaunchActor: initActorId returned -1\n");
    goto exit;
  }

  theRegistry[slot] = actid;
  retval = spec->name;

 exit:

  rannounce("registryLaunchActor unlocking mutex\n");  
  pthread_mutex_unlock(&theRegistryMutex);
  rannounce("registryLaunchActor unlocked mutex\n");  
  rannounce("registryLaunchActor complete!\n");  
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
    argc = makeArgv(args, " \t", &argv);
    /*
      printArgv(stderr, argc, argv);
    */
    if(argc > 0){
      actorName = registryLaunchActor(name, argc, argv);
      if(actorName != NULL){
	if(sender != NULL){
	  rannounce("%s\n%s\nstartOK %s\n", sender, REGISTRY_ACTOR, actorName);
	  sendFormattedMsgFP(stdout, "%s\n%s\nstartOK %s\n", sender, REGISTRY_ACTOR, actorName);
	}
      } else {
	if(sender != NULL){
	  rannounce("%s\n%s\nstartFAILED %s\n", sender, REGISTRY_ACTOR, name);
	  sendFormattedMsgFP(stdout, "%s\n%s\nstartFAILED %s\n", sender, REGISTRY_ACTOR, name);
	}
      }
      freeArgv(argc, argv);

      if(notify && !iop_no_windows_flag){
	rannounce("processRegistryStartMessage notifying GUI\n");  
	sendMsg2Input(NULL, UPDATE);
	rannounce("processRegistryStartMessage notified GUI\n");  
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
	  rannounce("processRegistryStopMessage locking mutex\n");  
	  pthread_mutex_lock(&theRegistryMutex);
	  rannounce("processRegistryStopMessage locked mutex\n");  
	  theRegistry[slot] = NULL;
	  rannounce("processRegistryStopMessage unlocking mutex\n");  
	  pthread_mutex_unlock(&theRegistryMutex);
	  rannounce("processRegistryStopMessage unlocked mutex\n");  
	}
	shutdownActor(actid);  
	if(!iop_no_windows_flag){
	  sendMsg2Input(NULL, UPDATE);
	}
	if(REGISTRY_DEBUG || iop_debug_flag)
	  fprintf(stderr, "%s\n%s\nstopOK %s\n", sender, REGISTRY_ACTOR, name);
	sendFormattedMsgFP(stdout, "%s\n%s\nstopOK %s\n", sender, REGISTRY_ACTOR, name);
      } else {
	fprintf(stderr, "processRegistryStopMessage: actid of %s is NULL\n", name);
	if(REGISTRY_DEBUG || iop_debug_flag)
	  fprintf(stderr,  "%s\n%s\nstopFAILED %s\n", sender, REGISTRY_ACTOR, name);
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
	rannounce("processRegistryNameMessage: calling makeActorId\n");
	actid = makeActorId(aspec);
	rannounce("processRegistryNameMessage: called makeActorId\n");
	
	if(actid == NULL){
	  fprintf(stderr, "processRegistryNameMessage:  makeActorId returned NULL\n");
	  goto exit;
	}
	
        rannounce("processRegistryNameMessage: locking mutex\n");
	pthread_mutex_lock(&theRegistryMutex);
	rannounce("processRegistryNameMessage: locked mutex\n");
	
	rannounce("processRegistryNameMessage: allocating unique name\n");
	
	if(_allocateUniqueName(aspec) < 0){
	  fprintf(stderr, "_allocateUniqueName failed (name = %s)!\n", aspec->name);
	  goto unlock;
	}
	rannounce("allocated unique name\n");
	
	slot = _getEmptyRegistrySlot();
	
	rannounce("got empty slot\n");
	
	if(slot >= 0){ 
	  theRegistry[slot] = actid;
	}
	
	
      unlock:
	
	rannounce("processRegistryNameMessage: unlocking mutex\n");
	pthread_mutex_unlock(&theRegistryMutex);
	rannounce("processRegistryNameMessage: unlocked mutex\n");
	
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
	rannounce("processRegistryUnenrollMessage: found actor %s in slotNumber = %d\n", 
		  name, slot);
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
      rannounce("Configuration file not opened: %s", strerror(errno));
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
	  rannounce("registryProcessConfigFile: selected actor = %s\n", name);
	  strncpy(selectedActor, name, SIZE);
	  selected = 1;
	}
      }
      
      if(!iop_no_windows_flag){
	rannounce("registryProcessConfigFile notifying GUI\n");  
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
	rannounce("registryProcessConfigFile notified GUI\n");  
      }
      return counter;
    }
  }
}
