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

#ifndef _LN_ERROR_H_
#define _LN_ERROR_H_

typedef enum ln_error_level ln_error_level;
enum ln_error_level {
     LN_ERROR,
     LN_ERROR_SYS,
     LN_WARNING,
     LN_WARNING_SYS,
     LN_INFO
};

typedef struct ln_error ln_error;
struct ln_error {
     char            *err_str;
     ln_error_level   level;
};

#define ln_error_emit(level, fmt, varg...)                              \
     do {                                                               \
          ln_error *err = ln_error_create((level), (fmt), ##varg);      \
          ln_error_handle(&err);                                        \
          ln_error_free(err);                                           \
     } while (0)

#ifdef __cplusplus
LN_CPPSTART
#endif

ln_error *ln_error_create(ln_error_level level, const char *fmt, ...);
void ln_error_free(ln_error *error);
void ln_error_handle(ln_error **error);

#ifdef __cplusplus
LN_CPPEND
#endif

#endif	/* _LN_ERROR_H_ */
