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
#include "externs.h"

static char pvs_exe[] = "pvs";
static char* pvs_argv[] = {"pvs", "-raw", NULL};
static char* myname = "pvs";
static int pin[2], pout[2], perr[2];

static int child_died = 0;

static void pvs_wrapper_sigint_handler(int sig){
  char pvs_exit[] = "(excl:exit)\n";
  if(child > 0){
    write(pin[1], pvs_exit, strlen(pvs_exit));
  }
  _exit(EXIT_FAILURE);
}

static void pvs_wrapper_sigchild_handler(int sig){
  fprintf(stderr, "PVS died! Exiting\n");
  child_died = 1;
  exit(EXIT_FAILURE);
}

static void pvs_wrapper_installHandler(){
  struct sigaction sigactchild;
  struct sigaction sigactint;
  sigactchild.sa_handler = pvs_wrapper_sigchild_handler;
  sigactchild.sa_flags = SA_NOCLDSTOP;
  sigfillset(&sigactchild.sa_mask);
  sigaction(SIGCHLD, &sigactchild, NULL);
  sigactint.sa_handler = pvs_wrapper_sigint_handler;
  sigactint.sa_flags = 0;
  sigfillset(&sigactint.sa_mask);
  sigaction(SIGINT, &sigactint, NULL);
}


int main(int argc, char** argv){
  if(argc != 1){
    fprintf(stderr, "Usage: %s\n", argv[0]);
  }

  pvs_wrapper_installHandler();

  if((pipe(pin) != 0) || 
     (pipe(perr) != 0) ||
     (pipe(pout) != 0)){
    perror("couldn't make pipes");
    exit(EXIT_FAILURE);
  }
  child = fork();
  if(child < 0){
    perror("couldn't fork");
    exit(EXIT_FAILURE);
  } 
  if(child == 0){
    /* i'm destined to be pvs */
    if((dup2(pin[0],  STDIN_FILENO) < 0)  ||
       (dup2(perr[1], STDERR_FILENO) < 0) ||
       (dup2(pout[1], STDOUT_FILENO) < 0)){
      perror("couldn't dup fd's");
      exit(EXIT_FAILURE);
    }
    if((close(pin[0]) !=  0) ||
       (close(perr[1]) !=  0) ||
       (close(pout[1]) !=  0)){
      perror("couldn't close fd's");
      exit(EXIT_FAILURE);
    }
    execvp(pvs_exe, pvs_argv);
    perror("couldn't execvp");
    exit(EXIT_FAILURE);
  } else {
    /* i'm the boss */
    pthread_t errThread;
    fdBundle  errFdB;

    /* for monitoring the error stream */
    errFdB.fd = perr[0];
    errFdB.exit = &child_died;

    if((close(pin[0]) !=  0) ||
       (close(perr[1]) !=  0) ||
       (close(pout[1]) !=  0)){
      perror("couldn't close fd's");
      exit(EXIT_FAILURE);
    }
    
    /*      sleep(2); */
    
    if(pthread_create(&errThread, NULL, echoErrorsSilently, &errFdB)){
      fprintf(stderr, "Could not spawn echoErrorsSilently thread\n");
      exit(EXIT_FAILURE);
    }
      
    if(WRAPPER_DEBUG)fprintf(stderr, "Listening to PVS\n");
    parsePVSThenEcho("\ncl-user(", pout[0], STDOUT_FILENO);

    write(pin[1], "(in-package 'pvs)\n", strlen("(in-package 'pvs)\n"));
    
    if(WRAPPER_DEBUG)fprintf(stderr, "Listening to PVS\n");
    parsePVSThenEcho("\npvs(", pout[0], STDOUT_FILENO);
    
    while(1){
      int length, retval;
      msg *query = NULL, *response = NULL;
      char *sender, *command;
      
      if(WRAPPER_DEBUG)fprintf(stderr, "Listening to IO\n");
      query = acceptMsg(STDIN_FILENO);
      if(query == NULL) continue;
      retval = parseActorMsg(query->data, &sender, &command);
      
      write(pin[1], command, strlen(command));
      write(pin[1], "\n", 1);
      
      if(WRAPPER_DEBUG)fprintf(stderr, "Listening to PVS\n");
      
      response = readPVSMsg("\npvs(", pout[0]);
      if(response != NULL){
	length = parseString(response->data, response->bytesUsed);
	response->bytesUsed = length;
	
	sendFormattedMsgFP(stdout, "%s\n%s\n%s\n", sender, myname, response->data);
	
	if(WRAPPER_DEBUG){
	  writeMsg(STDERR_FILENO, response);
	  fprintf(stderr, "\nparseThenEcho wrote %d bytes\n", response->bytesUsed);
	}
      }
      
      freeMsg(query);
      freeMsg(response);
      
    }
  }
}


