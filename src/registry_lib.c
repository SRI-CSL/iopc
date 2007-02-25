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
#include "ec.h"

static char javaErrorsFileName[PATH_MAX];
static char *errorsFileName = NULL;
static FILE *errorsFile = NULL;
static char cr = '\n';
static char selectedActor[SIZE];
static int  selected = 0;

static pthread_mutex_t theRegistryMutex = PTHREAD_MUTEX_INITIALIZER;
static actor_id  **theRegistry;
static int theRegistrySize = REGISTRYSZ;
static pthread_mutex_t iop_errlog_mutex = PTHREAD_MUTEX_INITIALIZER;

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

/* externs used in the log2File routine */
extern int   self_debug_flag;
extern char* self;

void log2File(const char *format, ...){
  va_list arg;
  va_start(arg, format);
  if(self_debug_flag && (format != NULL) && (errorsFileName != NULL)){
    ec_rv( pthread_mutex_lock(&iop_errlog_mutex) );
    ec_null( errorsFile = fopen(errorsFileName, "a") );
    fprintf(errorsFile, "%s(%ld)\t:\t", self, (long)pthread_self());
    vfprintf(errorsFile, format, arg);
    fflush(errorsFile);
    fclose(errorsFile);
    ec_rv( pthread_mutex_unlock(&iop_errlog_mutex) );
  }
  va_end(arg);
  return;
EC_CLEANUP_BGN
  va_end(arg);
  return;
EC_CLEANUP_END
}

static void registry_sig_handler(int sig){
  log2File("Got signal %d\n", sig);
  if((sig == SIGUSR1) || (sig == SIGSEGV)) {
    bail();
    exit(EXIT_FAILURE);
  } 
}

static void registry_sigchld_handler(int sig){
  int status;
  pid_t child = waitpid(-1, &status, WNOHANG);
  log2File("waited on child with pid %d with exit status %d\n",  child, status);
}

int registry_installHandler(){
  struct sigaction sigactInt;
  struct sigaction sigactSegv;
  struct sigaction sigactChld;
  sigset_t sigmask;
  ec_neg1( sigemptyset(&sigmask) );
  ec_neg1( sigaddset(&sigmask, SIGINT) );
  ec_neg1( sigprocmask(SIG_BLOCK, &sigmask, NULL) );
  sigactInt.sa_handler = registry_sig_handler;
  sigactInt.sa_flags = 0;
  ec_neg1( sigfillset(&sigactInt.sa_mask) );
  ec_neg1( sigaddset(&sigactInt.sa_mask, SIGINT) );
  ec_neg1( sigaction(SIGUSR1, &sigactInt, NULL) );
  sigactSegv.sa_handler = registry_sig_handler;
  sigactSegv.sa_flags = 0;
  ec_neg1( sigfillset(&sigactSegv.sa_mask) );
  ec_neg1( sigaction(SIGSEGV, &sigactSegv, NULL) );
  sigactChld.sa_handler = registry_sigchld_handler;
  sigactChld.sa_flags = 0;
  ec_neg1( sigfillset(&sigactChld.sa_mask) );
  ec_neg1( sigaction(SIGCHLD, &sigactChld, NULL) );
  return 0; 
EC_CLEANUP_BGN
  return -1;
EC_CLEANUP_END
}


int makeRegistryFifos(){
  log2File("Unlinking %s\n", registry_fifo_in);  
  /* try and clean up old copies, ignore failure */
  (void)unlink(registry_fifo_in);
  log2File("Creating %s\n", registry_fifo_in);  
  /* make new ones               */
  ec_neg1( mkfifo(registry_fifo_in,  S_IRWXU) );
  log2File("Unlinking %s\n", registry_fifo_out);  
  /* try and clean up old copies, ignore failure */
  (void)unlink(registry_fifo_out);
  log2File("Creating %s\n", registry_fifo_out);  
  /* make new ones               */
  ec_neg1( mkfifo(registry_fifo_out, S_IRWXU) );    
  return 0;
EC_CLEANUP_BGN
  return -1;
EC_CLEANUP_END
}

int registryInit(int *fifo_in_fd, int *fifo_out_fd){
  log2File("registryInit locking mutex\n");
  ec_rv( pthread_mutex_lock(&theRegistryMutex) );
  log2File("registryInit locked mutex\n");  
  log2File("Opening %s\n", registry_fifo_in);  
  ec_neg1( *fifo_in_fd = open(registry_fifo_in, O_RDWR) );
  log2File("Opening %s\n", registry_fifo_out);  
  ec_neg1( *fifo_out_fd = open(registry_fifo_out, O_RDWR) );
  log2File("Callocing\n");  
  ec_null( theRegistry = calloc(theRegistrySize, sizeof(actor_id*)) );
  log2File("registryInit unlocking mutex\n");  
  ec_rv( pthread_mutex_unlock(&theRegistryMutex) );
  log2File("registryInit unlocked mutex\n");  
  log2File("Registry successfully initialized\n");  
  return 0;
EC_CLEANUP_BGN
  return -1;
EC_CLEANUP_END
}


int errorsInit(){
  ec_null( errorsFileName = calloc(PATH_MAX, sizeof(char)) );
  snprintf(errorsFileName, PATH_MAX, "/tmp/iop_%d_c_errors", iop_pid);
  snprintf(javaErrorsFileName, PATH_MAX, "/tmp/iop_%d_java_errors", iop_pid);
  ec_null( errorsFile = fopen(errorsFileName, "w") );
  ec_cmp( fclose(errorsFile), EOF );
  return 0;
EC_CLEANUP_BGN
  return -1;
EC_CLEANUP_END
}


static actor_id *makeActorId(actor_spec *acts){
  int i;
  actor_id *retval = NULL;
  if(acts != NULL){  
    log2File("makeActorId commencing for %s\n", acts->name);
    ec_null( retval = calloc(1, sizeof(actor_id)) );
    retval->spec = acts;
    retval->exitFlag = 0;
    for(i = 0; i < 3; i++){ retval->fds[i] = -1; }
    log2File("makeActorId finished  %s\n", acts->name);
  } else {
    fprintf(stderr, "Failure in makeActorId: acts == NULL\n");
  }
  return retval;
EC_CLEANUP_BGN
  return NULL;
EC_CLEANUP_END

}

static int initActorId(actor_id *actid){
  int i, flags[3] = { O_RDWR, O_RDWR,  O_RDWR };
  actor_spec *aspec = NULL;
  if(actid != NULL){  
    aspec = actid->spec;
    log2File("initActorId commencing for %s\n", aspec->name);
    ec_rv( pthread_mutex_init(&(actid->mutex), NULL) );
    for(i = 0; i < 3; i++){
      ec_neg1( actid->fds[i] = open(aspec->fifos[i], flags[i]) );
    }
    log2File("initActorId launching errorLog thread  for %s\n", aspec->name);
    ec_rv( pthread_create(&(actid->tids[0]), NULL, errorLog, actid) );
    log2File("initActorId launching echoOut thread  for %s\n", aspec->name);
    ec_rv( pthread_create(&(actid->tids[1]), NULL, echoOut, actid) );
    log2File("initActorId finished  %s\n", aspec->name);
    return 0;
  } else {
    fprintf(stderr, "Failure in initActorId: actid == NULL\n");
    return -1;
  }
EC_CLEANUP_BGN
  return -1;
EC_CLEANUP_END
}

static void outputRegistry(int fd){
  int i, count = 0;
  log2File("outputRegistry locking mutex\n");
  pthread_mutex_lock(&theRegistryMutex);
  log2File("outputRegistry locked mutex\n");


  for(i = 0; i < theRegistrySize; i++)
    if(theRegistry[i] != NULL){
      if(theRegistry[i]->spec->pid > 0){ count++; }
    }
  
  if(writeInt(fd, count) < 0){
    goto exit;
  }
  log2File("sent the count %d\n", count);

  for(i = 0; i < theRegistrySize; i++)
    if(theRegistry[i] != NULL){
      if(theRegistry[i]->spec->pid > 0){
	char *name = theRegistry[i]->spec->name;
	int lenN = strlen(name);
	if(writeInt(fd, i) < 0) goto exit;
	log2File("sent i =  %d\n", i);
	if(write(fd, name, lenN) != lenN)
	  goto exit;
	if(write(fd, &cr, sizeof(char)) != sizeof(char))
	  goto exit;
	log2File("sent name =  %s\n", name);
      }
    }

 exit:
  
  log2File("outputRegistry unlocking mutex\n");
  pthread_mutex_unlock(&theRegistryMutex);
  log2File("outputRegistry unlocked mutex\n");
  return;
}

static void outputRegistrySize(int fd){
  int i, count = 0;
  log2File("outputRegistrySize locking mutex\n");
  pthread_mutex_lock(&theRegistryMutex);
  log2File("outputRegistrySize locked mutex\n");  

  for(i = 0; i < theRegistrySize; i++)
    if(theRegistry[i] != NULL){
      if(theRegistry[i]->spec->pid > 0){
	count++;
      }
    }
  
  if(writeInt(fd, count) < 0) goto exit;
  
 exit:
  
  log2File("outputRegistrySize unlocking mutex\n");
  pthread_mutex_unlock(&theRegistryMutex);
  log2File("outputRegistrySize unlocked mutex\n");
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
  log2File("_allocateUniqueName(%d): nameIn  = %s\n", requestNo, oldName);

  for(i = 0; i < theRegistrySize; i++){
    if((theRegistry[i] != NULL) && (strcmp(theRegistry[i]->spec->name, oldName) == 0)){
      found = 1;
      break;
    }
  }
  if(!found){ 
    log2File("_allocateUniqueName(%d): nameOut  = %s\n", requestNo, oldName);
    return 1;
  } else {
    int len = strlen(oldName) + 1;
    char *newName = (char *)calloc(len + SIZE, sizeof(char));
    int index = -1;
    if(newName == NULL){ return -1; }
    if(len + SIZE > PATH_MAX){ return -1; }
    while(found == 1){
      index++;
      snprintf(newName, len + SIZE, "%s%d", oldName, index);
      found = 0;
      for(i = 0; i < theRegistrySize; i++){
	if((theRegistry[i] != NULL) && (strcmp(theRegistry[i]->spec->name, newName) == 0)){
	  found = 1;
	  break;
	}
      }
    }/* while(found == 1) */
    strncpy(acts->name, newName, PATH_MAX);
    log2File("_allocateUniqueName(%d): nameOut  = %s\n", requestNo, newName);
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
  
  log2File("Inside registerActor\n");

  log2File("Calling makeActorId\n");
  actid = makeActorId(acts);
  log2File("Called makeActorId\n");

  if(actid == NULL){
    fprintf(stderr, "registerActor failed,  makeActorId returned NULL\n");
    return retval;
  }

  log2File("Calling initActorId\n");
  errcode = initActorId(actid);
  log2File("Called initActorId\n");

  if(errcode == -1){
    fprintf(stderr, "registerActor failed,  initActorId returned -1\n");
    return retval;
  }
  

  log2File("registerActor locking mutex\n");
  pthread_mutex_lock(&theRegistryMutex);
  log2File("registerActor locked mutex\n");
  
  log2File("allocating unique name\n");
  
  if(_allocateUniqueName(acts) < 0){
    fprintf(stderr, "_allocateUniqueName failed (name = %s)!\n", acts->name);
    goto exit;
  }
  log2File("allocated unique name\n");

  slot = _getEmptyRegistrySlot();

  log2File("got empty slot\n");
  
  if(slot < 0){ 
    goto exit;    
  } else {
    theRegistry[slot] = actid;
    retval = slot;
    goto exit;
  }

 exit:
  
  log2File("registerActor unlocking mutex\n");
  pthread_mutex_unlock(&theRegistryMutex);
  log2File("registerActor unlocked mutex\n");
  return retval;
}



static int unregisterActor(int slot){
  int retval = -1;
  actor_id *act = NULL;
  if((slot < 0) || (slot >= theRegistrySize)) return retval;
  log2File("unregisterActor locking mutex\n");  
  pthread_mutex_lock(&theRegistryMutex);
  log2File("unregisterActor locked mutex\n");  
  
  act = theRegistry[slot];
  theRegistry[slot] = NULL;
  retval = slot;
  log2File("unregisterActor unlocking mutex\n");  
  pthread_mutex_unlock(&theRegistryMutex);
  log2File("unregisterActor unlocked mutex\n");  
  return retval;
}


static actor_id *getActorBySlot(int slot){
  actor_id *retval = NULL;
  log2File("getActorBySlot locking mutex\n");  
  pthread_mutex_lock(&theRegistryMutex);
  log2File("getActorBySlot locked mutex\n");  
  retval = theRegistry[slot];
  log2File("getActorBySlot unlocking mutex\n");  
  pthread_mutex_unlock(&theRegistryMutex);
  log2File("getActorBySlot unlocked mutex\n");  
  return retval;
}

static actor_id *getActorByName(char *name){
  actor_id *retval = NULL;
  int i;
  if(name == NULL) return NULL;
  log2File("getActorByName locking mutex\n");  
  pthread_mutex_lock(&theRegistryMutex);
  log2File("getActorByName locked mutex\n");  
  for(i = 0; i < theRegistrySize; i++){
    if(theRegistry[i] == NULL) continue;
    if(theRegistry[i]->spec->name == NULL) continue;
    if(!strcmp(theRegistry[i]->spec->name, name)){ 
      retval =  theRegistry[i];
      goto exit;
    }
  }

 exit:
  log2File("getActorByName unlocking mutex\n");  
  pthread_mutex_unlock(&theRegistryMutex);
  log2File("getActorByName unlocked mutex\n");  
  return retval;
}

static int getActorsSlotByName(char *name){
  int retval = -1, i;
  if(name == NULL) return retval;
  log2File("getActorsSlotByName locking mutex\n");  
  pthread_mutex_lock(&theRegistryMutex);
  log2File("getActorsSlotByName locked mutex\n");  
  for(i = 0; i < theRegistrySize; i++){
    if(theRegistry[i] == NULL) continue;
    if(theRegistry[i]->spec->name == NULL) continue;
    if(!strcmp(theRegistry[i]->spec->name, name)){ 
      retval =  i;
      goto exit;
    }
  }

 exit:
  log2File("getActorsSlotByName unlocking mutex\n");  
  pthread_mutex_unlock(&theRegistryMutex);
  log2File("getActorsSlotByName unlocked mutex\n");  

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
      log2File("errorLog thread of %s exiting gracefully\n", act->spec->name);
      pthread_exit(NULL);
    }
 
    if((errmsg = readMsgVolatile(fd, &act->exitFlag)) == NULL){
      log2File("readMsgVolatile in errorLog returned NULL\n");
      if(++failures > 5){
	log2File("errorLog giving up!\n");
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
    log2File("\n%s those  %d bytes were:%s\n", act->spec->name, errmsg->bytesUsed, errmsg->data);
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
  
  log2File("echoOut for %s commencing\n", act->spec->name);


  while(1){
    if(act->exitFlag){
      log2File("echoOut thread of %s exiting gracefully\n", act->spec->name);
      pthread_exit(NULL);
    }
    log2File("echoOut for %s waiting\n", act->spec->name);

    if((outmsg = acceptMsgVolatile(fd, &act->exitFlag)) == NULL){
      log2File("acceptMsgVolatile in echoOut for %s returned NULL\n", act->spec->name);
      continue;
    }
    log2File("\nbuffer no of %s: %d\n", act->spec->name, buffno++);
    log2File("calling parseOut outmsg size = %d\n", outmsg->bytesUsed);
    log2File("calling parseOut outmsg = %s\n", outmsg->data);
    parseOut(outmsg);
    log2File("called parseOut\n");
    freeMsg(outmsg);
    log2File("freed outmsg\n");
  }
}

static void parseOut(msg* outmsg){
  char *copy = NULL;
  char *parsedmsg = NULL;
  char *target, *sender, *rest;
  actor_id *recipient;
  log2File("commencing parseOut\n");
  if((copy = (char *)calloc(outmsg->bytesUsed + 1, sizeof(char))) == NULL)
    goto fail;
  strncpy(copy, outmsg->data, outmsg->bytesUsed + 1);
  if(getNextToken(copy, &target, &rest) != 1)
    goto echo;
  log2File("target = \"%s\"\n", target);
  log2File("calling getActorByName\n");
  recipient = getActorByName(target);
  log2File("recipient == NULL: %d\n", recipient == NULL);
  if(recipient == NULL)
    goto echo;
  if(recipient->spec->pid <= 0)
    goto echo;
  if(getNextToken(rest, &sender, &rest) != 1)
    goto echo;
  log2File("sender = \"%s\"\n", sender);
  if(rest == NULL)
    goto echo;
  if((parsedmsg = (char *)calloc((outmsg->bytesUsed + 4), sizeof(char))) == NULL)
    goto fail;

  snprintf(parsedmsg, outmsg->bytesUsed + 4, "(%s %s)", sender, rest);


  log2File("parsedmsg = \"%s\"\n", parsedmsg);
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
#ifndef _MAC
      if(act->fds[i] > 0){ close(act->fds[i]); }
#endif
    }
    for(i = 0; i < 3; i++){
      if(unlink(act->spec->fifos[i]) < 0){
	perror("Unlink in shutdownActor failed");
      }
    }
    if(strcmp(act->spec->name, REGISTRY_ACTOR))    
      kill(act->spec->pid, SIGINT);
  }
}

void deleteActor(actor_id *act){
  int i;
  act->exitFlag = 1;
  if(act->spec->pid > 0){
    for(i = 0; i < 3; i++){
#ifndef _MAC
      if(act->fds[i] > 0){ close(act->fds[i]); }
#endif
      if(unlink(act->spec->fifos[i]) < 0){
	fprintf(stderr, 
		"Unlink of %s in deleteActor failed: %s\n", 
		act->spec->fifos[i], strerror(errno));
      }
    }
  }
}

static void shutdownRegistry(){
  pthread_mutex_lock(&theRegistryMutex);
  bail();
  pthread_mutex_unlock(&theRegistryMutex);
  exit(EXIT_SUCCESS);
}

void bail(){
  int i;
  for(i = 0; i < theRegistrySize; i++){
    if(theRegistry[i] != NULL){
      shutdownActor(theRegistry[i]);
    }
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
  log2File("SEND clause, actorId = %d\n", actorId);
  if(readInt(inFd, &bytesin) < 0)
    return;
  log2File("SEND clause, bytesin = %d\n", bytesin);
  if((buffin = (char *)calloc(bytesin + 1, sizeof(char))) == NULL){
    fprintf(stderr, "SEND clause, calloc of buffin failed\n");
    return;
  }
  if(read(inFd, buffin, bytesin) != bytesin){
    fprintf(stderr, "SEND clause, read into buffin failed\n");
    free(buffin);
    return;
  }
  log2File("buffin[bytesin - 1] == NULL : %d\n", buffin[bytesin - 1] == '\0');
  buffin[bytesin] = '\0';
  log2File("SEND clause, buffin = \"%s\"\n", buffin);
  if((target = getActorBySlot(actorId)) == NULL){
    fprintf(stderr, "SEND clause, getActorBySlot failed\n");
    free(buffin);
    return;
  }
  log2File("SEND clause, target = %s\n", target->spec->name);
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
  log2File("SEND clause, send OK\n");
  free(buffin);
  return;
}

void processRegisterCommand(int inFd, int outFd, int notifyGUI){
  int slotNumber;
  actor_spec *acts = NULL;
  log2File("Reading actor spec\n");
  if((acts =  readActorSpec(inFd)) == NULL){
    fprintf(stderr, "Reading actor spec failed\n");
    return; 
  }
  log2File("Registering actor spec\n");
  if((slotNumber = registerActor(acts)) == -1)
    return;
  
  log2File("Replying with slot %d\n", slotNumber);

  if(writeInt(outFd, slotNumber) < 0){
    fprintf(stderr, "REGISTER clause, write of slotNumber failed\n");
    return;
  }
  log2File("sent slotNumber = %d\n", slotNumber);
  
  if(notifyGUI){
    /* 
       the system is the first actor now, lots of race conditions here!  
       simplest way to fix this is have the system actor get registered 
       first, and not try and notify the no-existent GUI.
    */
    if(!iop_no_windows_flag){
      log2File("notifying GUI\n");
      sendMsg2Input(NULL, UPDATE);
      log2File("notified GUI\n");
    }
  }


  log2File("breaking\n");
  return;
}

void processUnregisterCommand(int inFd, int outFd){
  char name[PATH_MAX + 1];
  int len, slotNumber;
  actor_id* victim;
  log2File("UNREGISTER clause, Reading actor name\n");
  if((len = read(inFd, name, PATH_MAX)) <= 0)
    return; 
  name[len] ='\0';
  log2File("UNREGISTER clause, Registering actor name: %s\n", name);
  if((slotNumber = getActorsSlotByName(name)) == -1)
    return;
  if((victim = getActorBySlot(slotNumber)) == NULL){
    fprintf(stderr, "UNREGISTER clause, getActorBySlot failed\n");
    return;
  }
  if(unregisterActor(slotNumber) == -1)
    return;
  log2File("UNREGISTER clause, found actor %s in slotNumber = %d\n", name, slotNumber);
  if(writeInt(outFd, slotNumber) < 0){
    fprintf(stderr, "UNREGISTER clause, write of slotNumber failed\n");
    return;
  }
  log2File("UNREGISTER clause, sent slotNumber = %d\n", slotNumber);
  /* give victim time to exit by relinquishing CPU */
  usleep(1);
  if(victim != NULL){
    deleteActor(victim);
    freeActor(victim);
  }

  if(!iop_no_windows_flag){
    log2File("notifying GUI\n");
    sendMsg2Input(NULL, UPDATE);
    log2File("notified GUI\n");
  }
  log2File("UNREGISTER clause, breaking\n");
  return;

}

void processNameCommand(int cmd, int inFd, int outFd){
  int actorId, len;
  actor_id* target;
  char unk[] = UNKNOWNNAME;
  if(readInt(inFd, &actorId) < 0) goto fail;
  if((target = getActorBySlot(actorId)) == NULL){
    log2File("NAME clause, getActorBySlot failed\n");
    goto fail;
  }
  write(outFd, target->spec->name, strlen(target->spec->name));
  return;

 fail:
  len = strlen(unk);
  if(write(outFd, unk, len) != len){
    fprintf(stderr, "write(outFd, unk, strlen(unk)) failed\n");
  };
  log2File("command index = %d\n", cmd);
  return;
}

void processRegistryCommand(int inFd, int outFd, int notifyGUI){
  int cmd = -1;
  log2File("Awaiting a command \n");
  if(readInt(inFd, &cmd) < 0){
    fprintf(stderr, "Read of cmd failed\n");
    return;
  }
  log2File("Processing %s command (cmd = %d)\n", cmd2str(cmd), cmd);
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

  log2File("Waiting notification of reg2InPort\n");
  if((reg2InPort = wait4ReadyFromInputWindow(listeningSocket)) < 0){
    fprintf(stderr, "wait4ReadyFromInputWindow failed\n");
    bail();
    exit(EXIT_FAILURE);
  }
  log2File("reg2InPort = %d\n", reg2InPort);


  while(1){
    int  *msgsock;
    char *description = NULL;
    log2File("monitorInSocket blocking on acceptSocket\n");
    msgsock = acceptSocket(listeningSocket, &description);
    if (*msgsock == INVALID_SOCKET){
      fprintf(stderr, description);
      free(description); 
      pthread_exit(NULL);
}
    log2File("%s -- *msgsock = %d\n", description, *msgsock);
    processRegistryCommand(*msgsock, *msgsock, 1);
    free(description); 
    close(*msgsock);
  }
}

static int wait4ReadyFromInputWindow(int in2regSocket){
  int  *msgsock, bytesread;
  char *description = NULL, buff[SIZE];
  log2File("wait4ReadyFromInputWindow blocking on acceptSocket\n");
  msgsock = acceptSocket(in2regSocket, &description);
  if (*msgsock == INVALID_SOCKET){
    fprintf(stderr, description);
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
  close(socket);
  return retval;
}


char* registryLaunchActor(char* name, int argc, char** argv){
  int slot, i, errcode;
  char* retval = NULL;
  char* executable;
  actor_id* actid;
  actor_spec *spec;
  log2File("registryLaunchActor calling makeActorSpec for %s\n", name);  
  spec = makeActorSpec(name);
  log2File("registryLaunchActor called makeActorSpec for %s\n", name);  
  log2File("registryLaunchActor locking mutex\n");  
  pthread_mutex_lock(&theRegistryMutex);
  log2File("registryLaunchActor locked mutex\n");  

  log2File("registryLaunchActor calling _allocateUniqueName\n");  


  if(_allocateUniqueName(spec) < 0){
    fprintf(stderr, "_allocateUniqueName failed (name = %s)!\n", name);
    goto exit;
  }

  log2File("registryLaunchActor calling _getEmptyRegistrySlot\n");  


  if((slot = _getEmptyRegistrySlot()) < 0){
    fprintf(stderr, "_getEmptyRegistrySlot() failed (name = %s)!\n", name);
    goto exit;
  }

  /* create the actor  -- careful with names etc */
  executable = argv[0];
  argv[0] = spec->name;

  for(i = 0; i < argc; i++){
    char* newarg;
    int tilde = interpretTildes(argv[i], &newarg);
    if(tilde){
      strncpy(argv[i], newarg, PATH_MAX);
      free(newarg);
      continue;
    }
    if(!strcmp(argv[i], FIFO_IN)){
      strncpy(argv[i], registry_fifo_in, PATH_MAX);
    } else if(!strcmp(argv[i], FIFO_OUT)){
      strncpy(argv[i], registry_fifo_out, PATH_MAX);
    } else if(
	      !strcmp(argv[i], IOP_DENV_VAR)  ||
	      !strcmp(argv[i], IOP_DBENV_VAR) ||
	      !strcmp(argv[i], IOP_DPENV_VAR) ||
	      !strcmp(argv[i], IOP_BIN_DIR)
	      ){
      strncpy(argv[i], iop_bin_dir, PATH_MAX);
    }
  }

  log2File("registryLaunchActor calling newActor\n");  
  
  spec = newActor(0, executable,  argv);
  argv[0] = executable;
  if(spec == NULL){
    fprintf(stderr, "registryLaunchActor: newActor failed (name = %s)!\n", name);
    goto exit;
  }

  /*
    fprintf(stderr, "%s had pid_t %d\n", spec->name, spec->pid);
  */

  log2File("registryLaunchActor calling makeActorId\n");  
  actid = makeActorId(spec);
  log2File("registryLaunchActor called makeActorId\n");  

  if(actid == NULL){
    fprintf(stderr, "registryLaunchActor: makeActorId failed (name = %s)!\n", name);
    goto exit;
  }

  log2File("Calling initActorId\n");
  errcode = initActorId(actid);
  log2File("Called initActorId\n");

  if(errcode == -1){
    fprintf(stderr, "registryLaunchActor: initActorId returned -1\n");
    goto exit;
  }

  theRegistry[slot] = actid;
  retval = spec->name;

 exit:

  log2File("registryLaunchActor unlocking mutex\n");  
  pthread_mutex_unlock(&theRegistryMutex);
  log2File("registryLaunchActor unlocked mutex\n");  
  log2File("registryLaunchActor complete!\n");  
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
    /* printArgv(stderr, argc, argv, sender); */
    
    if(argc > 0){
      actorName = registryLaunchActor(name, argc, argv);
      if(actorName != NULL){
	if(sender != NULL){
	  log2File("%s\n%s\nstartOK %s\n", sender, REGISTRY_ACTOR, actorName);
	  sendFormattedMsgFP(stdout, "%s\n%s\nstartOK %s\n", sender, REGISTRY_ACTOR, actorName);
	}
      } else {
	if(sender != NULL){
	  log2File("%s\n%s\nstartFAILED %s\n", sender, REGISTRY_ACTOR, name);
	  sendFormattedMsgFP(stdout, "%s\n%s\nstartFAILED %s\n", sender, REGISTRY_ACTOR, name);
	}
      }
      freeArgv(argc, argv);

      if(notify && !iop_no_windows_flag){
	log2File("processRegistryStartMessage notifying GUI\n");  
	sendMsg2Input(NULL, UPDATE);
	log2File("processRegistryStartMessage notified GUI\n");  
      }

    }
  }
}

void  processRegistryStopMessage(char *sender, char *rest){
  char *name, *args;
  log2File("Stopping %s\n", sender);
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
	  log2File("processRegistryStopMessage locking mutex\n");  
	  pthread_mutex_lock(&theRegistryMutex);
	  log2File("processRegistryStopMessage locked mutex\n");  
	  theRegistry[slot] = NULL;
	  log2File("processRegistryStopMessage unlocking mutex\n");  
	  pthread_mutex_unlock(&theRegistryMutex);
	  log2File("processRegistryStopMessage unlocked mutex\n");  
	}
	shutdownActor(actid);  
	if(!iop_no_windows_flag){
	  sendMsg2Input(NULL, UPDATE);
	}
	log2File("%s\n%s\nstopOK %s\n", sender, REGISTRY_ACTOR, name);
	sendFormattedMsgFP(stdout, "%s\n%s\nstopOK %s\n", sender, REGISTRY_ACTOR, name);
      } else {
	fprintf(stderr, "processRegistryStopMessage: actid of %s is NULL\n", name);
	log2File("%s\n%s\nstopFAILED %s\n", sender, REGISTRY_ACTOR, name);
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
	strncpy(aspec->name, name, PATH_MAX);
	aspec->pid = 0;
	log2File("processRegistryNameMessage: calling makeActorId\n");
	actid = makeActorId(aspec);
	log2File("processRegistryNameMessage: called makeActorId\n");
	
	if(actid == NULL){
	  fprintf(stderr, "processRegistryNameMessage:  makeActorId returned NULL\n");
	  goto exit;
	}
	
        log2File("processRegistryNameMessage: locking mutex\n");
	pthread_mutex_lock(&theRegistryMutex);
	log2File("processRegistryNameMessage: locked mutex\n");
	
	log2File("processRegistryNameMessage: allocating unique name\n");
	
	if(_allocateUniqueName(aspec) < 0){
	  fprintf(stderr, "_allocateUniqueName failed (name = %s)!\n", aspec->name);
	  goto unlock;
	}
	log2File("allocated unique name\n");
	
	slot = _getEmptyRegistrySlot();
	
	log2File("got empty slot\n");
	
	if(slot >= 0){ 
	  theRegistry[slot] = actid;
	}
	
	
      unlock:
	
	log2File("processRegistryNameMessage: unlocking mutex\n");
	pthread_mutex_unlock(&theRegistryMutex);
	log2File("processRegistryNameMessage: unlocked mutex\n");
	
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
	log2File("processRegistryUnenrollMessage: found actor %s in slotNumber = %d\n", name, slot);
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


static int registryProcessFile(FILE* filep){
  char line[BUFFSZ];
  int counter = 0;
  while(fgets(line, BUFFSZ, filep) != NULL){
    int len;
    counter++;
    if((len = strlen(line)) == 0) continue;
    if(line[0] == '#') continue;
    if(line[len - 1] == '\n'){ line[len - 1] = '\0'; }
    if(strncmp(line, REGISTRY_START, strlen(REGISTRY_START)) == 0){
      /* its probably a start command */
      char *cmd, *rest;
      if(getNextToken(line, &cmd, &rest) != 1){
	fprintf(stderr, "registryProcessFile: parsing %s failed\n", line);
	continue;
      }
      if(!(strcmp(cmd, REGISTRY_START) == 0)){ 
	fprintf(stderr, "registryProcessFile: cmd %s not recognized\n", cmd);
	continue;
      } 
      processRegistryStartMessage(NULL, rest, 0);
    } else if(strncmp(line, REGISTRY_SELECT, strlen(REGISTRY_SELECT)) == 0){
      /* its probably a select command */
      char *cmd, *name, *rest;
      if(getNextToken(line, &cmd, &rest) != 1){
	fprintf(stderr, "registryProcessFile: parsing %s failed\n", line);
	continue;
      }
      if(!(strcmp(cmd, REGISTRY_SELECT) == 0)){ 
	fprintf(stderr, "registryProcessFile: cmd %s not recognized\n", cmd);
	continue;
      } 
      if(getNextToken(rest, &name, &rest) != 1){
	fprintf(stderr, "registryProcessFile: parsing %s failed\n", line);
	continue;
      }
      log2File("registryProcessFile: selected actor = %s\n", name);
      strncpy(selectedActor, name, SIZE);
      selected = 1;
    }
  }
  
  if(!iop_no_windows_flag){
    log2File("registryProcessFile notifying GUI\n");  
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
    log2File("registryProcessFile notified GUI\n");  
  }
  return counter;
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
      log2File("Configuration file not opened: %s\n", strerror(errno));
      return -1;
    } else {
      return registryProcessFile(rcfile);
    }
  }
}

int registryProcessStartupFile(char * sfile){
  FILE* startup = NULL;
  char * startfile = (sfile != NULL) ? sfile : STARTUP;
  startup = fopen(startfile, "r");
  if(startup == NULL){
    log2File("Startup file (%s) not opened: %s\n", startfile, strerror(errno));
    return -1;
  } else {
    return registryProcessFile(startup);
  }
}

