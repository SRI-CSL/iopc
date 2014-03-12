#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include "authenticate.h"
#include "msg.h"


static int exit_flag = 0;

#define AUTH_DEBUG   0

#define MIN_VERSION  2046
#define OK           "true"
#define UPDATE_URL   "http://pl.csl.sri.com/online.html"

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
 * returns the svn version number of the client
 */

int clientOK(msg* token);
int clientOK(msg* token){
  if(token != NULL){
    char* version;
    if(((strstr(token->data, "PLAClient_online ") == token->data) ||
        (strstr(token->data, "PLAClient_garuda ") == token->data)) &&
       ((version = strchr(token->data, ' ')) != NULL)){
      return (int)strtol(version, (char **)NULL, 10);
    }
  }
  return 0;
}

/*
 * returns 1 if the version is ok and the reply was happy
 * returns 0 if the version was too small and the reply was happy
 * returns -1 if the reply was not happy otherwise
 *
 */
int acknowledge(int socket, int version);
int acknowledge(int socket, int version){
  int retval = -1, version_ok;
  if((socket > 0) && (version > 0)){
    msg* reply =  makeMsg(1024);
    int ok;
    version_ok = (version >= MIN_VERSION);
    if(version_ok){
      ok = addToMsg(reply, strlen(OK), OK);
    } else {
      ok = addToMsg(reply, strlen(UPDATE_URL), UPDATE_URL);
    }
    if(ok == 0){
      ok = sendMsg(socket, reply);
      if(ok == reply->bytesUsed){
        retval = (1 && version_ok);
      } else {
        retval = -1;
      }
    }
    freeMsg(reply);
  }
  return retval;
}

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
      /* cancel alarm then check the token  */
      alarm(0);
      if(itoken != NULL &&  token->bytesUsed <= itokensz){
        strncpy(itoken, token->data, itokensz);
      }
      int version = clientOK(token);

      if(version > 0){
        int reply_ok = acknowledge(socket, version);
        if(reply_ok == 1){
          retval = 1;  
        }
      }
    }
  }
  if(token != NULL){ freeMsg(token); }
  return retval;
}


