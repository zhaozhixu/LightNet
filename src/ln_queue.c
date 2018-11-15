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

#include "ln_queue.h"
#include "ln_util.h"

ln_queue *ln_queue_create(void)
{
     ln_queue *q;

     q = (ln_queue *)ln_alloc(sizeof(ln_queue));
     q->head = NULL;
     q->tail = NULL;
     q->size = 0;

     return q;
}

/* return the queue with a new element (a new queue if queue == NULL) */
ln_queue *ln_queue_enqueue(ln_queue *queue, void *data)
{
     ln_queue *q;

     if (!queue) {
          q = (ln_queue *)ln_alloc(sizeof(ln_queue));
          q->head = ln_list_append(NULL, data);
          q->tail = q->head;
          q->size = 1;
          return q;
     }

     q = queue;
     q->tail = ln_list_append(q->tail, data);
     q->size++;
     if (!q->head)
          q->head = q->tail;
     else
          q->tail = q->tail->next;

     return q;
}

void *ln_queue_dequeue(ln_queue *queue)
{
     void *data;

     data = ln_list_nth_data(queue->head, 0);
     queue->head = ln_list_remove_nth(queue->head, 0);
     if (queue->head == NULL)
          queue->tail = NULL;
     if (queue->size > 0)
          queue->size--;

     return data;
}

void ln_queue_free(ln_queue *queue)
{
     ln_list_free(queue->head);
     ln_free(queue);
}
