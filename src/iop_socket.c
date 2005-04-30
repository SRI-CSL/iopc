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
#include "ec.h"

/* externs used in the announce routine */
int   local_debug_flag  = SOCKET_DEBUG;
char* local_process_name;

static int requestNo = 0;
static char* myname;
static int sock = -1;
static int closed = 0;

static void socket_sigpipe_handler(int sig){
  announce("Socket %s got a signal %d\n", myname, sig);	
  deleteFromRegistry(myname);
  if(sock > 0){ (void)close(sock); }
  exit(EXIT_FAILURE);
}

static int socket_installHandler(){
  struct sigaction sigactpipe;
  sigactpipe.sa_handler = socket_sigpipe_handler;
  sigactpipe.sa_flags = 0;
  ec_neg1( sigemptyset(&sigactpipe.sa_mask) );
  ec_neg1( sigaction(SIGPIPE, &sigactpipe, NULL) );
  return 0;
EC_CLEANUP_BGN
  return -1;
EC_CLEANUP_END
}

int main(int argc, char** argv){
  msg *messageIn = NULL, *messageOut = NULL;
  char *sender, *rest, *body, *cmd;
  int retval;
  if(argv == NULL){
    fprintf(stderr, "didn't understand: (argv == NULL)\n");
    exit(EXIT_FAILURE);
  }
  if(argc != 4){
    fprintf(stderr, "didn't understand: (argc != 4)\n");
    exit(EXIT_FAILURE);
  }

  local_process_name = myname = argv[0];
  sock = atoi(argv[1]);
  registry_fifo_in  = argv[2];
  registry_fifo_out = argv[3];

  if(socket_installHandler() != 0){
    perror("socket couldn't install handler\n");
    exit(EXIT_FAILURE);
  }

  while(1){
    requestNo++;
    if(messageIn  != NULL){ 
      freeMsg(messageIn);
      messageIn = NULL;
    }  
    if(messageOut != NULL){
      freeMsg(messageOut);
      messageOut = NULL;
    }
    announce("%s waiting to process request number %d\n", myname, requestNo);
    messageIn = acceptMsg(STDIN_FILENO);
    if(messageIn == NULL){
      perror("socket acceptMsg failed");
      continue;
    }
    announce("%s processing request:\n\"%s\"\n", myname, messageIn->data);
    retval = parseActorMsg(messageIn->data, &sender, &body);
    if(!retval){
      fprintf(stderr, "didn't understand: (parseActorMsg)\n\t \"%s\" \n", messageIn->data);
      continue;
    }
    if(getNextToken(body, &cmd, &rest) != 1){
      fprintf(stderr, "didn't understand: (cmd)\n\t \"%s\" \n", body);
      continue;
    }
    if(!strcmp(cmd, "read")){
      if(closed){
        goto readfail;
      } else {
        char *bytes2read;
        int bytesDesired;
        if(getNextToken(rest, &bytes2read, &rest) != 1){
          fprintf(stderr, 
                  "didn't understand: (bytes2read)\n\t \"%s\" \n", 
                  rest);
          continue;
        }
        bytesDesired = atoi(bytes2read);
        if(bytesDesired == 0){
          goto readfail;
        } else {
	  messageOut = readMsg(sock);
	  if(messageOut == NULL){
	    perror("socket readMsg failed");
	    goto readfail;
	  }
	  announce("%s\n%s\nreadOK\n%d\n%s\n", 
		   sender, 
		   myname,
		   messageOut->bytesUsed,
		   messageOut->data);
	  sendFormattedMsgFP(stdout, 
			     "%s\n%s\nreadOK\n%d\n%s\n", 
			     sender, 
			     myname,
			     messageOut->bytesUsed,
			     messageOut->data);
	}
      
	continue;
      
      readfail:
	announce("%s\n%s\nreadFailure\n", sender, myname);
	sendFormattedMsgFP(stdout, "%s\n%s\nreadFailure\n", sender, myname);
	continue;

      }
        
    } else if(!strcmp(cmd, "write")){
      if(closed){
	fprintf(stderr, "closed\n");
        goto writefail;
      } else {
        char *bytes2write;
        int bytesToSend, bytesSent;
        if(getNextToken(rest, &bytes2write, &rest) != 1){
          fprintf(stderr, 
                  "didn't understand: (bytes2write)\n\t \"%s\" \n", 
                  rest);
          continue;
        }
        bytesToSend = atoi(bytes2write);
        if(bytesToSend == 0){
	  fprintf(stderr, "(bytesToSend == 0)\n");
	  goto writefail;
        } else {
	  int len = strlen(rest);
	  bytesToSend = (len < bytesToSend ) ? len : bytesToSend;
          if((bytesSent = write(sock, rest, bytesToSend)) != bytesToSend){
	    fprintf(stderr, "(bytesSent = write(sock, rest, bytesToSend)) != bytesToSend\n");
            goto writefail;
          } else {
            announce("%s\n%s\nwriteOK\n%d\n", 
		     sender, 
		     myname,
		     bytesSent);
            sendFormattedMsgFP(stdout, 
			       "%s\n%s\nwriteOK\n%d\n", 
			       sender, 
			       myname,
			       bytesSent);
          }
        }
      }
      
      continue;
      
    writefail:
      announce("%s\n%s\nwriteFailure\n", sender, myname);
      sendFormattedMsgFP(stdout, "%s\n%s\nwriteFailure\n", sender, myname);
    } else if(!strcmp(cmd, "close")){
      int slotNumber = -1;
      if(!closed){
        closed = 1;
	if(close(sock) < 0){
	  fprintf(stderr, "close failed in socket close case\n");
	}
        announce("%s\n%s\ncloseOK\n", sender, myname);
        sendFormattedMsgFP(stdout, "%s\n%s\ncloseOK\n", sender, myname);
	usleep(1);
        announce("Socket called %s unregistering\n", myname);
	slotNumber = deleteFromRegistry(myname);
        announce("Socket called %s removed from slot %d, now exiting\n", 
		 myname, slotNumber);
	exit(EXIT_SUCCESS);
      } else {
        announce("%s\n%s\ncloseFailure\n", sender, myname);
        sendFormattedMsgFP(stdout, "%s\n%s\ncloseFailure\n", sender, myname);
      }
    } else {
      fprintf(stderr, "didn't understand: (command)\n\t \"%s\" \n", messageIn->data);
      continue;
    }
  }
}
