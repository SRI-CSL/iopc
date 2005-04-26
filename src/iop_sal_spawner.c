#include "cheaders.h"
#include "constants.h"
#include "types.h"
#include "actor.h"
#include "msg.h"
#include "iop_lib.h"
#include "externs.h"
#include "dbugflags.h"
#include "wrapper_lib.h"
#include "ec.h"

int   local_debug_flag  = SALSPAWNER_DEBUG;
char* local_process_name;

static int    requestNo = 0;
static char*  myName;
static int    clientNo = 0;
static char*  clientChildExe = "iop_sal_actor";
static char*  clientChildArgv[4];
static char   clientChildName[] = "SALActor";


static void child_handler(int sig){
  /* for the prevention of zombies */
  pid_t child;
  int status;
  child = waitpid(-1, &status, WNOHANG); 
  announce("SALspawner waited on child with pid %d with exit status %d\n", 
	   child, status);
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
  local_process_name = myName = argv[0];
  registry_fifo_in  = argv[1];
  registry_fifo_out = argv[2];

  
  ec_neg1( wrapper_installHandler(child_handler, wrapper_sigint_handler) );

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
    
    announce("Received message->data = \"%s\"\n", message->data);
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
      announce("clientChildArgv[0] = %s\n", clientChildArgv[0]);
      clientChildArgv[1] = registry_fifo_in;
      announce("clientChildArgv[1] = %s\n", clientChildArgv[1]);
      clientChildArgv[2] = registry_fifo_out;
      announce("clientChildArgv[2] = %s\n", clientChildArgv[2]);
      clientChildArgv[3] = NULL;
      announce("clientChildArgv[3] = %s\n", clientChildArgv[3]);
      announce("Spawning SALactor\n");
      if(newActor(1, clientChildExe, clientChildArgv) == NULL) goto openclientfail;
      announce("%s\n%s\nopenClientOK\n%s\n", sender, myName, childName);
      sendFormattedMsgFP(stdout,
			 "%s\n%s\nopenClientOK\n%s\n", 
			 sender, myName, childName);
      clientNo++;
      continue;
      
    openclientfail:
      announce("%s\n%s\nopenSALActorFailure\n", sender, myName);
      sendFormattedMsgFP(stdout, "%s\n%s\nopenSALActorFailure\n", sender, myName);
      continue;
    }
    else {
      fprintf(stderr, "didn't understand: (command)\n\t \"%s\" \n", message->data);
      continue;
    }
  }
  exit(EXIT_SUCCESS);

EC_CLEANUP_BGN
  exit(EXIT_FAILURE);
EC_CLEANUP_END

}


