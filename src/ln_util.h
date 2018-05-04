#ifndef _LN_UTIL_H_
#define _LN_UTIL_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef enum ln_bool ln_bool;
enum ln_bool {
     LN_FALSE = 0,
     LN_TRUE = 1
};

typedef int (*ln_cmp_func)(void *, void *);

#define ln_free free

#define LN_MAXLINE 4096

#ifdef __cplusplus
extern "C" {
#endif

void *ln_alloc(size_t size);
char *ln_path_alloc(size_t *sizep);
void *ln_clone(const void *src, size_t size);
void *ln_repeat(void *data, size_t size, int times);
void ln_err_msg(const char *fmt, ...);
void ln_err_cont(int error, const char *fmt, ...);
void ln_err_ret(const char *fmt, ...);
void ln_err_quit(const char *fmt, ...);
void ln_err_exit(int error, const char *fmt, ...);
void ln_err_sys(const char *fmt, ...);
void ln_err_dump(const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif	/* _LN_UTIL_H_ */
