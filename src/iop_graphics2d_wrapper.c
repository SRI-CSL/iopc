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

static char* myName;
static char  graphics_exe[] = "java";
static char* graphics_argv[] = {"java", "-cp", NULL, "g2d.Main", NULL, NULL};

static int child_died = 0;

static void graphics_wrapper_sigchild_handler(int sig){
  fprintf(stderr, "%s died! Exiting\n", graphics_argv[3]);
  child_died = 1;
  sendFormattedMsgFD(STDOUT_FILENO, "system\n%s\nstop %s\n", myName, myName);
}

static void graphics_wrapper_installHandler(){
  struct sigaction sigactchild;
  struct sigaction sigactint;
  sigactchild.sa_handler = graphics_wrapper_sigchild_handler;
  sigactchild.sa_flags = SA_NOCLDSTOP;
  sigfillset(&sigactchild.sa_mask);
  sigaction(SIGCHLD, &sigactchild, NULL);
  sigactint.sa_handler = wrapper_sigint_handler;
  sigactint.sa_flags = 0;
  sigfillset(&sigactint.sa_mask);
  sigaction(SIGINT, &sigactint, NULL);
}

int main(int argc, char** argv){
  int pin[2], pout[2], perr[2];
  if((argc != 2)){
    fprintf(stderr, "Usage: %s <iop bin directory>\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  myName = argv[0];

  if((graphics_argv[2] = iop_alloc_jarpath(argv[1], myName)) == NULL){
    exit(EXIT_FAILURE);
  }

  graphics_argv[4] = myName;
  graphics_wrapper_installHandler();

  if((pipe(pin)  != 0) || 
     (pipe(perr) != 0) ||
     (pipe(pout) != 0)){
    perror("couldn't make pipes");
    exit(EXIT_FAILURE);
  }
  child = fork();
  if(child < 0){
    perror("couldn't fork");
    exit(EXIT_FAILURE);
  } else if(child == 0){
      /* i'm destined to be the java graphics program */
    if((dup2(pin[0],  STDIN_FILENO) < 0)  ||
       (dup2(perr[1], STDERR_FILENO) < 0) ||
       (dup2(pout[1], STDOUT_FILENO) < 0)){
      perror("couldn't dup fd's");
      exit(EXIT_FAILURE);
    }
    if((close(pin[0])  !=  0) ||
       (close(perr[1]) !=  0) ||
       (close(pout[1]) !=  0)){
      perror("couldn't close fd's");
      exit(EXIT_FAILURE);
    } 
    execvp(graphics_exe, graphics_argv);
    perror("couldn't execvp");
    exit(EXIT_FAILURE);
    /* end of child code */
  } else {
    /* i'm the boss */
    pthread_t errThread, outThread;
    msg* message = NULL;
    int requestNo = 0;
    fdBundle  errFdB, outFdB;

    /* for monitoring the error stream */
    errFdB.fd = perr[0];
    errFdB.exit = &child_died;

    /* for monitoring the output stream */
    outFdB.fd = pout[0];
    outFdB.exit = &child_died;

    if((close(pin[0])  !=  0) ||
       (close(perr[1]) !=  0) ||
       (close(pout[1]) !=  0)){
      perror("couldn't close fd's");
      exit(EXIT_FAILURE);
    }

    if(pthread_create(&errThread, NULL, echoErrorsSilently, &errFdB)){
      fprintf(stderr, "Could not spawn echoErrorsSilently thread\n");
      exit(EXIT_FAILURE);
    }

    if(pthread_create(&outThread, NULL, wrapper_echoOutSilently, &outFdB)){
      fprintf(stderr, "Could not spawn wrapper_echoOut thread\n");
      exit(EXIT_FAILURE);
    }
      
    while(1){
      int size;
      requestNo++;
      freeMsg(message);
      message = acceptMsg(STDIN_FILENO);
      if(message == NULL){
	perror("graphics readMsg failed");
	continue;
      }
      size = message->bytesUsed;
      sendMsg(pin[1], message);
    }
    /* end of boss code */
  }
}

