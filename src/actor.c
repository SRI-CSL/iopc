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
#include "actor.h"
#include "msg.h"
#include "dbugflags.h"

static mode_t mode[3] = { S_IRWXU,  S_IRWXU, S_IRWXU };

extern int iop_debug_flag;
extern int iop_no_windows_flag;


extern char* registry_fifo_in;
extern char* registry_fifo_out;
extern pid_t iop_pid;


actor_spec* newActor(int notify, char* executable, char** argv){
  if((executable == NULL) ||  (argv == NULL)){
    goto fail;
  } else {
    actor_spec *retval;
    if(ACTOR_DEBUG)fprintf(stderr, "newActor\t:\tMaking actor_spec\n");
    retval = makeActorSpec(argv[0]);
    if(ACTOR_DEBUG)fprintf(stderr, "newActor\t:\tCalling spawnActor\n");
    if(spawnActor(retval, executable, argv) < 0) goto fail;
    if(ACTOR_DEBUG)fprintf(stderr, "newActor\t:\tCalling notifyRegistry\n");
    if(notify){
      if(notifyRegistry(retval) < 0) goto fail;
    }
    return retval;
  }

 fail:
  
  fprintf(stderr, "newActor\t:\tFailure in newActor: %s\n", strerror(errno));
  return NULL;
  
}

/* 
   We don't want to inherit the registry's handlers, on the of chance
   that we get created by it.
*/

void nullifyHandler(){
  sigset_t sigmask;
  struct sigaction sigact;
  sigfillset(&sigmask);
  sigprocmask(SIG_UNBLOCK, &sigmask, NULL);
  sigact.sa_handler = SIG_DFL;
  sigact.sa_flags = 0;
  sigemptyset(&sigact.sa_mask);
  sigaction(SIGCHLD, &sigact, NULL);
  sigaction(SIGINT, &sigact, NULL);
  sigaction(SIGUSR1, &sigact, NULL);
}

pid_t spawnActor(actor_spec *act, char* executable, char** argv){
  pid_t retval;
  int i, fds[3], flags[3] = { O_RDWR, O_RDWR,  O_RDWR };
  char* self = argv[0];
  if(ACTOR_DEBUG)fprintf(stderr, "spawnActor\t:\tOpening fifos\n");  
  for(i = 0; i < 3; i++){
    if(ACTOR_DEBUG)fprintf(stderr, "spawnActor\t:\tOpening %s\n", act->fifos[i]);
    if((fds[i] = open(act->fifos[i], flags[i])) == -1){
      if(ACTOR_DEBUG)fprintf(stderr, "spawnActor\t:\tCan't open fifo: %s\n", act->fifos[i]);
      goto fail;
    }
    if(ACTOR_DEBUG)fprintf(stderr, "spawnActor\t:\tOpened %s\n", act->fifos[i]);
  }
  if(ACTOR_DEBUG)fprintf(stderr, "spawnActor\t:\tCalling fork\n");  
  retval = fork();
  if(retval < 0){
    goto fail;
  } else if(retval == 0){
    /* child process, destined to be new actor */
    nullifyHandler();
    if(iop_debug_flag || iop_no_windows_flag){
      /* better not rely on the registry to handles its errors */
      if(strcmp(act->name, REGISTRY_ACTOR)){
	if(dup2(fds[2], STDERR_FILENO) < 0){
	  goto fail;
	}
      }
      if((dup2(fds[0], STDIN_FILENO) < 0)  ||
	 (dup2(fds[1], STDOUT_FILENO) < 0)){
	goto fail;
      } 
    } else {
      /* normal mode */
      if((dup2(fds[0], STDIN_FILENO) < 0)  ||
      	 (dup2(fds[1], STDOUT_FILENO) < 0) ||
      	 (dup2(fds[2], STDERR_FILENO) < 0)
      	 ){
      	goto fail;
      }
    }
    if((close(fds[0]) !=  0) ||
       (close(fds[1]) !=  0) ||
       (close(fds[2]) !=  0)){
      goto fail;
    } 
    /* stupid hack to keep java happy! */
    if(!strcmp(executable, "java")){
      argv[0] = "java";
      execvp(executable, argv);
    } else {
      execvp(executable, argv);
    }
    fprintf(stderr, "WARNING: Failure of execvp in actor creation: %s executable:%s \n",strerror(errno), executable);
    sleep(1);  /* give the registry time to see this error message */
    sendFormattedMsgFP(stdout, "system\n%s\nunenroll %s\n", self, self);
    sleep(1);  /* give the registry time to see this request */
    exit(EXIT_FAILURE);
  } else {
    /* calling process */
    if(ACTOR_DEBUG)fprintf(stderr, "spawnActor\t:\tForked OK\n");  
    act->pid = retval;
    if(ACTOR_DEBUG)fprintf(stderr, "spawnActor\t:\tClosing fifos\n");  
    if((close(fds[0]) !=  0) ||
       (close(fds[1]) !=  0) ||
       (close(fds[2]) !=  0)){
      perror("spawnActor\t:\tCouldn't close fifos");
      kill(retval, SIGKILL);
      return -1;
    } else {
      return retval;
    }
  }

  
  
 fail:
  
  fprintf(stderr, "spawnActor\t:\tFailure in spawnActor: %s\n", strerror(errno));
  return -1;
  
}

actor_spec* makeActorSpec(char *name){
  actor_spec *retval = NULL;
  if(name == NULL){
    goto fail;
  } else {
    int i;
    retval = (actor_spec *)calloc(1, sizeof(actor_spec));
    if(retval == NULL) goto fail;
    sprintf(retval->name, name);
    for(i = 0; i < 3; i++){
      sprintf(retval->fifos[i],
	      "/tmp/iop_%d_%s_%s",
	      iop_pid,
	      name,
	      ((i == 0) ? "IN" : ((i == 1) ? "OUT" : "ERR")));
      /* try and clean up old copies */
      (void)unlink(retval->fifos[i]);
      /* make new ones               */
      if(mkfifo(retval->fifos[i], mode[i]) == -1) goto fail;
    }
  }
  
  return retval;

 fail:
  
  fprintf(stderr, "makeActorSpec\t:\tFailure in makeActorSpec: %s\n", strerror(errno));
  return NULL;
}



void lockFD(struct flock* lock, int fd, char* comment){
  if(LOCKS_DEBUG)fprintf(stderr, "Locking %s\n", comment);  
#ifdef _MAC
  /* cannot fcntl fifos on Mac OS X */
  flock(fd, LOCK_EX);
#else
  memset(lock, 0, sizeof(struct flock));
  lock->l_type = F_WRLCK;
  if(fcntl(fd, F_SETLKW, lock) == -1){
    fprintf(stderr, "lockFD(%s)\t:\tFailure in fcntl: %s\n", comment, strerror(errno));
    return;
  }
#endif
  if(LOCKS_DEBUG)fprintf(stderr, "Locked %s\n", comment);  
}

void unlockFD(struct flock* lock, int fd, char* comment){
  if(LOCKS_DEBUG)fprintf(stderr, "Unlocking %s\n", comment);  
#ifdef _MAC
  /* cannot fcntl fifos on Mac OS X */
  flock(fd, LOCK_UN);
#else
  lock->l_type = F_UNLCK;
  if(fcntl(fd, F_SETLKW, lock) == -1){
    fprintf(stderr, "unlockFD(%s)\t:\tFailure in fcntl: %s\n", comment, strerror(errno));
    return;
  }
#endif
  if(LOCKS_DEBUG)fprintf(stderr, "Unlocked %s\n", comment);  
}

int notifyRegistry(actor_spec *acts){
  int reg_wr_fd, reg_rd_fd, retval = -1;
  struct flock wr_lock, rd_lock;
  registry_cmd_t cmd = REGISTER;
  if(acts == NULL) 
    goto fail;
  if(ACTOR_DEBUG){fprintf(stderr, "notifyRegistry\t:\tOpening Registry  write fifo\n");  
  fprintf(stderr,"Registry write fifo is: %s \n",registry_fifo_in);
  }
  if((reg_wr_fd = open(registry_fifo_in,  O_RDWR)) < 0) 
    goto fail;

  if(ACTOR_DEBUG)fprintf(stderr, "notifyRegistry\t:\tOpened Registry write fifo\n");  
    
  if(ACTOR_DEBUG)fprintf(stderr, "notifyRegistry\t:\tOpening Registry read fifo\n");  
  if((reg_rd_fd = open(registry_fifo_out,  O_RDWR)) < 0) 
    goto fail;
  if(ACTOR_DEBUG)fprintf(stderr, "notifyRegistry\t:\tOpened Registry read fifo\n");  

  if(ACTOR_DEBUG) fprintf(stderr,"notifyRegistry\t:\t registry_fifo_out: %s\n",registry_fifo_out);

  lockFD(&wr_lock, reg_wr_fd, "notifyRegistry: Registry write fifo");

  lockFD(&rd_lock, reg_rd_fd, "notifyRegistry: Registry read  fifo");

  if(ACTOR_DEBUG)fprintf(stderr, "notifyRegistry\t:\tWriting cmd\n");  
  if(writeInt(reg_wr_fd, cmd) < 0) 
    goto unlock;

  if(ACTOR_DEBUG)fprintf(stderr, "notifyRegistry\t:\tWriting actor spec\n");  
  if(writeActorSpec(reg_wr_fd, acts) < 0) goto unlock;

  retval = 0;

  {
    int slotNumber;
    if(ACTOR_DEBUG)
      fprintf(stderr, "notifyRegistry\t:\tWaiting for registry slotNumber ACK\n");
    if(readInt(reg_rd_fd, &slotNumber) < 0)
      goto fail;
    if(ACTOR_DEBUG)
      fprintf(stderr, "notifyRegistry\t:\tRead %d from registry\n", slotNumber);

  }

 unlock:

  unlockFD(&wr_lock, reg_wr_fd, "notifyRegistry: Registry write fifo");

  unlockFD(&rd_lock, reg_rd_fd, "notifyRegistry: Registry read fifo");
  
  if(ACTOR_DEBUG)fprintf(stderr, "notifyRegistry\t:\tClosing read and write fifo\n");  

  if((close(reg_wr_fd) == -1) || (close(reg_rd_fd) == -1)) 
    goto fail;

  return retval;

 fail:
  
  fprintf(stderr, "notifyRegistry\t:\tFailure in notifyRegistry: %s\n", strerror(errno));
  return retval;

  
}


int deleteFromRegistry(char *name){
  int len, reg_wr_fd, reg_rd_fd, retval = -1;
  struct flock wr_lock, rd_lock;
  registry_cmd_t cmd = UNREGISTER;
  if(name == NULL) 
    goto fail;
  len = strlen(name);
  if(ACTOR_DEBUG){
    fprintf(stderr, "deleteFromRegistry\t:\tOpening Registry  write fifo\n");  
    fprintf(stderr,"deleteFrom Registry\t:\t FIFO is: %s\n",registry_fifo_in);
  }
  if((reg_wr_fd = open(registry_fifo_in,  O_RDWR)) < 0) 
    goto fail;

  if(ACTOR_DEBUG)fprintf(stderr, "deleteFromRegistry\t:\tOpened Registry write fifo\n");  
    
  if(ACTOR_DEBUG)fprintf(stderr, "deleteFromRegistry\t:\tOpening Registry read fifo\n");  
  if((reg_rd_fd = open(registry_fifo_out,  O_RDWR)) < 0) 
    goto fail;
  //rapolzan
  if(ACTOR_DEBUG) fprintf(stderr,"deleteFromRegistry \t:\t registry_fifo_out is: %s\n",registry_fifo_out);
  //rapolzan
  if(ACTOR_DEBUG)fprintf(stderr, "deleteFromRegistry\t:\tOpened Registry read fifo\n");  


  lockFD(&wr_lock, reg_wr_fd, "deleteFromRegistry: Registry write fifo");

  lockFD(&rd_lock, reg_rd_fd, "deleteFromRegistry: Registry read  fifo");

  if(ACTOR_DEBUG)fprintf(stderr, "deleteFromRegistry\t:\tWriting cmd\n");  
  if(writeInt(reg_wr_fd, cmd) < 0) goto unlock;

  if(ACTOR_DEBUG)fprintf(stderr, "deleteFromRegistry\t:\tWriting actor name\n");  
  if(write(reg_wr_fd, name, len) != len)
    goto unlock;


  {
    int slotNumber;
    if(ACTOR_DEBUG)
      fprintf(stderr, "deleteFromRegistry\t:\tWaiting for registry slotNumber ACK\n");
    if(readInt(reg_rd_fd, &slotNumber) < 0)
      goto fail;
    retval = slotNumber;
    if(ACTOR_DEBUG)
      fprintf(stderr, "deleteFromRegistry\t:\tRead %d from registry\n", slotNumber);

  }

 unlock:

  unlockFD(&wr_lock, reg_wr_fd, "deleteFromRegistry: Registry write fifo");

  unlockFD(&rd_lock, reg_rd_fd, "deleteFromRegistry: Registry read fifo");
  
  if(ACTOR_DEBUG)fprintf(stderr, "deleteFromRegistry\t:\tClosing read and write fifo\n");  

  if((close(reg_wr_fd) == -1) || (close(reg_rd_fd) == -1)) 
    goto fail;

  return retval;

 fail:
  
  fprintf(stderr, "deleteFromRegistry\t:\tFailure %s\n", strerror(errno));
  return retval;

  
}

void sendRequest(int index, int bytes, char* buff){
  int reg_fd;
  struct flock lock;
  registry_cmd_t cmd = SEND;
  
  if(ACTOR_DEBUG)fprintf(stderr, "sendRequest\t:\tOpening Registry fifo\n");  
  if((reg_fd = open(registry_fifo_in,  O_RDWR)) < 0) 
    goto fail;
  if(ACTOR_DEBUG)fprintf(stderr, "sendRequest\t:\tOpened Registry fifo\n");  

  lockFD(&lock, reg_fd, "sendRequest: Registry fifo");

  if(ACTOR_DEBUG)fprintf(stderr, "sendRequest\t:\tWriting cmd\n");  
  if(writeInt(reg_fd, cmd) < 0) goto unlock;

  if(ACTOR_DEBUG)fprintf(stderr, "sendRequest\t:\tWriting index\n");  
  if(writeInt(reg_fd, index) < 0) goto unlock;

  if(ACTOR_DEBUG)fprintf(stderr, "sendRequest\t:\tWriting bytes\n");  
  if(writeInt(reg_fd, bytes) < 0) goto unlock;

  
  if(ACTOR_DEBUG)fprintf(stderr, "sendRequest\t:\tWriting buff\n");  
  if(write(reg_fd, buff, bytes) != bytes) 
    goto unlock;


 unlock:
  unlockFD(&lock, reg_fd, "sendRequest: Registry fifo");
  
  if(ACTOR_DEBUG)fprintf(stderr, "sendRequest\t:\tClosing fifo\n");  
  if(close(reg_fd) == -1)
    goto fail;
  return;

 fail:
  
  fprintf(stderr, "sendRequest\t:\tFailure: %s\n", strerror(errno));
  return;


}

void terminateIOP(void){
  int reg_fd;
  struct flock lock;
  registry_cmd_t cmd = KILL;
  
  if(ACTOR_DEBUG)fprintf(stderr, "Opening Registry fifo\n");  
  if((reg_fd = open(registry_fifo_in,  O_RDWR)) < 0) 
    goto fail;
  if(ACTOR_DEBUG)fprintf(stderr, "Opened Registry fifo\n");  

  lockFD(&lock, reg_fd, "terminateIOP: Registry fifo");

  if(ACTOR_DEBUG)fprintf(stderr, "Writing cmd\n");  
  if(writeInt(reg_fd, cmd) < 0) goto unlock;

 unlock:
  unlockFD(&lock, reg_fd, "terminateIOP: Registry fifo");

  if(ACTOR_DEBUG)fprintf(stderr, "Closing fifo\n");  
  if(close(reg_fd) == -1)
    goto fail;
  return;

 fail:
  fprintf(stderr, "Failure in terminateIOP: %s\n", strerror(errno));
  return;
}

