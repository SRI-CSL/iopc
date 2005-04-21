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


extern pid_t child;

void wrapper_sigint_handler(int sig){
  if(child > 0){
    kill(child, SIGKILL);
  }
  /* calling exit rather than _exit causes a SIGSEGV in RH9 */
  /* don't think it used to?                                */
  _exit(EXIT_FAILURE);
}

static void wrapper_sigchild_handler(int sig){
  fprintf(stderr, "wrapper's child died! Exiting\n");
  exit(EXIT_FAILURE);
}

void wrapper_installHandler(){
  struct sigaction sigactchild;
  struct sigaction sigactint;
  sigactchild.sa_handler = wrapper_sigchild_handler;
  sigactchild.sa_flags = 0;
  sigfillset(&sigactchild.sa_mask);
  sigaction(SIGCHLD, &sigactchild, NULL);
  sigactint.sa_handler = wrapper_sigint_handler;
  sigactint.sa_flags = 0;
  sigfillset(&sigactint.sa_mask);
  sigaction(SIGINT, &sigactint, NULL);
}

void parseMaudeThenEcho(int from, int to){
  msg* message;
  int length;
  if(MAUDE_WRAPPER_DEBUG)
    fprintf(stderr, "parseMaudeThenEcho\t:\tCalling readMaudeMsg\n");
  message = readMaudeMsg(from);
  if(MAUDE_WRAPPER_DEBUG)
    fprintf(stderr, "parseMaudeThenEcho\t:\treadMaudeMsg returned %d bytes\n", message->bytesUsed);
  if(message != NULL){
    length = parseString(message->data, message->bytesUsed);
    message->bytesUsed = length;
    sendMsg(to, message);
    if(MAUDE_WRAPPER_DEBUG){
      writeMsg(STDERR_FILENO, message);
      fprintf(stderr, "\nparseThenEcho wrote %d bytes\n", message->bytesUsed);
    }
    freeMsg(message);
  }
}

void parsePVSThenEcho(char *prompt, int from, int to){
  msg* message;
  int length;
  message = readPVSMsg(prompt, from);
  if(message != NULL){
    length = parseString(message->data, message->bytesUsed);
    message->bytesUsed = length;
    sendMsg(to, message);
    if(WRAPPER_DEBUG){
      writeMsg(STDERR_FILENO, message);
      fprintf(stderr, "\nparseThenEcho wrote %d bytes\n", message->bytesUsed);
    }
    freeMsg(message);
  }
}


void *echoErrors(void *arg){
  int fd;
  sigset_t mask;
  if(arg == NULL){
    fprintf(stderr, "Bad arg to echoErrors\n");
    return NULL;
  }
  fd = *((int *)arg);
  if((sigemptyset(&mask) != 0) && (sigaddset(&mask, SIGCHLD) != 0)){
    fprintf(stderr, "futzing with sigsets failed in echoErrors\n");
  }
  if(pthread_sigmask(SIG_BLOCK, &mask, NULL) != 0){
    fprintf(stderr, "pthread_sigmask failed in echoErrors\n");
  }
  while(1)
    echo(fd, STDERR_FILENO);
  return NULL;
}


void *wrapper_echoOut(void *arg){
  int fd;
  if(arg == NULL){
    fprintf(stderr, "Bad arg to echoOut\n");
    return NULL;
  }
  fd = *((int *)arg);
  while(1){
    echoMsg(fd, STDOUT_FILENO);
  }
  return NULL;
}

int parseString(char* inBytes, int bytesIn){
  char *pin = inBytes, *pout = inBytes;
  int retval = 0, insideString = 0;
  if((inBytes == NULL) || (bytesIn == 0)) return retval;
  while(bytesIn > 0){
    if(*pin == '\0'){
      *pout = '\n';
      retval++;
      if(WRAPPER_DEBUG)
	fprintf(stderr, "parseString saw a NULL, wasn't expecting it!\n");
      return retval;
    }
    if(*pin == '"'){
      if(insideString){
	insideString  = 0;
      } else {
	insideString  = 1;
      }
      *pout = ' ';
      pin++;
      pout++;
      bytesIn--;
      retval++;
    } else if(insideString && (*pin == '\\') && (*(pin + 1) == 'n')){
      *pout = '\n';
      pout++;
      retval++;
      pin += 2;
      bytesIn -= 2;
    } else if(insideString && (*pin == '\\') && (*(pin + 1) == 'r')){
      *pout = '\r';
      pout++;
      retval++;
      pin += 2;
      bytesIn -= 2;
    } else if(insideString && (*pin == '\\') && (*(pin + 1) == 't')){
      *pout = '\t';
      pout++;
      retval++;
      pin += 2;
      bytesIn -= 2;
    }  else if(insideString && (*pin == '\\') && (*(pin + 1) == 's')){
      *pout = ' ';
      pout++;
      retval++;
      pin += 2;
      bytesIn -= 2;
    }  else if(insideString && (*pin == '\\') && (*(pin + 1) == '\\')){
      *pout = '\\';
      pout++;
      retval++;
      pin += 2;
      bytesIn -= 2;
    }  else if(insideString && (*pin == '\\') && (*(pin + 1) == '"')){
      *pout = '"';
      pout++;
      retval++;
      pin += 2;
      bytesIn -= 2;
    } else {
      *pout = *pin;
      pin++;
      pout++;
      bytesIn--;
      retval++;
    }
  }
  return retval;
}
