#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include "authenticate.h"
#include "msg.h"


static int exit_flag = 0;

#define AUTH_DEBUG 0

static void alarm_handler(int signum){
  if(signum == SIGALRM){
    if(AUTH_DEBUG){ fprintf(stderr, "authenticate: alarm goes OFF!\n"); }
    exit_flag = 1;
  }
}

/*
 *
 * Current token looks like either:
 *  "PLAClient_online <svn version number>"
 *  "PLAClient_garuda <svn version number>"
 */

int authenticate(int socket, char* itoken, int itokensz){
  int retval = 0;
  msg* token = NULL;
  if(AUTH_DEBUG){ fprintf(stderr, "authenticate: got %d as socket\n", socket); }
  if(socket >= 0){
    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_handler = alarm_handler;
    if(sigaction(SIGALRM, &act, NULL) == -1){
      if(AUTH_DEBUG){ fprintf(stderr, "authenticate: sigaction failed; errno = %d\n", errno); }
      return retval;
    }
    alarm(5);
    if(AUTH_DEBUG){ fprintf(stderr, "authenticate: alarm set\n"); }
    token = acceptMsgVolatile(socket, &exit_flag);
    if(AUTH_DEBUG){ fprintf(stderr, "authenticate: acceptMsgVolatile returns\n"); }
    if(token == NULL){
      if(AUTH_DEBUG){ fprintf(stderr, "authenticate: got NULL token\n"); }
      close(socket);
    } else {
      /* accept anything at this point. desperate for users we are  */
      alarm(0);
      if(itoken != NULL &&  token->bytesUsed <= itokensz){
        strncpy(itoken, token->data, itokensz);
      }
      if(
         (strstr(token->data, "PLAClient_online ") == token->data) ||
         (strstr(token->data, "PLAClient_garuda ") == token->data) 
         ){
        retval = 1;  
      } else {
        retval = 0;  
      }
    }
  }
  if(token != NULL){ freeMsg(token); }
  return retval;
}


