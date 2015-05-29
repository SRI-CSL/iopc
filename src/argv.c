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

static char quotes = '"';

int makeArgv(const char *s, const char *delimiters, char ***argvp){
  int max;
  char **argv = NULL;
  if(strchr(delimiters, quotes) != NULL){
    fprintf(stderr, "makeArgv warning: bad args -- delimiters contains quotes\n");
    *argvp = NULL;
    return 0;
  }
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
	int insideQuotes = 0;
	while((s[start] != '\0') &&  (strchr(delimiters, s[start]) != NULL))
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
	if(s[start] == quotes){ insideQuotes = 1; }
	end = start;
	while((s[end] != '\0') &&
	      (insideQuotes || strchr(delimiters, s[end]) == NULL)){
	  if(s[end] == quotes){ insideQuotes = !insideQuotes; }
	  end++;
	}
	max = (PATH_MAX < (end - start) + 1) ? (end - start) + 1 : PATH_MAX;
	ec_null( argv[argc] = calloc(max, sizeof(char)) );
	strncpy(argv[argc], &s[start], end - start);
	argv[argc][end - start] = '\0';
	if(insideQuotes){
	  fprintf(stderr, "makeArgv warning: matching quote not found: %s!\n", argv[argc]);
	}
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

/* returns 0 if it didn't do something, and 1 if it did */
int interpretTildes(const char* filename, char **newfilenamep){
  if((filename == NULL) || 
     (strchr(filename, '~') != filename) || 
     (newfilenamep == NULL)){
    return 0;
  } else {
    uid_t me = getuid();
    char *newfilename = (char *)calloc(PATH_MAX, sizeof(char));
    struct passwd *myEntry = getpwuid(me);
    if((newfilename == NULL) || (myEntry == NULL)){
      fprintf(stderr, 
	      "Failure in interpretTildes: (newfilename == NULL) || (myEntry == NULL) -- %s\n",
	      strerror(errno));
      return 0;
    }
    snprintf(newfilename, PATH_MAX, "%s%s", myEntry->pw_dir, filename + 1);
    *newfilenamep = newfilename;
    return 1;
  }
}

char *argv2String(int argc, char** argv, const char* seperator){
  char *retval = NULL;
  if(argc < 2){
    char* arg = argv[0];
    if(arg != NULL){
      int len = strlen(arg) + 1;
      retval = (char *)calloc(len, sizeof(char));
      if(retval == NULL){ 
	fprintf(stderr, "calloc failure in argv2String: %s\n", strerror(errno));
	return retval;
      }
      strcpy(retval, arg);
    }
  } else {
    int i, len = 0, slen = (seperator == NULL) ? 0 : strlen(seperator) + 1;
    if(seperator == NULL){ seperator = ""; }
    for(i = 0; i < argc; i++){
      char *arg = argv[i];
      len += (arg == NULL) ? 0 : strlen(arg);
      len += slen;
    }
    if(len > 0){
      retval = (char *)calloc(len, sizeof(char));
      if(retval == NULL){ 
	fprintf(stderr, "calloc failure in argv2String: %s\n", strerror(errno));
      } else {
	for(i = 0; i < argc; i++){
	  if(i == 0){ 
	    strcpy(retval, argv[i]); 
	  } else {
	    strcat(retval, seperator);
	    strcat(retval, argv[i]);
	  }
	}
      }
    }
  }
  return retval;
}


/* returns 0 if it didn't do something, and >= 1 if it did */
int interpretTildesCSL(const char* path, char **newpathp){
  char **pathv = NULL;
  int retval = 0, i, pathc;
  if(path == NULL){ return retval; }
  pathc = makeArgv(path, ":\n", &pathv);
  for(i = 0; i < pathc; i++){
    char *entryI = pathv[i];
    char *entryO = NULL;
    int tilde = interpretTildes(entryI, &entryO);
    if(tilde){
      pathv[i] = entryO;
      free(entryI);
      retval++;
    }
  }
  *newpathp = argv2String(pathc, pathv, ":");
  freeArgv(pathc, pathv);
  return retval;
}
