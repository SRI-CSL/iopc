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
#include "iop_utils.h"
#include "externs.h"
#include "dbugflags.h"
#include "ec.h"

static char logFile[]  =      "/var/log/iop/daemon.log";
static char outputFile[]  =   "/var/log/iop/output.log";


static pthread_mutex_t daemon_log_mutex = PTHREAD_MUTEX_INITIALIZER;


static void daemonLog(const char *format, ...){
  FILE* logfp = NULL;
  va_list arg;
  va_start(arg, format);
  logfp = fopen(logFile, "aw");
  if(logfp != NULL){
    ec_rv( pthread_mutex_lock(&daemon_log_mutex) );
    if(DAEMON_DEBUG)vfprintf(stderr, format, arg);
    fprintf(logfp, "%s", time2string());
    vfprintf(logfp, format, arg);
    ec_rv( pthread_mutex_unlock(&daemon_log_mutex) );
    fflush(logfp);
    fclose(logfp);
  }
  va_end(arg);
  return;
EC_CLEANUP_BGN
  if(logfp != NULL){ fclose(logfp); }
  va_end(arg);
  return;
EC_CLEANUP_END
}


static void iop_daemon_sigchild_handler(int sig){
  /* for the prevention of zombies and logging statistics */
  pid_t child;
  int status;
  child = wait(&status);
  daemonLog("Daemon waited(%d) on child with pid %d with exit status %d\n", sig, child, status);
}



static int iop_daemon_io_config(void){
  int retval = 1;
  int outfd = open(outputFile, O_CREAT|O_RDWR|O_APPEND, S_IRWXU);
  if(outfd < 0){
    return retval;
  }
  close(STDOUT_FILENO);
  close(STDERR_FILENO);
  if((dup2(outfd, STDOUT_FILENO) < 0) || (dup2(outfd, STDERR_FILENO) < 0)){
    close(outfd);
    retval = 2 ;
  } else {
    retval = 0;
  }
  close(outfd);
  return retval;
}


int main(int argc, char *argv[]){
  char *iop_executable_dir, *maude_executable_dir;
  int logok;
  pid_t sid, pid;
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <iop exe dir> <maude exe dir>\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  iop_executable_dir = argv[1];
  maude_executable_dir = argv[2];
  
  /* we want to be a daemon, so lets do that first */

  /* N.B. all errors should now go to outputFile */
  if(iop_install_handler(SIGCHLD, 0, iop_daemon_sigchild_handler) != 0){
    fprintf(stderr, "iop_daemon could not install signal handler");
    exit(EXIT_FAILURE);
  }
  
  /* we have already been forked by iop_main (hopefully) so we start with: */
  /* detaching ourselves from the controlling tty                          */

  if((sid = setsid()) < 0){
    perror("iop_daemon could not create a new session id");
    exit(EXIT_FAILURE);
  }

  if((pid = fork()) < 0){
    perror("iop_daemon could not fork");
    exit(EXIT_FAILURE);
  }
  
  if(pid > 0){
    /* only the child should continue */
    exit(EXIT_SUCCESS);
  }
  
  logok = iop_daemon_io_config();

  if(logok != 0){
    daemonLog("iop_daemon_io_config failed (wasting my breath...)");
    exit(EXIT_FAILURE);
  }


  daemonLog("Daemon preparing to morph into iop\n");
  
  { 
    char *iop_argv[] = {"iop_main", "-n", NULL, NULL, NULL};
    iop_argv[2] = iop_executable_dir;
    iop_argv[3] = maude_executable_dir;

    execvp(iop_argv[0], iop_argv);
  }
  exit(EXIT_SUCCESS);
}
