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
#include "externs.h"

int main(int argc, char** argv){
  pthread_t outThreadhandle;
  pthread_t inThreadhandle;
  unsigned short port;  
  char *host;
  int lsocket;

  if(argc != 3) {
    fprintf(stderr, "Usage: %s host port\n", argv[0]);
    return -1;
  }

  host = argv[1];

  port = atoi(argv[2]);

  if(allocateSocket(port, host, &lsocket) != 1){
    return 0;
  }

  
  if(pthread_create(&outThreadhandle, NULL, in2socket, &lsocket)){
    fprintf(stderr, "Could not spawn in2socket thread\n");
    return -1;
  }
  
  if(pthread_create(&inThreadhandle, NULL, socket2outViolent, &lsocket)){
    fprintf(stderr, "Could not spawn socket2outViolent thread\n");
    return -1;
  }
  
  pthread_join(outThreadhandle, NULL);
  pthread_join(inThreadhandle, NULL);

  return 0;
}
