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

#ifndef _IOPTYPES
#define _IOPTYPES

#include <sys/types.h>

#ifndef PATH_MAX
#define PATH_MAX 255
#endif

typedef struct _actor_spec {
  char  name[PATH_MAX];      /* name should be globally unique                               */
  pid_t  pid;                /* the pid of the actor or 0 if just a name placeholder         */
  char fifos[3][PATH_MAX];   /* fifos corresponding to IN OUT & ERR                          */
} actor_spec;


typedef struct _actor_id {
  actor_spec *spec;
  int  fds[3];
  volatile int  exitFlag;
  pthread_t tids[2];
  pthread_mutex_t mutex;
} actor_id;

/* must be kept in synch with RegCmds.java */
typedef enum registry_cmds { SEND = 0,  KILL, REGISTER, UNREGISTER, DUMP, NAME, RSIZE } registry_cmd_t;

/* must be kept in synch with RegCmds.java */
typedef enum output_cmds { ERROR = 7,  OUTPUT = 8, UPDATE =  9, SELECT = 10} output_cmd_t;

typedef struct _msg {
  int bytesUsed;
  int bytesLeft;
  char* data;
} msg;

typedef struct _echofds {
  int from;
  int to;
} echofds;



#endif
