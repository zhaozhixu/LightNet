/*
 * Copyright (c) 2018-2020 Zhao Zhixu
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

#ifndef _LN_QUEUE_H_
#define _LN_QUEUE_H_

#include <stdlib.h>
#include "ln_list.h"

struct ln_queue {
    ln_list  *head;
    ln_list  *tail;
    size_t    size;
};
typedef struct ln_queue ln_queue;

#ifdef __cplusplus
LN_CPPSTART
#endif

ln_queue *ln_queue_create(void);
void ln_queue_free(ln_queue *queue);
ln_queue *ln_queue_enqueue(ln_queue *queue, void *data);
void *ln_queue_dequeue(ln_queue *queue);

#ifdef __cplusplus
LN_CPPEND
#endif

#endif	/* _LN_QUEUE_H_ */
