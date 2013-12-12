/* 
   N.B. This code is testing the ec.h error macros!
   See: "Advanced UNIX Programming" 
         Marc J. Rochkind
         2nd Edition 2004
	 Addison Wesley.
*/
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
#include "argv.h"
#include "ec.h"
#include "sal_lib.h"


static int child_died = 0;

static int requestNo = 0;
static char ** sal_argv;
static int pin[2], pout[2], perr[2];
static int sal_argc;

static void intr_handler(int sig){
  char sal_exit[] = "(exit)\n";
  if(child > 0){
    write(pin[1], sal_exit, strlen(sal_exit));
  }
  _exit(EXIT_FAILURE);
}

static void chld_handler(int sig){
  announce("\nSAL died! Exiting\n"); 
  child_died = 1;
}

int main(int argc, char** argv){
  msg *messageIn = NULL, *messageOut = NULL;
  char *sender, *body; 
  int retval;
  if(argv == NULL){
    fprintf(stderr, "didn't understand: (argv == NULL)\n");
    exit(EXIT_FAILURE);
  }

  self_debug_flag  = SAL_ACTOR_DEBUG;
  self = argv[0];

  ec_neg1( wrapper_installHandler(chld_handler, intr_handler) );


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
    announce("%s waiting to process request number %d\n", self, requestNo);
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
    if ((sal_argc = makeArgv(body," \t\n",&sal_argv)) <= 1 ){
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
  
    ec_neg1( pipe(pin) );
    ec_neg1( pipe(perr) );
    ec_neg1( pipe(pout) );

    /*it's time to fork */
    ec_neg1( child = fork() );

    if(child == 0){
      /* i'm destined to be sal */
      ec_neg1( dup2(pin[0],  STDIN_FILENO) );
      ec_neg1( dup2(perr[1], STDERR_FILENO) );
      ec_neg1( dup2(pout[1], STDOUT_FILENO) );

      ec_neg1( close(pin[0]) );
      ec_neg1( close(perr[1]) );
      ec_neg1( close(pout[1]) );

      ec_neg1( execvp(sal_argv[0], sal_argv) );

    } else { 
      /* I am the boss */
      msg *response = NULL;
      pthread_t errThread;
      fdBundle errFdB, outFdB;

      /* for monitoring the error stream of SAL */
      errFdB.fd = perr[0];
      errFdB.exit = &child_died;

      /* for monitoring the output stream of SAL */
      outFdB.fd = pout[0];
      outFdB.exit = &child_died;

      ec_neg1( close(pin[0]) );
      ec_neg1( close(perr[1]) );
      ec_neg1( close(pout[1]) );
      
      ec_rv( pthread_create(&errThread, NULL, echoErrorsSilently, &errFdB) );

      response = readSALMsg(&outFdB);

      if((response != NULL) && (response->bytesUsed > 0)){
	sendFormattedMsgFD(STDOUT_FILENO, "%s\n%s\n%s\n", sender, self, response->data);
      }
    }
    iop_usleep(100);
    sendFormattedMsgFD(STDOUT_FILENO, "system\n%s\nstop %s\n", self, self);
  }

  exit(EXIT_SUCCESS);

EC_CLEANUP_BGN
  exit(EXIT_FAILURE);
EC_CLEANUP_END

}


