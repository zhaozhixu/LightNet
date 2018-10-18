/*
 * Copyright (c) 2018 Zhao Zhixu
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <errno.h>
#include <limits.h>
#include <unistd.h>
#include <err.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/stat.h>

#include "ln_util.h"

void *ln_alloc(size_t size)
{
     void *p;

     p = malloc(size);
     if (p == NULL) {
          err(EXIT_FAILURE, "ln_alloc: malloc(%luz) failed", size);
     }

     return p;
}

void *ln_realloc(void *ptr, size_t size)
{
     void *p;

     p = realloc(ptr, size);
     if (p == NULL && size != 0) {
          err(EXIT_FAILURE, "ln_realloc: realloc() failed");
     }

     return p;
}

char *ln_strdup(const char *s)
{
     char *new_s;

     new_s = strdup(s);
     if (new_s == NULL) {
          err(EXIT_FAILURE, "ln_strdup: strdup(%s) failed", s);
     }

     return new_s;
}

void *ln_clone(const void *src, size_t size)
{
     assert(src);
     void *p;
     p = ln_alloc(size);
     memmove(p, src, size);
     return p;
}

void *ln_repeat(void *data, size_t size, int times)
{
     assert(data && times > 0);
     void *p, *dst;
     int i;
     dst = p = ln_alloc(size * times);
     for (i = 0; i < times; i++, p = (char *)p + size * times)
          memmove(p, data, size);
     return dst;
}

char *ln_read_text(const char *file_name)
{
     struct stat buf;
     FILE *fp;
     size_t n;
     char *str;

     if (stat(file_name, &buf) < 0)
          err(EXIT_FAILURE, "ln_read_text: cannot stat %s", file_name);

     if (!(fp = fopen(file_name, "rb")))
          err(EXIT_FAILURE, "ln_read_text: cannot open %s", file_name);

     str = ln_alloc(buf.st_size+1);
     n = fread(str, buf.st_size, 1, fp);
     if (n < 1 && ferror(fp))
          err(EXIT_FAILURE, "ln_read_text: cannot read %s", file_name);

     fclose(fp);
     return str;
}

static void err_doit(int errnoflag, int error, const char *fmt, va_list ap)
{
     char buf[LN_MAXLINE];

     vsnprintf(buf, LN_MAXLINE-1, fmt, ap);
     if (errnoflag)
          snprintf(buf+strlen(buf), LN_MAXLINE-strlen(buf)-1, ": %s",
               strerror(error));
     strcat(buf, "\n");
     fflush(stdout);
     fputs(buf, stderr);
     fflush(NULL);
}

/*
 * Nonfatal error unrelated to a system call.
 * Print a message and return.
 */
void ln_err_msg(const char *fmt, ...)
{
     va_list ap;
     va_start(ap, fmt);
     err_doit(0, 0, fmt, ap);
     va_end(ap);
}

/*
 * Nonfatal error unrelated to a system call.
 * Error code passed as explict parameter.
 * Print a message and return.
 */
void ln_err_cont(int error, const char *fmt, ...)
{
     va_list ap;
     va_start(ap, fmt);
     err_doit(1, error, fmt, ap);
     va_end(ap);
}

/*
 * Nonfatal error related to a system call.
 * Print a message and return.
 */
void ln_err_ret(const char *fmt, ...)
{
     va_list ap;
     va_start(ap, fmt);
     err_doit(1, errno, fmt, ap);
     va_end(ap);
}

/*
 * Fatal error unrelated to a system call.
 * Print a message and terminate.
 */
void ln_err_quit(const char *fmt, ...)
{
     va_list ap;
     va_start(ap, fmt);
     err_doit(0, 0, fmt, ap);
     va_end(ap);
     exit(1);
}

/*
 * Fatal error unrelated to a system call.
 * Error code passed as explict parameter.
 * Print a message and terminate.
 */
void ln_err_exit(int error, const char *fmt, ...)
{
     va_list
          ap;
     va_start(ap, fmt);
     err_doit(1, error, fmt, ap);
     va_end(ap);
     exit(1);
}

/*
 * Fatal error related to a system call.
 * Print a message and terminate.
 */
void ln_err_sys(const char *fmt, ...)
{
     va_list ap;
     va_start(ap, fmt);
     err_doit(1, errno, fmt, ap);
     va_end(ap);
     exit(1);
}

/*
 * Fatal error related to a system call.
 * Print a message, dump core, and terminate.
 */
void ln_err_dump(const char *fmt, ...)
{
     va_list ap;
     va_start(ap, fmt);
     err_doit(1, errno, fmt, ap);
     va_end(ap);
     abort();
/* dump core and terminate */
     exit(1);
/* shouldn’t get here */
}
