#include <assert.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

#include "iop_utils.h"
#include "ec.h"


int iop_install_handler(int sig, int flags, void (*sig_handler)(int)){
  struct sigaction sigactsignal;
  sigactsignal.sa_handler = sig_handler;
  sigactsignal.sa_flags = flags;
  ec_neg1( sigfillset(&sigactsignal.sa_mask) );
  ec_neg1( sigaction(sig, &sigactsignal, NULL) );

  return 0;
  
EC_CLEANUP_BGN
  return -1;
EC_CLEANUP_END

}

int  iop_install_handlers(void (*chld_fun)(int), void (*intr_fun)(int)){
  int retcode = iop_install_handler(SIGCHLD, SA_NOCLDSTOP, chld_fun);
  if(retcode != 0){ return retcode; }
  return iop_install_handler(SIGINT, 0, intr_fun);
}


void iop_usleep(uint32_t msec){
  struct timespec timeout0;
  struct timespec timeout1;
  struct timespec* tmp;
  struct timespec* t0 = &timeout0;
  struct timespec* t1 = &timeout1;
  
  t0->tv_sec = msec / 1000;
  t0->tv_nsec = (msec % 1000) * (1000 * 1000);

  while ((nanosleep(t0, t1) == (-1)) && (errno == EINTR)){
    tmp = t0;
    t0 = t1;
    t1 = tmp;
  }
}

char* time2string(void){
  time_t t;
  char *date;
  time(&t);
  date = ctime(&t);
  if(date != NULL){
    char* cr = strchr(date, '\n');
    if(cr != NULL){ cr[0] = '\t';  }
  }
  return date;
}



char* iop_getcwd(void){
  long maxpath = 0;
  char *fullpath = NULL;
  
  if((maxpath = pathconf(".",  _PC_PATH_MAX)) != -1){
    fullpath = (char *)calloc(maxpath, sizeof(char));
    if(fullpath != NULL && getcwd(fullpath, maxpath) == NULL){
      free(fullpath);
      fullpath = NULL;
    }
  }
  
  return fullpath;
}



