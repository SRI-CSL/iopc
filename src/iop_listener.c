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
 
static int    requestNo = 0;
static char*  myName;
static char*  myClient;
static int    connectionNo = 0;
static char*  childExe = "iop_socket";
static char*  childArgv[5];
static char   childName[] = "connectionsocket";
static int    listenFd;
static int    closed = 0;

static void *listener_cmd_thread(void *arg){
  msg *message = NULL;
  char *sender, *rest, *body, *cmd;
  int retval;
  while(1){
    requestNo++;
    freeMsg(message);
    if(LISTENER_DEBUG)
      fprintf(stderr, "%s waiting to process request number %d\n", myName, requestNo);
    message = acceptMsg(STDIN_FILENO);
    if(message == NULL){
      perror("listener readMsg failed");
      continue;
    }
    if(LISTENER_DEBUG)
      fprintf(stderr, "%s processing request:\n\"%s\"\n", myName, message->data);
    retval = parseActorMsg(message->data, &sender, &body);
    if(!retval){
      fprintf(stderr, "didn't understand: (parseActorMsg)\n\t \"%s\" \n", message->data);
      continue;
    }
    if(getNextToken(body, &cmd, &rest) != 1){
      fprintf(stderr, "didn't understand: (cmd)\n\t \"%s\" \n", body);
      continue;
    }
    if(!strcmp(cmd, "close")){
      int slotNumber = -1;
      if(!closed){
        closed = 1;
        closeSocket(listenFd);
        if(LISTENER_DEBUG)
          fprintf(stderr, "%s\n%s\ncloseOK\n", sender, myName);
        sendFormattedMsgFP(stdout, "%s\n%s\ncloseOK\n", sender, myName);
        if(LISTENER_DEBUG)
	  fprintf(stderr, "Listener called %s unregistering\n", myName);
	slotNumber = deleteFromRegistry(myName);
        if(LISTENER_DEBUG)
	  fprintf(stderr, 
		  "Listener called %s removed from slot %d, now exiting\n", 
		  myName, slotNumber);
	exit(EXIT_SUCCESS);
      } else {
        if(DEBUG)
          fprintf(stderr, "%s\n%s\ncloseFailure\n", sender, myName);
        sendFormattedMsgFP(stdout, "%s\n%s\ncloseFailure\n", sender, myName);
      }
    } else {
      fprintf(stderr, "didn't understand: (command)\n\t \"%s\" \n", message->data);
      continue;
    }
  }
}


static void listener_sigchild_handler(int sig){
  /* for the prevention of zombies */
  pid_t child;
  int status;
  child = wait(&status);
  if(LISTENER_DEBUG)
    fprintf(stderr, 
	    "Listener waited on child with pid %d with exit status %d\n", 
	    child, status);
}

static int listener_installHandler(){
  struct sigaction sigactchild;
  sigactchild.sa_handler = listener_sigchild_handler;
  sigactchild.sa_flags = 0;
  sigfillset(&sigactchild.sa_mask);
  return sigaction(SIGCHLD, &sigactchild, NULL);
}

int main(int argc, char** argv){
  char *description, socketName[SIZE], fdName[SIZE];
  int *msgsock;
  pid_t myPid = getpid();
  pthread_t cmdTid;
  if(argv == NULL){
    fprintf(stderr, "didn't understand: (argv == NULL)\n");
    exit(EXIT_FAILURE);
  }
  if(argc != 6){
    fprintf(stderr, "didn't understand: (argc != 6)\n");
    exit(EXIT_FAILURE);
  }

  myName   = argv[0];
  listenFd = atoi(argv[1]);
  myClient = argv[2];
  registry_fifo_in  = argv[3];
  registry_fifo_out = argv[4];
  iop_pid = atoi(argv[5]);

  if(listener_installHandler() != 0){
    perror("Could not install signal handler");
    exit(EXIT_FAILURE);
  }

  if(pthread_create(&cmdTid, NULL, listener_cmd_thread, NULL) != 0){
    fprintf(stderr, "couldn't create listener_cmd_thread\n");
    exit(EXIT_FAILURE);
  };

  while(1){
    if(LISTENER_DEBUG)
      fprintf(stderr, "Blocking on acceptSocket\n");
    msgsock = acceptSocket(listenFd, &description);
    if (*msgsock == INVALID_SOCKET) {
      fprintf(stderr, description);
      free(description);
      continue;
    }
    if(LISTENER_DEBUG)
      printf(description);
    sprintf(socketName, "%s.%d.%d", childName, myPid, connectionNo);
    sprintf(fdName, "%d", *msgsock);
    childArgv[0] = socketName;
    childArgv[1] = fdName;
    childArgv[2] = registry_fifo_in;
    childArgv[3] = registry_fifo_out;
    childArgv[4] = NULL;
    newActor(1, childExe, childArgv);
    if(LISTENER_DEBUG)
      fprintf(stderr, 
              "%s\n%s\nnewConnection\n%s\n", 
              myClient, myName, socketName);
    sendFormattedMsgFP(stdout,
		       "%s\n%s\nnewConnection\n%s\n", 
		       myClient, myName, socketName);
    connectionNo++;
    closeSocket(*msgsock);
    free(description);
    free(msgsock);
  }
  return 0;
}
