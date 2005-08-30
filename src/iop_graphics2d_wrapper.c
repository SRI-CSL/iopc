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
#include "argv.h"
#include "dbugflags.h"
#include "wrapper_lib.h"
#include "iop_lib.h"
#include "ec.h"
#include "externs.h"

static  int child_died = 0;

static char  g2dexe[] = "java";

static void chld_handler(int sig){
  fprintf(stderr, "%s died! Exiting\n", self);
  child_died = 1;
  sendFormattedMsgFD(STDOUT_FILENO, "system\n%s\nstop %s\n", self, self);
}

int make_arguments(int argcIn, char** argvIn, char*** argvOut){
  int i, cpIndex = -1, argcO;
  char *classpath = NULL;
  char** argvO = NULL;
  for(i = 1; i < argcIn; i++){
    if((strcmp(argvIn[i], "-cp") == 0) || 
       (strcmp(argvIn[i], "-classpath") == 0)){
      cpIndex = i; 
      if(i < argcIn - 1){ 
	classpath = argvIn[i + 1];  
      } else {
	fprintf(stderr, "Bad classpath argument to graphics2d\n");
	exit(EXIT_FAILURE);
      }
    }
  }
  if((cpIndex < 0) || (classpath == NULL)){
    /* no usable classpath passed in */
    argcO = argcIn + 4;
    ec_null( argvO = calloc(argcO, sizeof(char*)) );
    argvO[0] = "java";
    argvO[1] = "-cp";
    if((argvO[2] = iop_alloc_jarpath(argvIn[1], argvIn[0], NULL)) == NULL){
      exit(EXIT_FAILURE);
    }
    for(i = 2; i < argcIn; i++){
      argvO[i + 1] = argvIn[i];
    }
    argvO[argcIn + 1] = "g2d.Main";
    argvO[argcIn + 2] = self;
    argvO[argcIn + 3] = NULL;

  } else {
    argcO = argcIn + 2;
    ec_null( argvO = calloc(argcO, sizeof(char*)) );
    argvO[0] = "java";
    for(i = 1; i < cpIndex; i++){
      argvO[i] = argvIn[i + 1];
    }
    if((argvO[cpIndex] = 
	iop_alloc_jarpath(argvIn[1], argvIn[0], argvIn[cpIndex + 1])) == NULL){
      exit(EXIT_FAILURE);
    }
    for(i = cpIndex + 1; i < argcIn - 1; i++){
      argvO[i] = argvIn[i + 1];
    }
    argvO[argcIn - 1] = "g2d.Main";
    argvO[argcIn] = self;
    argvO[argcIn + 1] = NULL;
  }
  *argvOut = argvO;
  return argcO;

EC_CLEANUP_BGN
  exit(EXIT_FAILURE);
EC_CLEANUP_END
}

int main(int argc, char** argv){
  int pin[2], pout[2], perr[2];
  char **g2dargv;
  int g2dargc;

  if(argc < 2){
    fprintf(stderr, "Usage: %s <iop bin directory> [jvm options]\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  self_debug_flag  = G2D_ACTOR_DEBUG;
  self = argv[0];

  g2dargc = make_arguments(argc, argv, &g2dargv);

  /* printArgv(stderr, g2dargc, g2dargv, "g2dargv"); */

  /*
    ec_null( g2dargv = calloc(argc + 4, sizeof(char*)) );
    g2dargv[0] = "java";
    g2dargv[1] = "-cp";
    if((g2dargv[2] = iop_alloc_jarpath(argv[1], self)) == NULL){
    exit(EXIT_FAILURE);
    }
    for(i = 2; i < argc; i++){
    g2dargv[i + 1] = argv[i];
    }
    g2dargv[argc + 1] = "g2d.Main";
    g2dargv[argc + 2] = self;
    g2dargv[argc + 3] = NULL;
  */

  ec_neg1( wrapper_installHandler(chld_handler, wrapper_sigint_handler) );

  ec_neg1( pipe(pin) );
  ec_neg1( pipe(perr) );
  ec_neg1( pipe(pout) );

  /*it's time to fork */
  ec_neg1( child = fork() );

  if(child == 0){
    /* i'm destined to be the java graphics program */
    ec_neg1( dup2(pin[0],  STDIN_FILENO) );
    ec_neg1( dup2(perr[1], STDERR_FILENO) );
    ec_neg1( dup2(pout[1], STDOUT_FILENO) );

    ec_neg1( close(pin[0])  );
    ec_neg1( close(perr[1]) );
    ec_neg1( close(pout[1]) );

    ec_neg1( execvp(g2dexe, g2dargv) );

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

    ec_neg1( close(pin[0])  );
    ec_neg1( close(perr[1]) );
    ec_neg1( close(pout[1]) );

    ec_rv( pthread_create(&errThread, NULL, echoErrorsSilently, &errFdB) );

    ec_rv( pthread_create(&outThread, NULL, wrapper_echoOutSilently, &outFdB) );

      
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

  exit(EXIT_SUCCESS);

EC_CLEANUP_BGN
  exit(EXIT_FAILURE);
EC_CLEANUP_END

}

