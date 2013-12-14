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
#include "msg.h"
#include "socket_lib.h"
#include "iop_lib.h"
#include "iop_utils.h"
#include "externs.h"
#include "dbugflags.h"
#include "ec.h"

static char logFile[] = "/var/log/iop/netrequest.log"; 

static int  requestNo = 0;

static pthread_mutex_t netrequest_log_mutex = PTHREAD_MUTEX_INITIALIZER;

static pthread_mutex_t netrequest_output_mutex = PTHREAD_MUTEX_INITIALIZER;

static void netlog(const char *format, ...);

static void *netrequest_cmd_thread(void *arg){
  msg *message = NULL;
  char* me = "netrequest_cmd_thread";
  while(1){
    requestNo++;
    freeMsg(message);
    netlog("%s waiting to process request number %d\n", me, requestNo);
    message = acceptMsg(STDIN_FILENO);
    if(message == NULL){
      netlog("%s readMsg failed; %p", me, arg);
      continue;
    }
    netlog("%s processing request:\n\"%s\"\n", me, message->data);
  }
  return NULL;
}


void netlog(const char *format, ...){
#if NETREQUEST_DEBUG == 1
  FILE* logfp = NULL;
  va_list arg;
  va_start(arg, format);
  logfp = fopen(logFile, "aw");
  if(logfp != NULL){
    ec_rv( pthread_mutex_lock(&netrequest_log_mutex) );
    /* if(NETREQUEST_DEBUG)vfprintf(stderr, format, arg);*/
    fprintf(logfp, "\n%s", time2string());
    vfprintf(logfp, format, arg);
    ec_rv( pthread_mutex_unlock(&netrequest_log_mutex) );
    fclose(logfp);
  }
  va_end(arg);
  return;
EC_CLEANUP_BGN
  va_end(arg);
  return;
EC_CLEANUP_END
#endif
}

static int msg2netlog(msg* message){
  if(message != NULL){
    FILE* fp = fopen(logFile,"aw");
    if(fp != NULL){
      fprintf(fp, "%s", time2string());
      writeMsg(fileno(fp), message);
      fclose(fp);
      return 1;
    }
  }
  return 0;
}

static void iop_netrequest_sigchild_handler(int sig){
  fprintf(stderr, "%s died (%d)!\n", self, sig);
  /* for the prevention of zombies */
  pid_t child;
  int status;
  child = wait(&status);
}

void* handleRequest(void* args);
void* handleRequest(void* args){
  int from = -1, to = -1;
  echofds* fds = (echofds*)args;
  if(fds == NULL) goto fail;
  from = fds->from;
  to = fds->to;
  while(1){
    msg* message = acceptMsg(from);
    if(message != NULL){
      msg2netlog(message);
      pthread_mutex_lock(&netrequest_output_mutex);
      sendMsg(to, message);
      pthread_mutex_unlock(&netrequest_output_mutex);
      freeMsg(message);
    } else {
      netlog("handleRequest exiting gracefully!\n");
      break;
    }
  }
  
 fail:
  if(from != -1){ close(from); }
  return NULL;
}





int main(int argc, char *argv[]){
  unsigned short port;
  char *description = NULL;
  int connections = 0, listen_socket, *sockp;
  pthread_t commandThread;
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <port> <self>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  port = atoi(argv[1]);
  self = argv[2];

  if(iop_install_handler(SIGCHLD, 0, iop_netrequest_sigchild_handler) != 0){
    netlog("iop_netrequest could not install signal handler");
    exit(EXIT_FAILURE);
  }

  pthread_create(&commandThread, NULL, netrequest_cmd_thread, NULL);


  if(allocateListeningSocket(port, &listen_socket) != 1){
    fprintf(stderr, "Couldn't listen on port %d\n", port);
    exit(EXIT_FAILURE);
  }
  
  netlog("Netrequest listening on port %d\n", port);
  
  while(1){
    echofds net2Sys;
    pthread_t thrNet2Sys;

    description = NULL;
    netlog("Blocking on acceptSocket (connections = %d)\n", connections);
    sockp = acceptSocket(listen_socket, &description);
    
    connections++;

    netlog("Woken from acceptSocket: %s)\n", description);

    if (*sockp == INVALID_SOCKET) {
      netlog("%s", description);
      free(description);
      continue;
    }

    
    net2Sys.from = *sockp;
    net2Sys.to = STDOUT_FILENO;

    if(pthread_create(&thrNet2Sys, NULL, handleRequest, &net2Sys) != 0){
      netlog("Couldn't create thrR2A thread\n");
    }
    netlog("%s", description);
    free(sockp);
    free(description);
  }
}
