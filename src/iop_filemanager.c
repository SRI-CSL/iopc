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
#include "actor.h"
#include "msg.h"
#include "iop_lib.h"
#include "externs.h"
#include "dbugflags.h"
#include "argv.h"



static int requestNo = 0;

static mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
int main(int argc, char** argv){
  msg *messageIn = NULL, *messageOut = NULL;
  int byteswritten;
  char *sender, *rest, *body, *cmd, *filename;
  int retval;
  struct flock lock;

  self_debug_flag  = FILEMANAGER_DEBUG;
  self = argv[0];

  while(1){
    requestNo++;
    announce("filemanager at top of loop\n");
    if(messageIn  != NULL){ 
      freeMsg(messageIn);
      messageIn = NULL;
    }  
    if(messageOut != NULL){
      freeMsg(messageOut);
      messageOut = NULL;
    }
    announce("filemanager readMsg-ing\n");
    messageIn = acceptMsg(STDIN_FILENO);
    if(messageIn == NULL){
      perror("filemanager readMsg failed");
      continue;
    }
    announce("filemanager readMsg-ed\n");
    retval = parseActorMsg(messageIn->data, &sender, &body);
    if(!retval){
      fprintf(stderr, "didn't understand: (parseActorMsg)\n\t \"%s\" \n", messageIn->data);
      continue;
    }
    announce("filemanager parseActorMsg-ed\n");
    if(getNextToken(body, &cmd, &rest) != 1){
      fprintf(stderr, "didn't understand: (cmd)\n\t \"%s\" \n", body);
      continue;
    }
    announce("filemanager getNextToken-ed of cmd\n");
    if(getNextToken(rest, &filename, &rest) != 1){
      fprintf(stderr, "didn't understand: (filename)\n\t \"%s\" \n", rest);
      continue;
    }
    announce("filemanager getNextToken-ed of filename\n");
    if(!strcmp(cmd, "read")){
      char *newfilename = NULL;
      char *oldfilename = filename;
      int fd, tilde;
      announce("filemanager entering a read command\n");
      tilde = interpretTildes(filename, &newfilename);
      announce("filemanager interpretTildes-ed\n");
      if(tilde) filename = newfilename;
      fd = open(filename, O_RDONLY, 0);
      if(fd < 0){
        fprintf(stderr, "file = \"%s\"\n", filename);
        perror("couldn't open file for reading");
	goto rfail;
      }
      announce("filemanager file opened\n");

      lockFD(&lock, fd, filename);
      announce("filemanager file locked\n");

      if((messageOut = readMsg(fd)) == NULL){
        perror("filemanager read from file failed");
	close(fd);
	goto urfail;
      }
      if(FILEMANAGER_DEBUG)fprintf(stderr,"%s\n%s\ncontents %s\n%s\n", 
				   sender, self, oldfilename, messageOut->data);
      sendFormattedMsgFP(stdout, "%s\n%s\ncontents %s\n%s\n", 
			 sender, self, oldfilename, messageOut->data);
      close(fd);
      free(newfilename);
      continue;
      
    urfail:
      unlockFD(&lock, fd, filename);
      
    rfail:
      announce("%s\n%s\nreadFailure\n%s\n", sender, self, oldfilename);
      sendFormattedMsgFP(stdout, "%s\n%s\nreadFailure\n%s\n", sender, self, oldfilename);
      free(newfilename);
      continue;
      
    } else if(!strcmp(cmd, "write")){
      char *newfilename = NULL;
      char *oldfilename = filename;
      int fd, tilde;
      announce("filemanager entering write command\n");
      tilde = interpretTildes(filename, &newfilename);
      if(tilde) filename = newfilename;
      announce("filemanager interpretTildes-ed\n");
      fd = open(filename, O_WRONLY|O_CREAT|O_TRUNC, mode); 
      if(fd < 0){
        fprintf(stderr, "file = \"%s\"\n", filename);
        perror("couldn't open file for writing");
	goto wfail;
      }
      announce("filemanager opened file\n");
     
      lockFD(&lock, fd, filename);
      announce("filemanager locked file\n");
      
      while(isspace(*rest))rest++;

      if((byteswritten = write(fd, rest, strlen(rest))) < 0){
        perror("filemanager write to file failed");
	close(fd);
	goto uwfail;
      }

      announce("%s\n%s\nwriteOK\n%s\n", sender, self, oldfilename);
      sendFormattedMsgFP(stdout, "%s\n%s\nwriteOK\n%s\n", sender, self, oldfilename);
      close(fd);
      free(newfilename);
      continue;

    uwfail:
      unlockFD(&lock, fd, filename);

    wfail:
      announce("%s\n%s\nwriteFailure\n%s\n", sender, self, oldfilename);
      sendFormattedMsgFP(stdout, "%s\n%s\nwriteFailure\n%s\n", sender, self, oldfilename);
      free(newfilename);
      continue;

    } else if(!strcmp(cmd, "append")){
      char *newfilename = NULL;
      char *oldfilename = filename;
      int fd, tilde;
      announce("filemanager entering a append clause\n");
      tilde = interpretTildes(filename, &newfilename);
      announce("filemanager interpretTildes-ed\n");
      if(tilde) filename = newfilename;
      fd = open(filename, O_WRONLY|O_CREAT|O_APPEND, mode); 
      if(fd < 0){
        fprintf(stderr, "file = \"%s\"\n", filename);
        perror("couldn't open file for writing");
	goto afail;
      }

      announce("filemanager opened file\n");

      lockFD(&lock, fd, filename);

      announce("filemanager locked file\n");

      if((byteswritten = write(fd, rest, strlen(rest))) < 0){
        perror("filemanager append to file failed");
	close(fd);
	goto uafail;
      }
      announce("%s\n%s\nappendOK\n%s\n", sender, self, oldfilename);
      sendFormattedMsgFP(stdout, "%s\n%s\nappendOK\n%s\n", sender, self, oldfilename);
      close(fd);
      free(newfilename);
      continue;

    uafail:
      unlockFD(&lock, fd, filename);

    afail:
      announce("%s\n%s\nappendFailure\n%s\n", sender, self, oldfilename);
      sendFormattedMsgFP(stdout, "%s\n%s\nappendFailure\n%s\n", sender, self, oldfilename);
      free(newfilename);
    } else {
      fprintf(stderr, "didn't understand: (command)\n\t \"%s\" \n", messageIn->data);
    }
    continue;
  }
}

