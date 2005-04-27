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
#include "iop_lib.h"
#include "registry_lib.h"
#include "externs.h"
#include "dbugflags.h"
#include "options.h"
#include "msg.h"

int   local_debug_flag;
char* local_process_name;

static int fifoOut;
static int fifoIn;


void *registryCommandThread(void* arg){
  while(1){
    processRegistryCommand(fifoIn, fifoOut, 1);
  }
}

int main(int argc, char** argv){
  pthread_t inThread;
  pthread_t commandThread;

#ifdef _LINUX
  parseOptions(argc, argv, short_options, long_options);
#elif defined(_MAC)
  parseOptions(argc, argv, short_options);
#endif

  local_debug_flag  = (REGISTRY_DEBUG || iop_debug_flag);
  local_process_name = argv[0];

  fprintf(stderr, "%s local_debug_flag = %d\n", argv[0], local_debug_flag);  

  announce("optind = %d\n", optind);


  /* set externs */
  iop_pid           = getppid();
  registry_fifo_in  = argv[argc - 5];
  registry_fifo_out = argv[argc - 4];
  in2RegPort        = atoi(argv[argc - 3]);
  in2RegFd          = atoi(argv[argc - 2]);
  iop_bin_dir       = argv[argc - 1];
  registry_pid      = getpid();

  assert(strlen(registry_fifo_in)  < PATH_MAX);
  assert(strlen(registry_fifo_out) < PATH_MAX);
  assert(strlen(iop_bin_dir)       < PATH_MAX);
  

  /* install signal handlers */
  if(registry_installHandler() != 0){
    perror("could not install signal handler");
    goto killIOP;
  }


  announce("Calling registryInit\n");

  if(registryInit(&fifoIn, &fifoOut) < 0){
    fprintf(stderr, "registryInit failed\n");
    goto killIOP;
  }
  

  announce("Calling errorsInit\n");

  if(errorsInit() < 0){
    fprintf(stderr, "errorsInit failed\n");
    goto killIOP;
  }
 
  announce("sending ready message to iop\n");
  if(write(fifoOut, REGREADY, strlen(REGREADY)) != strlen(REGREADY)){
    fprintf(stderr, "ready message to iop failed\n");
    goto killIOP;
  }
 
  /* iop will send two notifications, one for the system, and one for the GUI */
  processRegistryCommand(fifoIn, fifoOut, 0);
  processRegistryCommand(fifoIn, fifoOut, 0);


  announce("creating monitorInSocket thread\n");
  if(pthread_create(&inThread, NULL, monitorInSocket, &in2RegFd) != 0){
    fprintf(stderr, "thread creation failed\n");
    bail();
  }

  announce("creating command thread\n");
  if(pthread_create(&commandThread, NULL, registryCommandThread, NULL) != 0){
    fprintf(stderr, " command thread creation failed\n");
    bail();
  }

  if(!iop_hardwired_actors_flag){
    announce("reading configuration file\n");
    if(registryProcessConfigFile() < 0){
      announce("configuration file reading failed\n");
    }
  }

  {
    int requestNo = 0;
    msg* message = NULL;
    char *sender, *body;
    int retval;

    while(1){
      requestNo++;
      freeMsg(message);
      message = acceptMsg(STDIN_FILENO);
      if(message == NULL){
	perror("registry readMsg failed");
	continue;
      }
      announce("received:\"%s\"\n", message->data);
      retval = parseActorMsg(message->data, &sender, &body);
      if(!retval){
	fprintf(stderr, "registry didn't understand: \n\t \"%s\" \n", message->data);
	continue;
      }
      if(body == NULL){
	fprintf(stderr, "registry didn't understand: (body == NULL)\n");
      } else {
	processRegistryMessage(sender, body);
      }
    }
  }



  if(pthread_join(commandThread, NULL) != 0){
    fprintf(stderr, "command thread creation failed\n");
    bail();
  }
  
 killIOP:
  kill(iop_pid, SIGKILL);
  exit(EXIT_SUCCESS);
}

