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
#include <assert.h>
#include "ln_msg.h"
#include "ln_util.h"

#define MAX_ERROR_LENGTH 4096

static int32_t disp_mask = 0;

void ln_msg_init(ln_option *option)
{

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

    if (level == LN_ERROR_SYS || level == LN_WARNING_SYS ||
        level == LN_INTER_ERROR_SYS || level == LN_INTER_WARNING_SYS)
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
    case LN_ERROR:
    case LN_ERROR_SYS:
        fprintf(stderr, "ERROR: %s\n", (*msg)->err_str);
        fflush(NULL);
        exit(EXIT_FAILURE);
        break;
    case LN_INTER_ERROR:
    case LN_INTER_ERROR_SYS:
        fprintf(stderr, "INTERNAL ERROR: %s\n", (*msg)->err_str);
        fflush(NULL);
        abort();
        break;
    case LN_WARNING:
    case LN_WARNING_SYS:
        fprintf(stderr, "WARNING: %s\n", (*msg)->err_str);
        fflush(NULL);
        break;
    case LN_INTER_WARNING:
    case LN_INTER_WARNING_SYS:
        fprintf(stderr, "INTERNAL WARNING: %s\n", (*msg)->err_str);
        fflush(NULL);
        break;
    case LN_INFO:
        fprintf(stderr, "INFO: %s\n", (*msg)->err_str);
        fflush(NULL);
        break;
    case LN_DEBUG_INFO:
        fprintf(stderr, "DEBUG INFO: %s\n", (*msg)->err_str);
        fflush(NULL);
        break;
    default :
        assert(0 && "shouldn't get here, unsupported ln_msg_level");
        break;
    }
    ln_msg_free(*msg);
    *msg = NULL;
}
