/*
 * Copyright (c) 2018-2019 Zhao Zhixu
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
#include <assert.h>
#include "ln_msg.h"
#include "ln_util.h"

#define MAX_ERROR_LENGTH 4096

#define BOLD(str) "\e[1m"str"\e[0m"
#define GREY(str) "\e[2m"str"\e[0m"
#define RED(str) "\e[31m"str"\e[0m"
#define GREEN(str) "\e[32m"str"\e[0m"
#define BROWN(str) "\e[33m"str"\e[0m"
#define BLUE(str) "\e[34m"str"\e[0m"
#define PURPLE(str) "\e[35m"str"\e[0m"
#define CYAN(str) "\e[36m"str"\e[0m"

static int32_t disp_mask = 0;

void ln_msg_init(ln_option *option)
{
    if (option->Wwarn) {
        disp_mask |= 1 << LN_MSG_WARN;
        disp_mask |= 1 << LN_MSG_WARN_SYS;
        if (option->Winter) {
            disp_mask |= 1 << LN_MSG_INTER_WARN;
            disp_mask |= 1 << LN_MSG_INTER_WARN_SYS;
        }
    }
    if (option->debug) {
        disp_mask |= 1 << LN_MSG_DEBUG;
    }
}

void ln_msg_cleanup(void)
{
    disp_mask = 0;
}

ln_msg *ln_msg_create(ln_msg_level level, const char *fmt, ...)
{
    ln_msg *msg;
    va_list ap;
    int errsv;

    errsv = errno;		/* first save errno to prevent overriding */
    msg = ln_alloc(sizeof(ln_msg));
    msg->level = level;
    msg->err_str = ln_alloc(sizeof(char)*MAX_ERROR_LENGTH);

    va_start(ap, fmt);
    vsnprintf(msg->err_str, MAX_ERROR_LENGTH-1, fmt, ap);
    va_end(ap);

    if (level == LN_MSG_ERROR_SYS || level == LN_MSG_WARN_SYS ||
        level == LN_MSG_INTER_ERROR_SYS || level == LN_MSG_INTER_WARN_SYS)
        snprintf(msg->err_str+strlen(msg->err_str),
                 MAX_ERROR_LENGTH-strlen(msg->err_str)-1, ": %s",
                 strerror(errsv));

    return msg;
}

void ln_msg_free(ln_msg *msg)
{
    ln_free(msg->err_str);
    ln_free(msg);
}

void ln_msg_handle(ln_msg **msg)
{
    if (!*msg)
        return;

    fflush(NULL);
    switch ((*msg)->level) {
    case LN_MSG_ERROR:
    case LN_MSG_ERROR_SYS:
        fprintf(stderr, RED("error: ")"%s\n", (*msg)->err_str);
        fflush(NULL);
        /* exit(EXIT_FAILURE); */
        abort();
        break;
    case LN_MSG_INTER_ERROR:
    case LN_MSG_INTER_ERROR_SYS:
        fprintf(stderr, RED("internal error: ")"%s\n", (*msg)->err_str);
        fflush(NULL);
        abort();
        break;
    case LN_MSG_WARN:
        if (!(disp_mask & 1 << LN_MSG_WARN))
            break;
    case LN_MSG_WARN_SYS:
        if (!(disp_mask & 1 << LN_MSG_WARN_SYS))
            break;
        fprintf(stderr, PURPLE("warning: ")"%s\n", (*msg)->err_str);
        fflush(NULL);
        break;
    case LN_MSG_INTER_WARN:
        if (!(disp_mask & 1 << LN_MSG_INTER_WARN))
            break;
    case LN_MSG_INTER_WARN_SYS:
        if (!(disp_mask & 1 << LN_MSG_INTER_WARN_SYS))
            break;
        fprintf(stderr, PURPLE("internal warning: ")"%s\n", (*msg)->err_str);
        fflush(NULL);
        break;
    case LN_MSG_INFO:
        fprintf(stderr, BOLD("info: ")"%s\n", (*msg)->err_str);
        fflush(NULL);
        break;
    case LN_MSG_DEBUG:
        if (!(disp_mask & 1 << LN_MSG_DEBUG))
            break;
        fprintf(stderr, GREY("debug: ")"%s\n", (*msg)->err_str);
        fflush(NULL);
        break;
    default :
        assert(0 && "shouldn't get here, unsupported ln_msg_level");
        break;
    }
    ln_msg_free(*msg);
    *msg = NULL;
}
