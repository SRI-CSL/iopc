#include "cheaders.h"
#include "constants.h"
#include "msg.h"
#include "iop_lib.h"
#include "dbugflags.h"
#include "wrapper_lib.h" 
#include "externs.h"
#include "ec.h"
#include "sal_lib.h"

/* externs used in the announce routine */
int   local_debug_flag  = SALWRAPPER_DEBUG;
char* local_process_name;

static char* self;

static char sal_exe[] = "salenv";
static char* sal_argv[] = {"salenv", NULL};

static void chld_handler(int sig){
  fprintf(stderr, "%s died! Exiting\n", sal_exe);
  sendFormattedMsgFD(STDOUT_FILENO, "system\n%s\nstop %s\n", self, self);
}

int main(int argc, char** argv){
  int pin[2], pout[2], perr[2];
  local_process_name = self = argv[0];
  
  ec_neg1( wrapper_installHandler(chld_handler, wrapper_sigint_handler) );
  
  ec_neg1( pipe(pin) );
  ec_neg1( pipe(perr) );
  ec_neg1( pipe(pout) );

  /*it's time to fork */
  ec_neg1( child = fork() );

  if(child == 0){
    /* i'm destined to be Sal */
    ec_neg1( dup2(pin[0],  STDIN_FILENO) );
    ec_neg1( dup2(perr[1], STDERR_FILENO) );
    ec_neg1( dup2(pout[1], STDOUT_FILENO) );

    ec_neg1( close(pin[0]) );
    ec_neg1( close(perr[1]) );
    ec_neg1( close(pout[1]) );

    ec_neg1( execvp(sal_exe, sal_argv) );

    /* end of child code */
  } else { 
    /* i'm the boss */
    
    ec_neg1( close(pin[0]) );
    ec_neg1( close(perr[1]) );
    ec_neg1( close(pout[1]) );
    
    wait4IO(pout[0], perr[0],parseSalThenEcho);
    
    while(1){
      announce("Listening to IO and then echoing to the salenv's input\n");
      echo2Input(STDIN_FILENO, pin[1]);
      announce("Listening to salenv\n");
      wait4IO(pout[0], perr[0],parseSalThenEcho);
    }
  } /* end of boss code */

  exit(EXIT_SUCCESS);

EC_CLEANUP_BGN
  exit(EXIT_FAILURE);
EC_CLEANUP_END

}
