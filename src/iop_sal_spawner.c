#include "cheaders.h"
#include "constants.h"
#include "types.h"
#include "actor.h"
#include "msg.h"
#include "iop_lib.h"
#include "socket_lib.h"
#include "externs.h"
#include "dbugflags.h"

static int    requestNo = 0;
static char*  myName;
static int    clientNo = 0;
static char*  clientChildExe = "iop_sal_actor";
static char*  clientChildArgv[4];
static char   clientChildName[] = "SALActor";

static pthread_mutex_t iop_err_mutex = PTHREAD_MUTEX_INITIALIZER;

static void ssannounce(const char *format, ...){
  va_list arg;
  va_start(arg, format);
  if(format == NULL){
    va_end(arg);
  } else {
    if(SALSPAWNER_DEBUG){
      pthread_mutex_lock(&iop_err_mutex);
      vfprintf(stderr, format, arg);
      pthread_mutex_unlock(&iop_err_mutex);
    }
    va_end(arg);
  }
  return;
}

static void SALspawner_sigchild_handler(int sig){
  /* for the prevention of zombies */
  pid_t child;
  int status;
  child = waitpid(-1, &status, WNOHANG); 
  child = wait(&status);
  ssannounce("SALspawner waited on child with pid %d with exit status %d\n", 
		       child, status);
}

static int SALspawner_installHandler(){
  struct sigaction sigactchild;
  sigactchild.sa_handler = SALspawner_sigchild_handler;
  sigactchild.sa_flags = SA_NOCLDSTOP;
  sigfillset(&sigactchild.sa_mask);
  return sigaction(SIGCHLD, &sigactchild, NULL);
}

int main(int argc, char** argv){
  msg *message = NULL;
  char childName[SIZE];
  int  retval;
  char *sender, *rest, *body, *cmd;

  if(argv == NULL){
    fprintf(stderr, "didn't understand: (argv == NULL)\n");
    exit(EXIT_FAILURE);
  }
  if(argc != 3){
    fprintf(stderr, "didn't understand: (argc != 3)\n");
    exit(EXIT_FAILURE);
  }
  
  iop_pid = getppid();
  myName = argv[0];
  registry_fifo_in  = argv[1];
  registry_fifo_out = argv[2];
  
  if(SALspawner_installHandler() != 0){
    perror("SALspawner could not install signal handler");
    exit(EXIT_FAILURE);
  }
  
  while(1){
    requestNo++;
    if (message != NULL){
      freeMsg(message);
      message = NULL;
    }
    
    message = acceptMsg(STDIN_FILENO);
    if(message == NULL){
      perror("SALspawner acceptMsg failed");
      continue;
    }
    
    ssannounce("Received message->data = \"%s\"\n", message->data);
    retval = parseActorMsg(message->data, &sender, &body);
    if(!retval){
      fprintf(stderr, "didn't understand: (parseActorMsg)\n\t \"%s\" \n", message->data);
      continue;
    }
    if(getNextToken(body, &cmd, &rest) != 1){
      fprintf(stderr, "didn't understand: (cmd)\n\t \"%s\" \n", body);
      continue;
    }
    if(!strcmp(cmd, "opensalactor")){
      sprintf(childName, "%s%d", clientChildName, clientNo);
      clientChildArgv[0] = childName;
      ssannounce("clientChildArgv[0] = %s\n", clientChildArgv[0]);
      clientChildArgv[1] = registry_fifo_in;
      ssannounce("clientChildArgv[1] = %s\n", clientChildArgv[1]);
      clientChildArgv[2] = registry_fifo_out;
      ssannounce("clientChildArgv[2] = %s\n", clientChildArgv[2]);
      clientChildArgv[3] = NULL;
      ssannounce("clientChildArgv[3] = %s\n", clientChildArgv[3]);
      ssannounce("Spawning SALactor\n");
      if(newActor(1, clientChildExe, clientChildArgv) == NULL) goto openclientfail;
      ssannounce("%s\n%s\nopenClientOK\n%s\n", sender, myName, childName);
      sendFormattedMsgFP(stdout,
			 "%s\n%s\nopenClientOK\n%s\n", 
			 sender, myName, childName);
      clientNo++;
      continue;
      
    openclientfail:
      ssannounce("%s\n%s\nopenSALActorFailure\n", sender, myName);
      sendFormattedMsgFP(stdout, "%s\n%s\nopenSALActorFailure\n", sender, myName);
      continue;
    }
    else {
      fprintf(stderr, "didn't understand: (command)\n\t \"%s\" \n", message->data);
      continue;
    }
  }
}


