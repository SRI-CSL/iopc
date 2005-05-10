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

static struct {
  char *ms_cat;
  intptr_t ms_code;
  char *ms_macro;
  char *ms_desc;
} macrostr_db[] = {
#include "macrostr.incl"
  { NULL, 0, NULL, NULL}
};

char *get_macrostr(const char *cat, int code, char **desc){
  int i;
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
