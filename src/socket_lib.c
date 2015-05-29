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
#include "dbugflags.h"


int *acceptSocket(int listenSocket, char **comments){
  int *retval = (int*)calloc(1, sizeof(int));
  char *buff = (char *)calloc(BUFFSZ,sizeof(char));
  char *hostname, *ip = NULL;
  struct sockaddr_in from;
  socklen_t fromlen = sizeof(from);
  struct hostent *hostptr;
  
  if(retval == NULL){
    fprintf(stderr, "calloc of retval failed in acceptSocket\n");
    free(buff);
    return NULL;
  }
  
  if(buff == NULL){
    fprintf(stderr, "calloc of buff failed in acceptSocket\n");
    free(retval);
    return NULL;
  }

 restart:

  *retval = accept(listenSocket, (struct sockaddr*)&from, &fromlen);

  if((*retval == -1) && (errno == EINTR)){  goto restart; }

  if (*retval == INVALID_SOCKET){
    sprintf(buff, "acceptSocket: accept() error %d\n", errno);
    
  } else {

    ip = inet_ntoa(from.sin_addr);
    hostptr = gethostbyaddr((char*)&(from.sin_addr.s_addr), 4, AF_INET);

    hostname = "unknown";
    if(hostptr != NULL){ hostname = hostptr->h_name; }
    snprintf(buff, 
	     BUFFSZ, 
	     "acceptSocket: host = %s, IP = %s\n", hostname, ip);
  }
  *comments = buff;
  return retval;
}

            

int allocateSocket(unsigned short port, char *host, int* sockp){
  int retval = - 1;
  struct sockaddr_in server;
  struct hostent *hp;
  int  conn_socket = -1;
  hp = gethostbyname(host);
  if (hp == NULL) {
    fprintf(stderr,
            "Cannot resolve address [%s]: Error %d\n",
            host,
            h_errno);
    return retval;
  }
  memset(&server, 0, sizeof(server));
  memcpy(&(server.sin_addr), hp->h_addr_list[0], hp->h_length);
  server.sin_family = hp->h_addrtype;
  server.sin_port = htons(port);
  /* Open a socket */
  while((conn_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1){
    if(errno == EINTR){ 
      continue; 
    } else {
      perror("Opening a socket failed in allocateSocket:");
      return retval;
    }
  }
  /* Now connect with it */
  while(connect(conn_socket, (struct sockaddr*)&server, sizeof(server))	== -1){
    if(errno == EINTR){ 
      continue; 
    } else {
      perror("allocateSocket: connect() failed:");
      if(conn_socket >= 0){ close(conn_socket); }
      return retval;
    }
  }
  retval = 1;
  *sockp = conn_socket;
  return retval;
}

int allocateListeningSocket(unsigned short port, int* sockp){
  struct sockaddr_in server;
  int listen_socket = -1;
  int retval = -1;
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY; 
  server.sin_port = htons(port);

  while((listen_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0){
    if(errno == EINTR){ 
      continue; 
    } else {
      announce("socket failed in allocateListeningSocket:");
      return retval;
    }
  }

  if(bind(listen_socket, (struct sockaddr*)&server, sizeof(server)) < 0){
    announce("bind failed in allocateListeningSocket:");
    if(listen_socket >= 0){ close(listen_socket); }
    return retval;
  }
  
  while(listen(listen_socket, MAXBACKLOG) < 0){
    if(errno == EINTR){ 
      continue; 
    } else {
      announce("listen failed in allocateListeningSocket:");
      if(listen_socket >= 0){ close(listen_socket); }
      return retval;
    }
  }
  retval = 1;
  *sockp = listen_socket;
  return retval;
}

void* in2socket(void *sp){
  int socket = *((int *)sp);
  char buff[BUFFSZ];
  int retval;
  while(fgets(buff, BUFFSZ, stdin) != NULL){
    retval = send(socket, buff, strlen(buff), 0);
    if (retval == SOCKET_ERROR) {
      fprintf(stderr, "send to socket failed\n");
      return NULL;
    }
    announce("sent %d chars to socket\n", retval);
  }
  return NULL;
}

void* socket2outGentle(void *sp){
  int socket = *((int *)sp);
  char buff[BUFFSZ];
  int i, retval, gots = 0;
  while(1){
    announce("socket2outGentle commencing recv on %d\n", (int)socket);
    gots++;
    retval = recv(socket, buff, BUFFSZ, 0);
    if(retval == 0){
      fprintf(stderr, "Closed connection\n");
      close(socket);
      return  NULL;
    } 
    else if (retval == SOCKET_ERROR) {
      perror("Closing socket");
      close(socket);
      return  NULL;
    } else {
      announce("sending %d chars to stdout\n", retval);
      for(i = 0; i < retval; i++) putc(buff[i], stdout);
      fprintf(stdout, "\n");
      fflush(stdout);
    }
  };
  return NULL;
}

void* socket2outGentleWithHttpAck(void *sp){
  int socket = *((int *)sp);
  char buff[BUFFSZ];
  int i, retval, gots = 0;
  char ack204[] = "HTTP/1.1 204 OK\r\n\r\n";
  while(1){
    announce("socket2outGentle commencing recv on %d\n", (int)socket);
    gots++;
    retval = recv(socket, buff, BUFFSZ, 0);
    if(retval == 0){
      fprintf(stderr, "Closed connection\n");
      close(socket);
      return  NULL;
    } 
    else if (retval == SOCKET_ERROR) {
      perror("Closing socket");
      close(socket);
      return  NULL;
    } else {
      announce("sending %d chars to stdout\n", retval);
      for(i = 0; i < retval; i++) putc(buff[i], stdout);
      fprintf(stdout, "\n");
      fflush(stdout);
      retval = send(socket, ack204, strlen(ack204), 0);
      if(retval < 0){ perror("send failed\n"); }
      /*      close(socket);    */
    }
  };
  return NULL;
}

void* socket2outViolent(void *sp){
  int socket = *((int *)sp);
  char buff[BUFFSZ];
  int i, retval, gots = 0;
  while((retval = recv(socket, buff, BUFFSZ, 0)) !=  SOCKET_ERROR){
    gots++;
    if(retval == 0){
      fprintf(stderr, "Closed connection\n");
      close(socket);
      exit(EXIT_SUCCESS);
    } 
    else
      for(i = 0; i < retval; i++) putc(buff[i], stdout);
  }
  if (retval == SOCKET_ERROR) {
    close(socket);
    return NULL;
  }
  return NULL;
}




