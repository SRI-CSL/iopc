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
#include "msg.h"
#include "iop_lib.h"
#include "dbugflags.h"
#include "wrapper_lib.h" 
#include "externs.h"
#include "ec.h"

/* externs used in the announce routine */
int   local_debug_flag  = MAUDE_WRAPPER_DEBUG;
char* local_process_name;

static char current_dir[PATH_MAX + 1];
static char* maudebindir;
static char* self;

#ifdef _LINUX
static char maude_exe[] = "maude.linux";
#elif defined(_MAC)
static char maude_exe[] = "maude.darwin";
#endif
static char* maude_argv[] = {"maude", "-no-tecla", "-interactive", NULL};

static void chld_handler(int sig){
  fprintf(stderr, "%s died! Exiting\n", maude_exe);
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
  if(MAUDE_WRAPPER_DEBUG)errDump(buff, bytesI);
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

static void wait4Maude(int fdout, int fderr){
  int maxfd = (fderr < fdout) ? fdout + 1 : fderr + 1;
  fd_set readset;
  int retval;
  announce("entering wait4Maude(pout[0], perr[0]);\n"); 
  FD_ZERO(&readset);
  FD_SET(fdout, &readset);
  FD_SET(fderr, &readset);
  retval = select(maxfd, &readset, NULL, NULL, NULL);
  announce("wait4Maude\t:\tselect returned %d (out: %d) (err: %d)\n", 
	   retval, FD_ISSET(fdout, &readset), FD_ISSET(fderr, &readset));
  if(retval < 0){
    fprintf(stderr, "wait4Maude\t:\tselect error\n");
  } else if(retval == 0){
    fprintf(stderr, "wait4Maude\t:\tselect returned 0\n");
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
	fprintf(stderr, "wait4Maude\t:\tthis shouldn't happen\n");
	wait4Maude(fdout, fderr);
	goto exit;
      } else {
	/* OK */
      }
    }
    if(FD_ISSET(fdout, &readset))
      parseMaudeThenEcho(fdout, STDOUT_FILENO);
  }

 exit:
  announce("exiting wait4Maude(pout[0], perr[0]);\n"); 
  return;
}

int main(int argc, char** argv){
  int pin[2], pout[2], perr[2];
  if((argc != 2)  && (argc != 3)){
    fprintf(stderr, "Usage: %s <maude bin dir>  [maude module]\n", argv[0]);
  }
  local_process_name = self = argv[0];
  maudebindir = argv[1];

  ec_neg1( wrapper_installHandler(chld_handler, wrapper_sigint_handler) );

  ec_null( getcwd(current_dir, PATH_MAX) );

  ec_neg1( pipe(pin) );
  ec_neg1( pipe(perr) );
  ec_neg1( pipe(pout) );

  /*it's time to fork */
  ec_neg1( child = fork() );

  if(child == 0){
    /* i'm destined to be maude */
    ec_neg1( dup2(pin[0],  STDIN_FILENO) );
    ec_neg1( dup2(perr[1], STDERR_FILENO) );
    ec_neg1( dup2(pout[1], STDOUT_FILENO) );

    ec_neg1( close(pin[0]) );
    ec_neg1( close(perr[1]) );
    ec_neg1( close(pout[1]) );

    ec_neg1( chdir(maudebindir) );

    ec_neg1( execvp(maude_exe, maude_argv) );

    /* end of child code */
  } else { 
    /* i'm the boss */
    char cmdBuff[PATH_MAX + SIZE];
    int len;
    
    ec_neg1( close(pin[0]) );
    ec_neg1( close(perr[1]) );
    ec_neg1( close(pout[1]) );
    
    wait4Maude(pout[0], perr[0]);
    
    announce("%s\t:\tcding to %s\n", argv[0], current_dir); 
    
    sprintf(cmdBuff, "cd %s\n", current_dir);
    len = strlen(cmdBuff);
    if(write(pin[1], cmdBuff, len) != len){
      fprintf(stderr, "write failed of \"%s\" command", cmdBuff);
      /* forge on, notify registry, die calmly? */
      exit(EXIT_FAILURE);
    };
    announce(cmdBuff); 
    wait4Maude(pout[0], perr[0]);
    
    if(argc == 3){
      announce("%s\t:\tloading %s\n", argv[0], argv[2]); 
      
      sprintf(cmdBuff, "load %s\n", argv[2]);
      len = strlen(cmdBuff);
      if(write(pin[1], cmdBuff, len) !=  len){
	fprintf(stderr, "write failed of \"%s\" command", cmdBuff);
	/* forge on, notify registry, die calmly? */
	exit(EXIT_FAILURE);
      };
      announce(cmdBuff);
      wait4Maude(pout[0], perr[0]);
    }
          
    while(1){
      announce("Listening to IO\n");
      echo2Maude(STDIN_FILENO, pin[1]);
      announce("Listening to Maude\n");
      wait4Maude(pout[0], perr[0]);
    }
  } /* end of boss code */

  exit(EXIT_SUCCESS);

EC_CLEANUP_BGN
  exit(EXIT_FAILURE);
EC_CLEANUP_END

}



