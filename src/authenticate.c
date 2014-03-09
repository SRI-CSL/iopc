#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include "authenticate.h"
#include "msg.h"


static int exit_flag = 0;

static void alarm_handler(int signum){
  if(signum == SIGALRM){
    exit_flag = 1;
  }
}


int authenticate(int socket){
  int retval = 0;
  msg* token = NULL;
  if(socket >= 0){
    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_handler = alarm_handler;
    if(sigaction(SIGALRM, &act, NULL) == -1){
      fprintf(stderr, "authenticate: sigaction failed; errno = %d\n", errno);
      return retval;
    }
    alarm(5);
    token = acceptMsgVolatile(socket, &exit_flag);
    if(token == NULL){
      fprintf(stderr, "authenticate: got NULL token");
      close(socket);
    } else {
      writeMsg(STDERR_FILENO, token);
      alarm(0);
      retval = 1;  
    }
  }
  if(token != NULL){ freeMsg(token); }
  return retval;
}


