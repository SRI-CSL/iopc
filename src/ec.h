#ifndef _EC_H_
#define _EC_H_

extern const int ec_in_cleanup;

typedef enum {EC_ERRNO = 0, EC_EAI = 1, EC_GETDATE = 2, EC_NONE = 3} EC_ERRTYPE;

#define EC_CLEANUP_BGN\
	ec_warn();\
	ec_cleanup_bgn:\
	{\
		int ec_in_cleanup;\
		ec_in_cleanup = 1;

#define EC_CLEANUP_END\
	}

#define ec_cmp(var, errrtn)\
	{\
		assert(!ec_in_cleanup);\
		if ((intptr_t)(var) == (intptr_t)(errrtn)) {\
			ec_push(__func__, __FILE__, __LINE__, #var, errno, EC_ERRNO);\
			goto ec_cleanup_bgn;\
		}\
	}

#define ec_rv(var)\
	{\
		int errrtn;\
		assert(!ec_in_cleanup);\
		if ((errrtn = (var)) != 0) {\
			ec_push(__func__, __FILE__, __LINE__, #var, errrtn, EC_ERRNO);\
			goto ec_cleanup_bgn;\
		}\
	}

#define ec_ai(var)\
	{\
		int errrtn;\
		assert(!ec_in_cleanup);\
		if ((errrtn = (var)) != 0) {\
			ec_push(__func__, __FILE__, __LINE__, #var, errrtn, EC_EAI);\
			goto ec_cleanup_bgn;\
		}\
	}

#define ec_neg1(x) ec_cmp(x, -1)
#define ec_null(x) ec_cmp(x, 0)
#define ec_zero(x) ec_null(x)
#define ec_false(x) ec_cmp(x, 0)
#define ec_eof(x) ec_cmp(x, EOF)
#define ec_nzero(x)\
	{\
		if ((x) != 0)\
			EC_FAIL\
	}

#define EC_FAIL ec_cmp(0, 0)

#define EC_CLEANUP goto ec_cleanup_bgn;

#define EC_FLUSH(str)\
	{\
		ec_print();\
		ec_reinit();\
	}

#define EC_EINTERNAL INT_MAX

void ec_push(const char *fcn, const char *file, int line,
	     const char *str, int errno_arg, EC_ERRTYPE type);
void ec_print(void);
void ec_reinit(void);
void ec_warn(void);

#endif /* _EC_H_ */
