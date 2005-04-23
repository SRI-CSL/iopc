/*
  Copyright 2003 by Marc J. Rochkind. All rights reserved.
  See: www.basepath.com/aup/copyright.htm.
  Munged by Ian A. Mason to be as self contained as possible.
*/
#ifndef _MACROSTR_H_
#define _MACROSTR_H_

#ifdef _MAC
#define BSD_DERIVED
#endif

char *get_macrostr(const char *cat, int code, char **desc);

#endif /* _MACROSTR_H_ */
