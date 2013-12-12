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
#include "actor.h"
#include "msg.h"
#include "iop_lib.h"
#include "socket_lib.h"
#include "externs.h"
#include "dbugflags.h"


static char* myname;
static int sock;

static void remote_actor_sigpipe_handler(int sig){
  if(SOCKET_DEBUG)
    fprintf(stderr, 
  	    "remote_actor %s got a signal %d\n", 
  	    myname, sig);	
  terminateIOP();
}

static int remote_actor_installHandler(void){
  struct sigaction sigactpipe;
  sigactpipe.sa_handler = remote_actor_sigpipe_handler;
  sigactpipe.sa_flags = 0;
  /*  sigfillset(&sigactpipe.sa_mask); */
  sigemptyset(&sigactpipe.sa_mask);
  return sigaction(SIGPIPE, &sigactpipe, NULL);
}

int main(int argc, char** argv){
  echofds fdsR2A, fdsA2R;
  pthread_t thrR2A, thrA2R;

  if(argv == NULL){
    fprintf(stderr, "didn't understand: (argv == NULL)\n");
    goto fail;
  }
  if(argc != 4){
    fprintf(stderr, "didn't understand: (argc != 4)\n");
    goto fail;
  }

  myname = argv[0];
  sock = atoi(argv[1]);
  registry_fifo_in  = argv[2];
  registry_fifo_out = argv[3];

  if(remote_actor_installHandler() != 0){
    perror("remote_actor couldn't install handler\n");
    goto fail;
  }

  fdsR2A.from = STDIN_FILENO;
  fdsR2A.to = sock;
 
  fdsA2R.from = sock;
  fdsA2R.to = STDOUT_FILENO;

  if(pthread_create(&thrR2A, NULL, echoLoop, &fdsR2A) != 0){
    fprintf(stderr, "couldn't create thrR2A thread\n");
    goto fail;
  };

  if(pthread_create(&thrA2R, NULL, echoLoopDieOnFail, &fdsA2R) != 0){
    fprintf(stderr, "couldn't create thrA2R thread\n");
    goto fail;
  };

  pthread_join(thrA2R, NULL);
  exit(EXIT_SUCCESS);

 fail:
  terminateIOP();
  exit(EXIT_FAILURE);
}
