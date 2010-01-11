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
#include "ec.h"

static int fifoOut;
static int fifoIn;

static void *registryCommandThread(void* arg){
  while(1){
    processRegistryCommand(fifoIn, fifoOut, 1);
  }
}

int main(int argc, char** argv){
  pthread_t inThread, commandThread;
#ifdef _LINUX
  parseOptions(argc, argv, short_options, long_options);
#elif defined(_MAC)
  parseOptions(argc, argv, short_options);
#endif
  self_debug_flag  = (REGISTRY_DEBUG || iop_debug_flag);
  self = argv[0];

  /* fprintf(stderr, "%s self_debug_flag = %d\n", argv[0], self_debug_flag);  */
  
   

  /* set externs */
  iop_pid           = getppid();
  registry_fifo_in  = argv[argc - 5];
  registry_fifo_out = argv[argc - 4];
  in2RegPort        = atoi(argv[argc - 3]);
  in2RegFd          = atoi(argv[argc - 2]);
  iop_bin_dir       = argv[argc - 1];
  registry_pid      = getpid();

  /* fprintf(stderr, "iop_startup_file = %s\n", iop_startup_file);  */

    
  /*
    if(atexit(bail) != 0){
    fprintf(stderr, "atexit(bail) failed\n");
    goto killIOP;
    }
  */

  if(errorsInit() < 0){
    fprintf(stderr, "errorsInit failed\n");
    goto killIOP;
  }
  
  fprintf(stderr, "%s self_debug_flag = %d REGISTRY_DEBUG = %d\n", argv[0], self_debug_flag, REGISTRY_DEBUG); 
  
  /* rethink this if they ever fail */
  assert(strlen(registry_fifo_in)  < PATH_MAX);
  assert(strlen(registry_fifo_out) < PATH_MAX);
  assert(strlen(iop_bin_dir)       < PATH_MAX);
  
  /* install signal handlers */
  if(registry_installHandler() != 0){
    perror("could not install signal handler");
    goto killIOP;
  }
  
  log2File("Calling registryInit\n");

  if(registryInit(&fifoIn, &fifoOut) < 0){
    fprintf(stderr, "registryInit failed\n");
    goto killIOP;
  }

 
  log2File("sending ready message to iop\n");
  if(mywrite(fifoOut, REGREADY, strlen(REGREADY), 1) != strlen(REGREADY)){
    fprintf(stderr, "ready message to iop failed\n");
    goto killIOP;
  }
 
  /* 
     iop will send at least one notification, one for the system, 
     and one for the GUI, if not in "no windows mode". 
  */
  processRegistryCommand(fifoIn, fifoOut, 0);
  if(!iop_no_windows_flag){
    processRegistryCommand(fifoIn, fifoOut, 0);
  }

  log2File("creating monitorInSocket thread\n");
  ec_rv( pthread_create(&inThread, NULL, monitorInSocket, &in2RegFd) );

  log2File("creating command thread\n");
  ec_rv( pthread_create(&commandThread, NULL, registryCommandThread, NULL) );

  if(!iop_hardwired_actors_flag){
    if(registryProcessStartupFile(iop_startup_file) < 0){
      log2File("startup file reading failed\n");
      log2File("reading configuration file\n");
      if(registryProcessConfigFile() < 0){
        log2File("configuration file reading failed\n");
      }
      log2File("reading startup file\n");
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
      log2File("received:\"%s\"\n", message->data);
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

  ec_rv( pthread_join(commandThread, NULL) );
  
 killIOP:
  /* kill iop, ignore failure */
  (void)kill(iop_pid, SIGKILL);
  exit(EXIT_SUCCESS);

  EC_CLEANUP_BGN
    bail();
  exit(EXIT_FAILURE);
  EC_CLEANUP_END

    }

