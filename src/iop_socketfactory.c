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
#include "iop_utils.h"
#include "socket_lib.h"
#include "externs.h"
#include "dbugflags.h"
#include "ec.h"

static int    requestNo = 0;
static int    clientNo = 0;
static char*  clientChildExe = "iop_socket";
static char*  clientChildArgv[5];
static char   clientChildName[] = "clientsocket";
static int    clientFd;
static int    listenerNo = 0;
static char*  listenerChildExe = "iop_listener";
static char*  listenerChildArgv[7];
static char   listenerChildName[] = "listener";
static int    listenerFd;

static void socketfactory_sigchild_handler(int sig){
  /* for the prevention of zombies */
  pid_t child;
  int status;
  child = waitpid(-1, &status, WNOHANG);
  announce("Waited on child with pid %d with exit status %d\n", child, status);
}

int main(int argc, char** argv){
  msg *message = NULL;
  char childName[SIZE], fdName[SIZE], iopPid[SIZE];
  int  retval, portNo;
  char *port, *sender, *rest, *body, *cmd;

  if(argv == NULL){
    fprintf(stderr, "didn't understand: (argv == NULL)\n");
    exit(EXIT_FAILURE);
  }
  if(argc != 3){
    fprintf(stderr, "didn't understand: (argc != 3)\n");
    exit(EXIT_FAILURE);
  }

  iop_pid = getppid();
  self_debug_flag  = SOCKETFACTORY_DEBUG;
  self = argv[0];
  registry_fifo_in  = argv[1];
  registry_fifo_out = argv[2];

  if(iop_install_handler(SIGCHLD, 0, socketfactory_sigchild_handler) != 0){
    perror("socketfactory could not install signal handler");
    exit(EXIT_FAILURE);
  }

  while(1){
    requestNo++;
    freeMsg(message);
    
    message = acceptMsg(STDIN_FILENO);
    if(message == NULL){
      perror("socketfactory acceptMsg failed");
      continue;
    }
    
    announce("Received message->data = \"%s\"\n", message->data);
    retval = parseActorMsg(message->data, &sender, &body);
    if(!retval){
      fprintf(stderr, "didn't understand: (parseActorMsg)\n\t \"%s\" \n", message->data);
      continue;
    }
    if(getNextToken(body, &cmd, &rest) <= 0){
      fprintf(stderr, "didn't understand: (cmd)\n\t \"%s\" \n", body);
      continue;
    }
    if(!strcmp(cmd, "openclient")){
      char *host;
      announce("openclient case\n");
     
      if(getNextToken(rest, &host, &rest) <= 0){
        fprintf(stderr, "didn't understand: (host)\n\t \"%s\" \n", rest);
        continue;
      }
      if(getNextToken(rest, &port, &rest) <= 0){
        fprintf(stderr, "didn't understand: (port)\n\t \"%s\" \n", rest);
        continue;
      }

      portNo = atoi(port);

      announce("port = %d\n", portNo);

      if(allocateSocket(portNo, host, &clientFd) != 1){ goto openclientfail; }
      snprintf(childName, SIZE, "%s%d", clientChildName, clientNo);
      snprintf(fdName, SIZE, "%d", clientFd);
      clientChildArgv[0] = childName;
      announce("clientChildArgv[0] = %s\n", clientChildArgv[0]);
      clientChildArgv[1] = fdName;
      announce("clientChildArgv[1] = %s\n", clientChildArgv[1]);
      clientChildArgv[2] = registry_fifo_in;
      announce("clientChildArgv[2] = %s\n", clientChildArgv[2]);
      clientChildArgv[3] = registry_fifo_out;
      announce("clientChildArgv[3] = %s\n", clientChildArgv[3]);
      clientChildArgv[4] = NULL;
      announce("clientChildArgv[4] = %s\n", clientChildArgv[4]);
      announce("Spawning actor\n");
      newActor(1, clientChildExe, clientChildArgv);
      announce("%s\n%s\nopenClientOK\n%s\n", sender, self, childName);
      sendFormattedMsgFP(stdout,
			 "%s\n%s\nopenClientOK\n%s\n", 
			 sender, self, childName);
      clientNo++;
      if(close(clientFd) < 0){
	fprintf(stderr, "close failed in openclient case\n");
      }
      continue;
      
    openclientfail:
      announce("%s\n%s\nopenClientFailure\n", sender, self);
      sendFormattedMsgFP(stdout, "%s\n%s\nopenClientFailure\n", sender, self);
      continue;
    } else if(!strcmp(cmd, "openlistener")){
      announce("openlistener case\n");
      if(getNextToken(rest, &port, &rest) <= 0){
        fprintf(stderr, "didn't understand: (port)\n\t \"%s\" \n", rest);
        continue;
      }
      portNo = atoi(port);

      announce("port = %d\n", portNo);

      if(allocateListeningSocket(portNo, &listenerFd) != 1){
        fprintf(stderr, "Couldn't listen on port\n");
        goto openlistenerfail;
      }
      snprintf(childName, SIZE, "%s%d", listenerChildName, listenerNo);
      snprintf(fdName, SIZE, "%d", listenerFd);
      snprintf(iopPid, SIZE, "%d", iop_pid);
      listenerChildArgv[0] = childName;
      announce("listenerChildArgv[0] = %s\n", listenerChildArgv[0]);
      listenerChildArgv[1] = fdName;
      announce("listenerChildArgv[1] = %s\n", listenerChildArgv[1]);
      listenerChildArgv[2] = sender;
      announce("listenerChildArgv[2] = %s\n", listenerChildArgv[2]);
      listenerChildArgv[3] = registry_fifo_in;
      announce("listenerChildArgv[3] = %s\n", listenerChildArgv[3]);
      listenerChildArgv[4] = registry_fifo_out;
      announce("listenerChildArgv[4] = %s\n", listenerChildArgv[4]);
      listenerChildArgv[5] = iopPid;
      announce("listenerChildArgv[5] = %s\n", listenerChildArgv[5]);
      listenerChildArgv[6] = NULL;
      announce("listenerChildArgv[6] = %s\n", listenerChildArgv[6]);
      announce("Spawning actor\n");
      newActor(1, listenerChildExe, listenerChildArgv);
      announce("Spawned actor\n");
      announce("%s\n%s\nopenListenerOK\n%s\n", 
                sender, self, childName);
      sendFormattedMsgFP(stdout,
			 "%s\n%s\nopenListenerOK\n%s\n", 
			 sender, self, childName);
      listenerNo++;
      if(close(listenerFd) < 0){
	fprintf(stderr, "close failed in openlistener case\n");
      }
      continue;

    openlistenerfail:
      announce("%s\n%s\nopenListenerFailure\n", sender, self);
      sendFormattedMsgFP(stdout, "%s\n%s\nopenListenerFailure\n", sender, self);
      continue;
    } else {
      fprintf(stderr, "didn't understand: (command)\n\t \"%s\" \n", message->data);
      continue;
    }
  }
}


