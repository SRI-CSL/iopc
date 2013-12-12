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

#include <signal.h>

#include "cheaders.h"
#include "constants.h"
#include "msg.h"
#include "dbugflags.h"
#include "wrapper_lib.h"
#include "iop_lib.h"
#include "ec.h"

extern pid_t child;

void wrapper_sigint_handler(int sig){
  if(child > 0){  kill(child, SIGKILL);   }
  /* calling exit rather than _exit causes a SIGSEGV in RH9 */
  /* don't think it used to?                                */
  _exit(EXIT_FAILURE);
}

int wrapper_installHandler(void (*chld_fun)(int), void (*intr_fun)(int)){
  struct sigaction sigactchild;
  struct sigaction sigactint;
  sigactchild.sa_handler = chld_fun;
  sigactchild.sa_flags = SA_NOCLDSTOP;
  ec_neg1( sigfillset(&sigactchild.sa_mask) );
  ec_neg1( sigaction(SIGCHLD, &sigactchild, NULL) );

  sigactint.sa_handler = intr_fun;
  sigactint.sa_flags = 0;
  ec_neg1( sigfillset(&sigactint.sa_mask) );
  ec_neg1( sigaction(SIGINT, &sigactint, NULL) );

  return 0;

EC_CLEANUP_BGN
  return -1;
EC_CLEANUP_END

}


void parseMaudeThenEcho(int from, int to){
  msg* message;
  int length;
  announce("parseMaudeThenEcho\t:\tCalling readMaudeMsg\n");
  message = readMaudeMsg(from);
  announce("parseMaudeThenEcho\t:\treadMaudeMsg returned %d bytes\n", message->bytesUsed);
  if(message != NULL){
    if(WATCH_MAUDE)logMsg(MAUDE_LOGFILE, message);
    length = parseString(message->data, message->bytesUsed);
    message->bytesUsed = length;
    if(sendMsg(to, message) < 0){
      fprintf(stderr, "sendMsg in parseMaudeThenEcho failed\n");
      return;
    }
    if(MAUDE_WRAPPER_DEBUG)writeMsg(STDERR_FILENO, message);
    announce("\nparseThenEcho wrote %d bytes\n", message->bytesUsed);
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
    if(sendMsg(to, message) < 0){
      fprintf(stderr, "sendMsg in parsePVSThenEcho failed\n");
      return;
    }
    if(WRAPPER_DEBUG)writeMsg(STDERR_FILENO, message);
    announce("\nparseThenEcho wrote %d bytes\n", message->bytesUsed);
    freeMsg(message);
  }
}


static int echoSilently(int from, int to){
  char buff[BUFFSZ];
  int bytesIn = 0, bytesOut = 0;
  bytesIn = read(from, buff, BUFFSZ);
  if(bytesIn <= 0){ return bytesIn; }
  bytesOut = mywrite(to, buff, bytesIn, 0);
  return bytesOut;
}

void *echoErrorsSilently(void *arg){
  fdBundle fdB;
  int errcode, failures = 0;
  sigset_t mask; 
  if(arg == NULL){
    fprintf(stderr, "Bad arg to echoErrorsSilently\n");
    return NULL;
  }
  fdB = *((fdBundle *)arg);

  ec_neg1( sigemptyset(&mask) );
  ec_neg1( sigaddset(&mask, SIGCHLD) );
  ec_rv( pthread_sigmask(SIG_BLOCK, &mask, NULL) );

  while(!(*(fdB.exit))){
    errcode = echoSilently(fdB.fd, STDERR_FILENO);
    if(errcode <= 0){
      if(++failures > 5){ return NULL; }
    } else {
      failures = 0;
    }
  }
  return NULL;
EC_CLEANUP_BGN
  return NULL;
EC_CLEANUP_END
}

void *wrapper_echoOutSilently(void *arg){
  fdBundle fdB;
  if(arg == NULL){
    fprintf(stderr, "Bad arg to echoOutSilently\n");
    return NULL;
  }
  fdB = *((fdBundle *)arg);
  while(!(*(fdB.exit))){
    echoMsgVolatile(fdB.fd, STDOUT_FILENO, fdB.exit);
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
      announce("parseString saw a NULL, wasn't expecting it!\n");
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
