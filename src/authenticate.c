#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "authenticate.h"
#include "msg.h"


void* timer(void* ptr);
void* timer(void* ptr){
  int socket = *(int*)ptr;
  sleep(6);
  close(socket);
  exit(EXIT_FAILURE);
  return NULL;
}

int authenticate(int socket){
  int retval = 0;
  msg* token = NULL;
  if(socket >= 0){
    pthread_t timer_thread;

    pthread_create (&timer_thread, NULL, &timer, (void *) &socket);

    msg* token = acceptMsg(socket);

    if(token == NULL){
      fprintf(stderr, "authenticate: got NULL token");
    } else {
      writeMsg(STDERR_FILENO, token);
      retval = 1;  
    }
  }
  if(token != NULL){ freeMsg(token); }
  close(socket);
  return retval;
}


