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
#include "iop_lib.h"
#include "dbugflags.h"
#include "ec.h"

static mode_t mode[3] = { S_IRWXU,  S_IRWXU, S_IRWXU };

/* externs used in the announce routine */
extern int iop_debug_flag;
extern int iop_no_windows_flag;


extern char* registry_fifo_in;
extern char* registry_fifo_out;
extern pid_t iop_pid;


actor_spec* newActor(int notify, char* executable, char** argv){
  actor_spec *retval = NULL;
  if((executable == NULL) ||  (argv == NULL)){
    fprintf(stderr, "newActor\t:\tFailure in newActor: BAD ARGS\n");
    return retval;
  } else {
    announce("newActor\t:\tMaking actor_spec\n");
    retval = makeActorSpec(argv[0]);
    announce("newActor\t:\tCalling spawnActor\n");
    if(spawnActor(retval, executable, argv) < 0) goto fail;
    announce("newActor\t:\tCalling notifyRegistry\n");
    if(notify){
      if(notifyRegistry(retval) < 0) goto fail;
    }
    return retval;
  }
 fail:
  free(retval);
  fprintf(stderr, "newActor\t:\tFailure in newActor: %s\n", strerror(errno));
  return NULL;
}

/* 
   We don't want to inherit the registry's handlers, on the off chance
   that we get created by it.
*/

static int nullifyHandler(void){
  sigset_t sigmask;
  struct sigaction sigact;
  ec_neg1( sigfillset(&sigmask) );
  ec_neg1( sigprocmask(SIG_UNBLOCK, &sigmask, NULL) );
  sigact.sa_handler = SIG_DFL;
  sigact.sa_flags = 0;
  ec_neg1( sigemptyset(&sigact.sa_mask)      );
  ec_neg1( sigaction(SIGCHLD, &sigact, NULL) );
  ec_neg1( sigaction(SIGINT, &sigact, NULL)  );
  ec_neg1( sigaction(SIGUSR1, &sigact, NULL) );
  return 0;
EC_CLEANUP_BGN
  return -1;
EC_CLEANUP_END
}

pid_t spawnActor(actor_spec *act, char* executable, char** argv){
  pid_t retval = -1;
  int i, fds[3], flags[3] = { O_RDWR, O_RDWR,  O_RDWR };
  char* self = argv[0];
  announce("spawnActor\t:\tOpening fifos\n");  
  for(i = 0; i < 3; i++){
    announce("spawnActor\t:\tOpening %s\n", act->fifos[i]);
    ec_neg1( fds[i] = open(act->fifos[i], flags[i]) );
    announce("spawnActor\t:\tOpened %s\n", act->fifos[i]);
  }
  announce("spawnActor\t:\tCalling fork\n");  
  ec_neg1( retval = fork() );
  if(retval == 0){
    /* child process, destined to be new actor */
    if( nullifyHandler() < 0){ goto fail;  }
    if(iop_debug_flag || iop_no_windows_flag){
      /* better not rely on the registry to handles its errors */
      if(strcmp(act->name, REGISTRY_ACTOR)){
        ec_neg1( dup2(fds[2], STDERR_FILENO) );
      }
      ec_neg1( dup2(fds[0], STDIN_FILENO)  );
      ec_neg1( dup2(fds[1], STDOUT_FILENO) );
    } else {
      /* normal mode */
      ec_neg1( dup2(fds[0], STDIN_FILENO)  );
      ec_neg1( dup2(fds[1], STDOUT_FILENO) );
      ec_neg1( dup2(fds[2], STDERR_FILENO) );
    }
    ec_neg1( close(fds[0]) );
    ec_neg1( close(fds[1]) );
    ec_neg1( close(fds[2]) );
    /* stupid hack to keep java happy! */
    if(!strcmp(executable, "java")){
      argv[0] = "java";
      execvp(executable, argv);
    } else {
      execvp(executable, argv);
    }
    fprintf(stderr, 
            "WARNING: Failure of execvp in actor creation: %s executable:%s \n",
            strerror(errno), executable);
    sleep(1);  /* give the registry time to see this error message */
    sendFormattedMsgFP(stdout, "system\n%s\nunenroll %s\n", self, self);
    sleep(1);  /* give the registry time to see this request */
    exit(EXIT_FAILURE);
  } else {
    /* calling process */
    announce("spawnActor\t:\tForked OK\n");  
    act->pid = retval;
    announce("spawnActor\t:\tClosing fifos\n");  
    ec_neg1( close(fds[0]) );
    ec_neg1( close(fds[1]) );
    ec_neg1( close(fds[2]) );
    return retval;
  }
 fail:
  EC_CLEANUP_BGN
    if(retval > 0){ kill(retval, SIGKILL); }
  return -1;
  EC_CLEANUP_END
}

actor_spec* makeActorSpec(char *name){
  int i;
  actor_spec *retval = NULL;
  if(name != NULL){
    ec_null( retval = calloc(1, sizeof(actor_spec)) );
    sprintf(retval->name, "%s", name);
    for(i = 0; i < 3; i++){
      char *stream = ((i == 0) ? "IN" : ((i == 1) ? "OUT" : "ERR"));
      sprintf(retval->fifos[i], "/tmp/iop_%d_%s_%s", iop_pid, name, stream);
      /* try and clean up old copies, ignore failure */
      (void)unlink(retval->fifos[i]);
      /* make new ones               */
      ec_neg1( mkfifo(retval->fifos[i], mode[i]) );
    }
    return retval;
  }
EC_CLEANUP_BGN
  free(retval);
 return NULL;
EC_CLEANUP_END
}



int lockFD(struct flock* lock, int fd, char* comment){
  if(LOCKS_DEBUG)fprintf(stderr, "Locking %s\n", comment);  
#ifdef _MAC
  /* cannot fcntl fifos on Mac OS X, what does this actually return?  */
  flock(fd, LOCK_EX) ;
  if(LOCKS_DEBUG)fprintf(stderr, "Locked(%p) %s\n", (void *)lock, comment);  
  return 0;
#else
  memset(lock, 0, sizeof(struct flock));
  lock->l_type = F_WRLCK;
  ec_neg1( fcntl(fd, F_SETLKW, lock) );
  if(LOCKS_DEBUG)fprintf(stderr, "Locked %s\n", comment);  
  return 0;
EC_CLEANUP_BGN
  return -1;
EC_CLEANUP_END
#endif
}

int unlockFD(struct flock* lock, int fd, char* comment){
  if(LOCKS_DEBUG)fprintf(stderr, "Unlocking %s\n", comment);  
#ifdef _MAC
  /* cannot fcntl fifos on Mac OS X, what does this actually return? */
  flock(fd, LOCK_UN);
  if(LOCKS_DEBUG)fprintf(stderr, "Unlocked(%p) %s\n", (void *)lock, comment);  
  return 0;
#else
  lock->l_type = F_UNLCK;
  ec_neg1( fcntl(fd, F_SETLKW, lock) ); 
  if(LOCKS_DEBUG)fprintf(stderr, "Unlocked %s\n", comment);  
  return 0;
EC_CLEANUP_BGN
  return -1;
EC_CLEANUP_END
#endif
}

int notifyRegistry(actor_spec *acts){
  int reg_wr_fd = -1, reg_rd_fd = -1, retval = -1;
  struct flock wr_lock, rd_lock;
  registry_cmd_t cmd = REGISTER;
  if(acts == NULL){ goto fail; }
  announce("notifyRegistry\t:\tOpening Registry  write fifo\n");  
  announce("Registry write fifo is: %s \n", registry_fifo_in);
  ec_neg1( reg_wr_fd = open(registry_fifo_in,  O_RDWR) );
  announce("notifyRegistry\t:\tOpened Registry write fifo\n");  
  announce("notifyRegistry\t:\tOpening Registry read fifo\n");  
  ec_neg1( reg_rd_fd = open(registry_fifo_out,  O_RDWR) );
  announce("notifyRegistry\t:\tOpened Registry read fifo\n");  
  announce("notifyRegistry\t:\t registry_fifo_out: %s\n",registry_fifo_out);
  if(lockFD(&wr_lock, reg_wr_fd, "notifyRegistry: Registry write fifo") < 0){ goto fail; }
  if(lockFD(&rd_lock, reg_rd_fd, "notifyRegistry: Registry read  fifo") < 0){ goto fail; }
  announce("notifyRegistry\t:\tWriting cmd\n");  
  if(writeInt(reg_wr_fd, cmd) < 0){ goto unlock; }
  announce("notifyRegistry\t:\tWriting actor spec\n");  
  if(writeActorSpec(reg_wr_fd, acts) < 0){ goto unlock; }
  retval = 0;
  {
    int slotNumber;
    announce("notifyRegistry\t:\tWaiting for registry slotNumber ACK\n");
    if(readInt(reg_rd_fd, &slotNumber, "notifyRegistry") < 0){ goto fail; }
    announce("notifyRegistry\t:\tRead %d from registry\n", slotNumber);
  }
 unlock:
  if(unlockFD(&wr_lock, reg_wr_fd, "notifyRegistry: Registry write fifo") < 0){ goto fail; }
  if(unlockFD(&rd_lock, reg_rd_fd, "notifyRegistry: Registry read fifo") < 0){ goto fail; }
  announce("notifyRegistry\t:\tClosing read and write fifo\n");  
  ec_neg1( close(reg_wr_fd) );
  ec_neg1( close(reg_rd_fd) ); 
  return retval;
 fail:
  if(reg_wr_fd >= 0){ ec_neg1( close(reg_wr_fd) ); }
  if(reg_rd_fd >= 0){ ec_neg1( close(reg_rd_fd) ); }
EC_CLEANUP_BGN
  return -1;
EC_CLEANUP_END

}


int deleteFromRegistry(char *name){
  int len, reg_wr_fd = -1, reg_rd_fd = -1, retval = -1;
  struct flock wr_lock, rd_lock;
  registry_cmd_t cmd = UNREGISTER;
  if(name == NULL){ goto fail; }
  len = strlen(name);
  announce("deleteFromRegistry\t:\tOpening Registry  write fifo\n");  
  announce("deleteFrom Registry\t:\t FIFO is: %s\n",registry_fifo_in);
  ec_neg1( reg_wr_fd = open(registry_fifo_in,  O_RDWR) );
  announce("deleteFromRegistry\t:\tOpened Registry write fifo\n");  
  announce("deleteFromRegistry\t:\tOpening Registry read fifo\n");  
  ec_neg1( reg_rd_fd = open(registry_fifo_out,  O_RDWR) );
  announce("deleteFromRegistry\t:\tOpened Registry read fifo\n");  
  if(lockFD(&wr_lock, reg_wr_fd, "deleteFromRegistry: Registry write fifo") < 0){ goto fail; }
  if(lockFD(&rd_lock, reg_rd_fd, "deleteFromRegistry: Registry read  fifo") < 0){ goto fail; }
  announce("deleteFromRegistry\t:\tWriting cmd\n");  
  if(writeInt(reg_wr_fd, cmd) < 0){ goto unlock; }
  announce("deleteFromRegistry\t:\tWriting actor name\n");  
  if(write(reg_wr_fd, name, len) != len){ goto unlock; }
  {
    int slotNumber;
    announce("deleteFromRegistry\t:\tWaiting for registry slotNumber ACK\n");
    if(readInt(reg_rd_fd, &slotNumber, "deleteFromRegistry") < 0)
      goto fail;
    retval = slotNumber;
    announce("deleteFromRegistry\t:\tRead %d from registry\n", slotNumber);

  }
 unlock:
  if(unlockFD(&wr_lock, reg_wr_fd, "deleteFromRegistry: Registry write fifo") < 0){ goto fail; }
  if(unlockFD(&rd_lock, reg_rd_fd, "deleteFromRegistry: Registry read fifo") < 0){ goto fail; }
  announce("deleteFromRegistry\t:\tClosing read and write fifo\n");  
  ec_neg1( close(reg_wr_fd) );
  ec_neg1( close(reg_rd_fd) ); 
  return retval;
 fail:
  if(reg_wr_fd >= 0){ ec_neg1( close(reg_wr_fd) ); }
  if(reg_rd_fd >= 0){ ec_neg1( close(reg_rd_fd) ); }
EC_CLEANUP_BGN
  return -1;
EC_CLEANUP_END

}

int sendRequest(int index, int bytes, char* buff){
  int reg_fd = -1;
  struct flock lock;
  registry_cmd_t cmd = SEND;
  announce("sendRequest\t:\tOpening Registry fifo\n");  
  ec_neg1( reg_fd = open(registry_fifo_in,  O_RDWR) );
  announce("sendRequest\t:\tOpened Registry fifo\n");  
  if(lockFD(&lock, reg_fd, "sendRequest: Registry fifo") < 0){ goto fail; }
  announce("sendRequest\t:\tWriting cmd\n");  
  if(writeInt(reg_fd, cmd) < 0){ goto unlock; }
  announce("sendRequest\t:\tWriting index\n");  
  if(writeInt(reg_fd, index) < 0){ goto unlock; }
  announce("sendRequest\t:\tWriting bytes\n");  
  if(writeInt(reg_fd, bytes) < 0){ goto unlock; }
  announce("sendRequest\t:\tWriting buff\n");  
  if(write(reg_fd, buff, bytes) != bytes){ goto unlock; }
 unlock:
  if(unlockFD(&lock, reg_fd, "sendRequest: Registry fifo") < 0){ goto fail; }
  announce("sendRequest\t:\tClosing fifo\n");  
  ec_neg1( close(reg_fd) );
  return 0;
 fail:
  if(reg_fd >= 0){ ec_neg1( close(reg_fd) ); }
EC_CLEANUP_BGN
  return -1;
EC_CLEANUP_END

}

int terminateIOP(void){
  int reg_fd = -1;
  struct flock lock;
  registry_cmd_t cmd = KILL;
  announce("Opening Registry fifo\n");  
  ec_neg1(reg_fd = open(registry_fifo_in,  O_RDWR) );
  announce("Opened Registry fifo\n");  
  if(lockFD(&lock, reg_fd, "terminateIOP: Registry fifo") < 0){ goto fail; }
  announce("Writing cmd\n");  
  if(writeInt(reg_fd, cmd) < 0){ goto unlock; }
 unlock:
  if(unlockFD(&lock, reg_fd, "terminateIOP: Registry fifo") < 0){ goto fail; }
  announce("Closing fifo\n");  
  ec_neg1( close(reg_fd) );
  return 0;
 fail:
  if(reg_fd >= 0){ ec_neg1( close(reg_fd) ); }
EC_CLEANUP_BGN
  return -1;
EC_CLEANUP_END

}

