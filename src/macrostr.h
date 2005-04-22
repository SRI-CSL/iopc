#ifndef _MACROSTR_H_
#define _MACROSTR_H_

#ifdef _MAC
#define BSD_DERIVED
#endif

char *get_macrostr(const char *cat, int code, char **desc);

#endif /* _MACROSTR_H_ */
