#include "cheaders.h"
#include "constants.h"
#include "types.h"
#include "actor.h"
#include "msg.h"
#include "iop_lib.h"
#include "socket_lib.h"
#include "externs.h"
#include "dbugflags.h"
#include "wrapper_lib.h"
#include "sal_lib.h"
#include "argv.h"

static volatile int child_died = 0;


static int requestNo = 0;
static char* myname;
static char ** sal_argv;
static int pin[2], pout[2], perr[2];
static int sal_argc;

static void sal_actor_sigint_handler(int sig){
  char sal_exit[] = "(exit)\n";
  if(child > 0){
    write(pin[1], sal_exit, strlen(sal_exit));
  }
  _exit(EXIT_FAILURE);
}
static void sal_actor_sigchild_handler(int sig){
  if (SAL_ACTOR_DEBUG){
    fprintf(stderr, "\nSAL died! Exiting\n"); 
  }
  child_died = 1;
}

static void sal_actor_installHandler(){
  struct sigaction sigactchild;
  struct sigaction sigactint;

  sigactchild.sa_handler = sal_actor_sigchild_handler;
  sigactchild.sa_flags = SA_NOCLDSTOP;
  sigfillset(&sigactchild.sa_mask);
  sigaction(SIGCHLD, &sigactchild, NULL);

  sigactint.sa_handler = sal_actor_sigint_handler;
  sigactint.sa_flags = 0;
  sigfillset(&sigactint.sa_mask);
  sigaction(SIGINT, &sigactint, NULL);
}

int main(int argc, char** argv){
  msg *messageIn = NULL, *messageOut = NULL;
  char *sender, *body; 
  int retval;
  if(argv == NULL){
    fprintf(stderr, "didn't understand: (argv == NULL)\n");
    exit(EXIT_FAILURE);
  }
  sal_actor_installHandler();
  myname = argv[0];
  while(1){
    requestNo++;
    if(messageIn  != NULL){ 
      freeMsg(messageIn);
      messageIn = NULL;
    }  
    if(messageOut != NULL){
      freeMsg(messageOut);
      messageOut = NULL;
    }
    if(SAL_ACTOR_DEBUG)
      fprintf(stderr, "%s waiting to process request number %d\n", myname, requestNo);
    messageIn = acceptMsg(STDIN_FILENO);
    if(messageIn == NULL){
      perror("sal_actor: acceptMsg failed");
      continue;
    }
    retval = parseActorMsg(messageIn->data, &sender, &body);
    if(!retval){
      fprintf(stderr, "didn't understand: (parseActorMsg)\n\t \"%s\" \n", messageIn->data);
      continue;
    }
    if ((sal_argc = makeArgv(body," \t\n",&sal_argv)) <=1 ){
      fprintf(stderr,"\nAn error occured in makeArgv\n");
      exit(EXIT_FAILURE);
    }
    
    if(strcmp(sal_argv[0], "sal-smc")         && 
       strcmp(sal_argv[0], "sal-bmc")         && 
       strcmp(sal_argv[0], "sal-path-finder") &&
       strcmp(sal_argv[0], "sal-deadlock-checker")){
      fprintf(stderr, "didn't understand: (command)\n\t \"%s\" \n", messageIn->data);
      freeArgv(sal_argc, sal_argv);
      continue;
    }
  
    if((pipe(pin) != 0) || 
       (pipe(perr) != 0) ||
       (pipe(pout) != 0)){
      perror("couldn't make pipes");
      exit(EXIT_FAILURE);
    } 
    /*it's time to fork */
    child = fork();
    if(child < 0){
      perror("couldn't fork");
      exit(EXIT_FAILURE);
    }

    if(child == 0){
      /* i'm destined to be sal */
      if((dup2(pin[0],  STDIN_FILENO) < 0)  ||
	 (dup2(perr[1], STDERR_FILENO) < 0) ||
	 (dup2(pout[1], STDOUT_FILENO) < 0)){
	perror("couldn't dup fd's");
	exit(EXIT_FAILURE);
      }
      if((close(pin[0]) !=  0) ||
	 (close(perr[1]) !=  0) ||
	 (close(pout[1]) !=  0)){
        perror("couldn't close fd's");
        exit(EXIT_FAILURE);
      }
      execvp(sal_argv[0], sal_argv);
      perror("couldn't execvp");
      exit(EXIT_FAILURE);
    }else { 
      /* I am the boss */
      msg *response = NULL;
      pthread_t errThread;
      fdBundle  errFdB, outFdB;

      /* for monitoring the error stream of SAL */
      errFdB.fd = perr[0];
      errFdB.exit = child_died;

      /* for monitoring the output stream of SAL */
      outFdB.fd = pout[0];
      outFdB.exit = child_died;
    

      if((close(pin[0])  !=  0) ||
	 (close(perr[1]) !=  0) ||
	 (close(pout[1]) !=  0)){
	perror("couldn't close fd's");
	exit(EXIT_FAILURE);
      }
      

      if(pthread_create(&errThread, NULL, echoErrorsSilently, &errFdB)){
	fprintf(stderr, "Could not spawn echoErrorsSilently thread\n");
	exit(EXIT_FAILURE);
      }

      response = readSALMsg(&outFdB);
	
      if((response != NULL) && (response->bytesUsed > 0)){
	sendFormattedMsgFD(STDOUT_FILENO, "%s\n%s\n%s\n", sender, myname, response->data);
      }
    }
    usleep(100);
    sendFormattedMsgFD(STDOUT_FILENO, "system\n%s\nstop %s\n", myname, myname);
  }
}


