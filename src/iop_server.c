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
#include "socket_lib.h"
#include "iop_lib.h"
#include "externs.h"
#include "dbugflags.h"

static void iop_server_sigchild_handler(int sig){
  /* for the prevention of zombies */
  pid_t child;
  int status;
  child = wait(&status);
  if(SERVER_DEBUG)
    fprintf(stderr, 
	    "Server waited on child with pid %d with exit status %d\n", 
	    child, status);
}

static int iop_server_installHandler(){
  struct sigaction sigactchild;
  sigactchild.sa_handler = iop_server_sigchild_handler;
  sigactchild.sa_flags = 0;
  sigfillset(&sigactchild.sa_mask);
  return sigaction(SIGCHLD, &sigactchild, NULL);
}

int main(int argc, char *argv[]){
  unsigned short port;
  char *description = NULL;
  char *iop_executable_dir, *maude_executable_dir;
  int listen_socket, *sockp;
  if (argc != 4) {
    fprintf(stderr, "Usage: %s <port> <iop_executable_dir> <maude_executable_dir>\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  port = atoi(argv[1]);
  iop_executable_dir = argv[2];
  maude_executable_dir = argv[3];


  if(iop_server_installHandler() != 0){
    perror("iop_server could not install signal handler");
    exit(EXIT_FAILURE);
  }

  if(allocateListeningSocket(port, &listen_socket) != 1){
    fprintf(stderr, "Couldn't listen on port %d\n", port);
    exit(EXIT_FAILURE);
  }
  if(SERVER_DEBUG)fprintf(stderr,"Listening on port %d\n", port);
  while(1){
    char remoteFd[SIZE];
    char *iop_argv[] = {"iop_main", "-r", NULL, NULL, NULL, NULL};
    description = NULL;
    if(SERVER_DEBUG)fprintf(stderr, "Blocking on acceptSocket\n");
    sockp = acceptSocket(listen_socket, &description);
    if (*sockp == INVALID_SOCKET) {
      fprintf(stderr, description);
      free(description);
      continue;
    }
    if(SERVER_DEBUG)fprintf(stderr, description);
    sprintf(remoteFd, "%d", *sockp);
    iop_argv[2] = remoteFd;
    iop_argv[3] = iop_executable_dir;
    iop_argv[4] = maude_executable_dir;
    /*
      spawn dedicated iop process
    */
    spawnProcess(iop_argv[0], iop_argv);
    close(*sockp);
    free(sockp);
    free(description);
  }
}
