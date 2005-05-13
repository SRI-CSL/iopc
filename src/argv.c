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
#include "types.h"
#include "argv.h"
#include "ec.h"

int makeArgv(const char *s, const char *delimiters, char ***argvp){
  int max;
  char **argv = NULL;
  if(argvp == NULL)    
    return 0;
  if(s == NULL){
    *argvp = NULL;
    return 1;
  } else {
    int argc = 0, len = strlen(s);
    if(len == 0){
      *argvp = NULL;
      return 1;
    } else {
      int start = 0, end = 0;
      ec_null( argv = calloc(len, sizeof(char *)) );
      while(s[start] != '\0'){
	while((s[start] != '\0') &&
	      (strchr(delimiters, s[start]) != NULL))
	  start++;
	if(s[start] == '\0'){
	  if(argc == 0){
	    free(argv);
	    *argvp = NULL;
	    return argc;
	  } else {
	    argv[argc] = NULL;
	    *argvp = argv;
	    return argc;
	  }
	}
	end = start;
	while((s[end] != '\0') &&
	      strchr(delimiters, s[end]) == NULL)
	  end++;
	max = (PATH_MAX < (end - start) + 1) ? (end - start) + 1 : PATH_MAX;
	ec_null( argv[argc] = calloc(max, sizeof(char)) );
	strncpy(argv[argc], &s[start], end - start);
	argv[argc][end - start] = '\0';
	argc++;
	start = end;
      }
      argv[argc] = NULL;
      *argvp = argv;
      return argc;
    }
  }

EC_CLEANUP_BGN
  (void)free(argv);
  return 0;
EC_CLEANUP_END
}

void freeArgv(int argc, char** argv){
  int i;
  for(i = 0; i < argc; i++){
    (void)free(argv[i]);
  }
  (void)free(argv);
}

void printArgv(FILE* file, int argc, char** argv, char* prefix){
  int i;
  for(i = 0; i < argc; i++){
    fprintf(file, "\t%s[%d] = %s\n", prefix, i, argv[i]);
  }
}
