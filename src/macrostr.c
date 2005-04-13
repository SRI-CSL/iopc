/*
	Convert macro values to strings
	AUP2, Sec. 5.08 (not in book)

	Copyright 2003 by Marc J. Rochkind. All rights reserved.
	May be copied only for purposes and under conditions described
	on the Web page www.basepath.com/aup/copyright.htm.

	The Example Files are provided "as is," without any warranty;
	without even the implied warranty of merchantability or fitness
	for a particular purpose. The author and his publisher are not
	responsible for any damages, direct or incidental, resulting
	from the use or non-use of these Example Files.

	The Example Files may contain defects, and some contain deliberate
	coding mistakes that were included for educational reasons.
	You are responsible for determining if and how the Example Files
	are to be used.

*/
#include "cheaders.h"
#include "ec.h"
#include "macrostr.h"

#ifndef _MAC
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
