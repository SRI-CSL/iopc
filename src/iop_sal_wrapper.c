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
    fprintf(stderr, "echo(%d,%d)\t:\terror bytesI = %d\n", 
	    from, to, bytesI);
    return;
  }
  if(SALWRAPPER_DEBUG)errDump(buff, bytesI);
  if((bytesO = write(to, buff, bytesI)) != bytesI){
    fprintf(stderr, "echo(%d,%d)\t:\terror bytes0 != bytesI (%d != %d)\n", 
	    from, to, bytesO, bytesI);
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
      announce("reverberate(%d,%d)\t:\tbreaking retval = %d\n",from, to, retval);
      break;
    } else {
      announce("reverberate(%d,%d)\t:\titeration = %d\n", from, to, iteration);
      echoChunk(from, to);
      iteration++;
    }
  }/* while */
}

static void wait4Sal(int fdout, int fderr){
  int maxfd = (fderr < fdout) ? fdout + 1 : fderr + 1;
  fd_set readset;
  int retval;
  announce("entering wait4Sal(pout[0], perr[0]);\n"); 
  FD_ZERO(&readset);
  FD_SET(fdout, &readset);
  FD_SET(fderr, &readset);
  retval = select(maxfd, &readset, NULL, NULL, NULL);
  announce("wait4Sal\t:\tselect returned %d (out: %d) (err: %d)\n", 
	   retval, FD_ISSET(fdout, &readset), FD_ISSET(fderr, &readset));
  if(retval < 0){
    fprintf(stderr, "wait4Sal\t:\tselect error\n");
  } else if(retval == 0){
    fprintf(stderr, "wait4Sal\t:\tselect returned 0\n");
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
	fprintf(stderr, "wait4Sal\t:\tthis shouldn't happen\n");
	wait4Sal(fdout, fderr);
	goto exit;
      } else {
	/* OK */
      }
    }
    if(FD_ISSET(fdout, &readset)){
      parseSalThenEcho(fdout, STDOUT_FILENO);
    }
  }

 exit:
  announce("exiting wait4Sal(pout[0], perr[0]);\n"); 
  return;
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
    
    wait4Sal(pout[0], perr[0]);
    
    while(1){
      announce("Listening to IO\n");
      echo2Sal(STDIN_FILENO, pin[1]);
      announce("Listening to Sal\n");
      wait4Sal(pout[0], perr[0]);
    }
  } /* end of boss code */

  exit(EXIT_SUCCESS);

EC_CLEANUP_BGN
  exit(EXIT_FAILURE);
EC_CLEANUP_END

}



