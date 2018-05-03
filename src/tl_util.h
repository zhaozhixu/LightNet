#ifndef _TL_UTIL_H_
#define _TL_UTIL_H_

#include <stdlib.h>

#define TL_MAXLINE 4096

#define tl_free free

#ifdef __cplusplus
extern "C" {
#endif

void *tl_alloc(size_t size);
void *tl_clone(const void *src, size_t size);
void *tl_repeat(void *data, size_t size, int times);
void tl_err_msg(const char *fmt, ...);
void tl_err_cont(int error, const char *fmt, ...);
void tl_err_ret(const char *fmt, ...);
void tl_err_quit(const char *fmt, ...);
void tl_err_bt(const char *fmt, ...);
void tl_err_exit(int error, const char *fmt, ...);
void tl_err_sys(const char *fmt, ...);
void tl_err_dump(const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif	/* _TL_UTIL_H_ */
