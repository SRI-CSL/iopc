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
#include "dbugflags.h"
#include "wrapper_lib.h"
#include "iop_lib.h"
#include "ec.h"
#include "externs.h"

int   local_debug_flag  = G2D_ACTOR_DEBUG;
char* local_process_name;

static char* myName;
static char  graphics_exe[] = "java";
static char* graphics_argv[] = {"java", "-cp", NULL, "g2d.Main", NULL, NULL};

static int child_died = 0;

static void chld_handler(int sig){
  fprintf(stderr, "%s died! Exiting\n", graphics_argv[3]);
  child_died = 1;
  sendFormattedMsgFD(STDOUT_FILENO, "system\n%s\nstop %s\n", myName, myName);
}

int main(int argc, char** argv){
  int pin[2], pout[2], perr[2];
  if((argc != 2)){
    fprintf(stderr, "Usage: %s <iop bin directory>\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  local_process_name = myName = argv[0];

  if((graphics_argv[2] = iop_alloc_jarpath(argv[1], myName)) == NULL){
    exit(EXIT_FAILURE);
  }

  graphics_argv[4] = myName;

  ec_neg1( wrapper_installHandler(chld_handler, wrapper_sigint_handler) );

  ec_neg1( pipe(pin) );
  ec_neg1( pipe(perr) );
  ec_neg1( pipe(pout) );

  /*it's time to fork */
  ec_neg1( child = fork() );

  if(child == 0){
    /* i'm destined to be the java graphics program */
    ec_neg1( dup2(pin[0],  STDIN_FILENO) );
    ec_neg1( dup2(perr[1], STDERR_FILENO) );
    ec_neg1( dup2(pout[1], STDOUT_FILENO) );

    ec_neg1( close(pin[0])  );
    ec_neg1( close(perr[1]) );
    ec_neg1( close(pout[1]) );

    ec_neg1( execvp(graphics_exe, graphics_argv) );

    /* end of child code */
  } else {
    /* i'm the boss */
    pthread_t errThread, outThread;
    msg* message = NULL;
    int requestNo = 0;
    fdBundle  errFdB, outFdB;

    /* for monitoring the error stream */
    errFdB.fd = perr[0];
    errFdB.exit = &child_died;

    /* for monitoring the output stream */
    outFdB.fd = pout[0];
    outFdB.exit = &child_died;

    ec_neg1( close(pin[0])  );
    ec_neg1( close(perr[1]) );
    ec_neg1( close(pout[1]) );

    ec_rv( pthread_create(&errThread, NULL, echoErrorsSilently, &errFdB) );

    ec_rv( pthread_create(&outThread, NULL, wrapper_echoOutSilently, &outFdB) );

      
    while(1){
      int size;
      requestNo++;
      freeMsg(message);
      message = acceptMsg(STDIN_FILENO);
      if(message == NULL){
	perror("graphics readMsg failed");
	continue;
      }
      size = message->bytesUsed;
      sendMsg(pin[1], message);
    }
    /* end of boss code */
  }

  exit(EXIT_SUCCESS);

EC_CLEANUP_BGN
  exit(EXIT_FAILURE);
EC_CLEANUP_END

}

