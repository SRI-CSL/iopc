/*
  Copyright 2003 by Marc J. Rochkind. All rights reserved.
  See: www.basepath.com/aup/copyright.htm.
  Munged by Ian A. Mason to be as self contained as possible.
*/
#include "cheaders.h"
#include "ec.h"
#include "macrostr.h"

#ifndef BSD_DERIVED
#include <sys/msg.h>
#endif

#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/mman.h>

/* ubuntu complaints */
#ifndef EAI_ADDRFAMILY
#define EAI_ADDRFAMILY 1
#endif

static int initialized = 0;

static struct {
  char *ms_cat;
  intptr_t ms_code;
  char *ms_macro;
  char *ms_desc;
} macrostr_db[] = {
#include "macrostr1.incl"
  { NULL, 0, NULL, NULL}
};

void macrostr_init(void){
  if (macrostr_db[0].ms_code == 0) {
#include "macrostr2.incl"
  }
}


char *get_macrostr(const char *cat, int code, char **desc){
  int i;
  /* iam's attempt at self initialization (probably not thread safe!) */
  if(!initialized){
    macrostr_init();
    initialized = 1;
  }
  for (i = 0; macrostr_db[i].ms_cat != NULL; i++)
    if (strcmp(macrostr_db[i].ms_cat, cat) == 0 &&
	macrostr_db[i].ms_code == code) {
      if (desc != NULL)
	*desc = macrostr_db[i].ms_desc;
      return macrostr_db[i].ms_macro;
    }
  if (desc != NULL)
    *desc = "?";
  return "?";
}
