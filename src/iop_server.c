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
#include "ec.h"

static char logFile[]   = "/tmp/iop_server_log.txt";
static char ouputFile[] = "/tmp/iop_server_output.txt";

static pthread_mutex_t server_log_mutex = PTHREAD_MUTEX_INITIALIZER;


static char* time2string(){
  time_t t;
  char *date;
  time(&t);
  date = ctime(&t);
  if(date != NULL){
    char* cr = strchr(date, '\n');
    if(cr != NULL){ cr[0] = '\t';  }
  }
  return date;
}

void serverLog(const char *format, ...){
  FILE* logfp = NULL;
  va_list arg;
  va_start(arg, format);
  logfp = fopen(logFile, "aw");
  if(logfp != NULL){
    ec_rv( pthread_mutex_lock(&server_log_mutex) );
    if(SERVER_DEBUG)vfprintf(stderr, format, arg);
    fprintf(logfp, "%s", time2string());
    vfprintf(logfp, format, arg);
    ec_rv( pthread_mutex_unlock(&server_log_mutex) );
    fclose(logfp);
  }
  va_end(arg);
  return;
EC_CLEANUP_BGN
  va_end(arg);
  return;
EC_CLEANUP_END
}


static void iop_server_sigchild_handler(int sig){
  /* for the prevention of zombies and logging statistics */
  pid_t child;
  int status;
  child = wait(&status);
  serverLog("Server waited on child with pid %d with exit status %d\n", 
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
  int listen_socket, *sockp, no_windows;
  int outfd;
  pid_t sid, pid;
  if (argc != 5) {
    fprintf(stderr, "Usage: %s <port> <iop exe dir> <maude exe dir> <no windows>\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  port = atoi(argv[1]);
  iop_executable_dir = argv[2];
  maude_executable_dir = argv[3];
  no_windows = atoi(argv[4]);
  
  /* we want to be a daemon, so lets do that first */

  /* we have already been forked by iop_main (hopefully) so we start with: */
  /* detaching ourselves from the controlling tty                          */

  if((sid = setsid()) < 0){
    perror("iop_server could not create a new session id");
    exit(EXIT_FAILURE);
  }

  if((pid = fork()) < 0){
    perror("iop_server could not fork");
    exit(EXIT_FAILURE);
  }
  
  if(pid > 0){
    /* only the child should continue */
    exit(EXIT_SUCCESS);
  }
  


  outfd = open(ouputFile, O_CREAT|O_TRUNC|O_RDWR, S_IRWXU);
  if(outfd < 0){
    perror("iop_server could not open output file");
    exit(EXIT_FAILURE);
  }

  /* hopefully we haven't inherited many more open file descriptors than these three. */
  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);


  if((dup2(outfd, STDOUT_FILENO) < 0) || (dup2(outfd, STDERR_FILENO) < 0)){
    perror("iop_server could not dup2 (who knows where this error msg goes?)");
    exit(EXIT_FAILURE);
  }

  /* N.B. all errors should now go to ouputFile */
  
  if(iop_server_installHandler() != 0){
    perror("iop_server could not install signal handler");
    exit(EXIT_FAILURE);
  }

  if(allocateListeningSocket(port, &listen_socket) != 1){
    fprintf(stderr, "Couldn't listen on port %d\n", port);
    exit(EXIT_FAILURE);
  }
  
  serverLog("Server listening on port %d\n", port);
  
  while(1){
    pid_t child;
    char remoteFd[SIZE];
    char *iop_argv[] = {"iop_main", "-r", NULL, NULL, NULL, NULL, NULL};
    description = NULL;
    serverLog("Blocking on acceptSocket\n");
    sockp = acceptSocket(listen_socket, &description);
    if (*sockp == INVALID_SOCKET) {
      serverLog("%s", description);
      free(description);
      continue;
    }
    serverLog("%s", description);
    sprintf(remoteFd, "%d", *sockp);
    iop_argv[2] = remoteFd;
    if(no_windows){
      iop_argv[3] = "-n";
      iop_argv[4] = iop_executable_dir;
      iop_argv[5] = maude_executable_dir;
    } else {
      iop_argv[3] = iop_executable_dir;
      iop_argv[4] = maude_executable_dir;
    }

    /* spawn dedicated iop process
       if(SERVER_DEBUG)serverLog("Spawning [%s %s %s %s %s %s]\n", 
			          iop_argv[0],
			          iop_argv[1],
			          iop_argv[2],
			          iop_argv[3],
			          iop_argv[4], 
			          iop_argv[5]);

    */

    child = spawnProcess(iop_argv[0], iop_argv);
    close(*sockp);
    free(sockp);
    free(description);
    if(child > 0){
      serverLog("Spawned iop process %d\n", child);
    }
  }
}
