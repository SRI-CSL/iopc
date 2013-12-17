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

#ifndef _IOP_MSG_H
#define _IOP_MSG_H

#include "types.h"

int mywrite(int fd, char *buff, int count, int verbose);


msg* makeMsg(int bytes);
int addToMsg(msg* m, int bytes, char* buff);

void freeMsg(msg* m);
msg* readMaudeMsg(int fd);
msg* readPVSMsg(char* prompt, int fd);
msg* readMsg(int fd);
int writeMsg(int fd, msg* m);
int logMsg(char* filename, msg* m);

int writeInt(int fd, int number);
int readInt(int fd, int* nump, const char* caller);
int readIntVolatile(int fd, int* nump, volatile int* exitFlag);
int writeActorSpec(int fd, actor_spec *act);
actor_spec *readActorSpec(int fd);
char* readline(int fd);
msg* readMsgVolatile(int fd, volatile int* exitFlag);

/* new forms that send/recieve the length first */
msg* acceptMsg(int fd);
msg* acceptMsgVolatile(int fd, volatile int* exitFlag);
int  sendMsg(int fd, msg*);
int sendFormattedMsgFP(FILE*, char* fmt, ...);
int sendFormattedMsgFD(int, char* fmt, ...);
void echoMsgVolatile(int from, int to,  volatile int* exitFlag);
void echo2Maude(int from, int to);
void echo2PVS(int from, int to);
void* echoLoopDieOnFail(void* args);
void* echoLoop(void* args);
void wait4IO(int fdout, int fderr,void (*fp)(int ,int ));
void echo2Input(int from, int to);

#endif /* _IOP_MSG_H */
