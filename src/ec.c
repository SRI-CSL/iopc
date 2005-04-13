/*
  Copyright 2003 by Marc J. Rochkind. All rights reserved.
  See: www.basepath.com/aup/copyright.htm.
  Munged by Ian A. Mason to be as self contained as possible.
*/
#include "ec.h"
#include "macrostr.h"
#include "cheaders.h"

const char *getdate_strerror(int e){
  const char *s[] = {
    "Invalid getdate_err value",
    "DATEMSK environment variable null or undefined",
    "Template file cannot be opened for reading",
    "Failed to get file status information",
    "Template file not a regular file",
    "I/O error encountered while reading template file",
    "Memory allocation failed",
    "No line in template that matches input",
    "Invalid input specification"
  };
  if (e < 1 || e > 8) return s[0];
  return s[e];
}

char *syserrmsgtype(char *buf, size_t buf_max, const char *msg, int errno_arg, EC_ERRTYPE type){
	const char *errmsg;
	char *cat = "?";
	if (msg == NULL)
		msg = "???";
	if (errno_arg == 0)
		snprintf(buf, buf_max, "%s", msg);
	else {
		if (errno_arg == EC_EINTERNAL)
			errmsg = "Internal error (nonstandard)";
		else if (type == EC_EAI) {
			cat = "eai";
			errmsg = gai_strerror(errno_arg);
		}
		else if (type == EC_GETDATE)
			errmsg = getdate_strerror(errno_arg);
		else {
			cat = "errno";
			errmsg = strerror(errno_arg);
		}
		snprintf(buf, buf_max, "%s\n\t\t*** %s (%d: \"%s\") ***", msg,
			 get_macrostr(cat, errno_arg, NULL), errno_arg,
			 errmsg != NULL ? errmsg : "no message string");
	}
	return buf;
}

static void ec_mutex(int lock){
  static pthread_mutex_t ec_mtx = PTHREAD_MUTEX_INITIALIZER;
  int errnum;
  char *msg;

  if (lock) {
    if ((errnum = pthread_mutex_lock(&ec_mtx)) == 0)
      return;
  }
  else {
    if ((errnum = pthread_mutex_unlock(&ec_mtx)) == 0)
      return;
  }
  if ((msg = strerror(errnum)) == NULL)
    fprintf(stderr, "Mutex error in ec_* function: %d\n", errnum);
  else
    fprintf(stderr, "Mutex error in ec_* function: %s\n", msg);
  exit(EXIT_FAILURE);
}

static void ec_atexit_fcn(void){
  ec_print();
}

static struct ec_node {
  struct ec_node *ec_next;
  int ec_errno;
  EC_ERRTYPE ec_type;
  char *ec_context;
} *ec_head, ec_node_emergency;
static char ec_s_emergency[100];

const int ec_in_cleanup = 0;

#define SEP1 " ["
#define SEP2 ":"
#define SEP3 "] "

void ec_push(const char *fcn, const char *file, int line,
	     const char *str, int errno_arg, EC_ERRTYPE type){
  struct ec_node node, *p;
  size_t len;
  static int attexit_called = 0;

  ec_mutex(1);
  node.ec_errno = errno_arg;
  node.ec_type = type;
  if (str == NULL)
    str = "";
  len = strlen(fcn) + strlen(SEP1) + strlen(file) + strlen(SEP2) +
    6 + strlen(SEP3) + strlen(str) + 1;
  node.ec_context = (char *)calloc(1, len);
  if (node.ec_context == NULL) {
    if (ec_s_emergency[0] == '\0')
      node.ec_context = ec_s_emergency;
    else
      node.ec_context = "?";
    len = sizeof(ec_s_emergency);
  }
  if (node.ec_context != NULL)
    snprintf(node.ec_context, len, "%s%s%s%s%d%s%s", fcn, SEP1,
	     file, SEP2, line, SEP3, str);
  p = (struct ec_node *)calloc(1, sizeof(struct ec_node));
  if (p == NULL && ec_node_emergency.ec_context == NULL)
    p = &ec_node_emergency; /* use just once */
  if (p != NULL) {
    node.ec_next = ec_head;
    ec_head = p;
    *ec_head = node;
  }
  if (!attexit_called) {
    attexit_called = 1;
    ec_mutex(0);
    if (atexit(ec_atexit_fcn) != 0) {
      ec_push(fcn, file, line, "atexit failed", errno, EC_ERRNO);
      ec_print(); /* so at least the error gets shown */
    }
  }
  else
    ec_mutex(0);
}

void ec_print(void){
  struct ec_node *e;
  int level = 0;

  ec_mutex(1);
  for (e = ec_head; e != NULL; e = e->ec_next, level++) {
    char buf[200], buf2[25 + sizeof(buf)];

    if (e == &ec_node_emergency)
      fprintf(stderr, "\t*** Trace may be incomplete ***\n");
    syserrmsgtype(buf, sizeof(buf), e->ec_context,
		  e->ec_next == NULL ? e->ec_errno : 0, e->ec_type);
    snprintf(buf2, sizeof(buf2), "%s\t%d: %s",
    	     (level == 0? "ERROR:" : ""), level, buf);
    fprintf(stderr, "%s\n", buf2);
  }
  ec_mutex(0);
}

void ec_reinit(void){
  struct ec_node *e, *e_next;

  ec_mutex(1);
  for (e = ec_head; e != NULL; e = e_next) {
    e_next = e->ec_next;
    if (e->ec_context != ec_s_emergency)
      free(e->ec_context);
    if (e != &ec_node_emergency)
      free(e);
  }
  ec_head = NULL;
  memset(&ec_node_emergency, 0, sizeof(ec_node_emergency));
  memset(&ec_s_emergency, 0, sizeof(ec_s_emergency));
  ec_mutex(0);
}

void ec_warn(void)
{
  fprintf(stderr, "***WARNING: Control flowed into EC_CLEANUP_BGN\n");
}
