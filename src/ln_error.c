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

#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include "ln_error.h"
#include "ln_util.h"

#define MAX_ERROR_LENGTH 4096

ln_error *ln_error_create(ln_error_level level, const char *fmt, ...)
{
     ln_error *error;
     va_list ap;
     int errsv;

     errsv = errno;		/* first save errno to prevent overriding */
     error = ln_alloc(sizeof(ln_error));
     error->level = level;
     error->err_str = ln_alloc(sizeof(char)*MAX_ERROR_LENGTH);

     va_start(ap, fmt);
     vsnprintf(error->err_str, MAX_ERROR_LENGTH-1, fmt, ap);
     va_end(ap);

     if (level == LN_ERROR_SYS || level == LN_WARNING_SYS)
          snprintf(error->err_str+strlen(error->err_str),
                   MAX_ERROR_LENGTH-strlen(error->err_str)-1, ": %s",
                   strerror(errsv));

     return error;
}

void ln_error_free(ln_error *error)
{
     ln_free(error->err_str);
     ln_free(error);
}

void ln_error_handle(ln_error **error)
{
     if (!*error)
          return;

     fflush(stdout);
     switch ((*error)->level) {
     case LN_ERROR:
          fprintf(stderr, "ERROR: %s\n", (*error)->err_str);
          fflush(NULL);
          abort();
          break;
     case LN_ERROR_SYS:
          fprintf(stderr, "ERROR_SYS: %s\n", (*error)->err_str);
          fflush(NULL);
          abort();
          break;
     case LN_WARNING:
          fprintf(stderr, "WARNING: %s\n", (*error)->err_str);
          fflush(NULL);
          ln_error_free(*error);
          *error = NULL;
          return;
     case LN_WARNING_SYS:
          fprintf(stderr, "WARNING_SYS: %s\n", (*error)->err_str);
          fflush(NULL);
          ln_error_free(*error);
          *error = NULL;
          return;
     case LN_INFO:
          fprintf(stderr, "INFO: %s\n", (*error)->err_str);
          fflush(NULL);
          ln_error_free(*error);
          *error = NULL;
          return;
     default :
          assert(0 && "shouldn't get here, unsupported ln_error_level");
          return;
     }
}
