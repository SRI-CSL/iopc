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

static char* maudebindir;

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



int main(int argc, char** argv){
  int pin[2], pout[2], perr[2];
  if((argc != 2)  && (argc != 3)){
    fprintf(stderr, "Usage: %s <maude bin dir>  [maude module]\n", argv[0]);
  }
  self_debug_flag  = MAUDE_WRAPPER_DEBUG;
  self = argv[0];
  maudebindir = argv[1];

  ec_neg1( wrapper_installHandler(chld_handler, wrapper_sigint_handler) );

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

    ec_neg1( execvp(maude_exe, maude_argv) );

    /* end of child code */
  } else { 
    /* i'm the boss */
    char cmdBuff[PATH_MAX + SIZE];
    int len;
    
    ec_neg1( close(pin[0]) );
    ec_neg1( close(perr[1]) );
    ec_neg1( close(pout[1]) );
    
    wait4IO(pout[0], perr[0], parseMaudeThenEcho);
    
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
      wait4IO(pout[0], perr[0], parseMaudeThenEcho);
    }
          
    while(1){
      announce("Listening to IO\n");
      echo2Maude(STDIN_FILENO, pin[1]);
      announce("Listening to Maude\n");
      wait4IO(pout[0], perr[0], parseMaudeThenEcho);
    }
  } /* end of boss code */

  exit(EXIT_SUCCESS);

EC_CLEANUP_BGN
  exit(EXIT_FAILURE);
EC_CLEANUP_END

}



