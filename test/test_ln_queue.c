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

#include "test_lightnet.h"
#include "../src/ln_queue.h"

static int *data;
static size_t data_len;

static void checked_setup(void)
{
     int i;
     data_len = 3;
     data = ln_alloc(sizeof(int) * data_len);
     for (i = 0; i < data_len; i++) {
          data[i] = i;
     }
}

static void checked_teardown(void)
{
     ln_free(data);
}

START_TEST(test_queue_create)
{
     ln_queue *q;

     q = ln_queue_create();
     ck_assert_int_eq(q->size, 0);
     ck_assert_ptr_eq(q->head, NULL);
     ck_assert_ptr_eq(q->tail, NULL);
     ln_queue_free(q);
}
END_TEST

START_TEST(test_queue_enqueue_dequeue)
{
     ln_queue *q;

     q = ln_queue_enqueue(NULL, &data[0]);
     q = ln_queue_enqueue(q, &data[1]);
     q = ln_queue_enqueue(q, &data[2]);
     ck_assert_int_eq(q->size, 3);
     ck_assert_int_eq(*(int *)ln_queue_dequeue(q), 0);
     ck_assert_int_eq(*(int *)ln_queue_dequeue(q), 1);
     ck_assert_int_eq(*(int *)ln_queue_dequeue(q), 2);
     ck_assert_ptr_eq(ln_queue_dequeue(q), NULL);
     ck_assert_int_eq(q->size, 0);

     ln_queue_free(q);
}
END_TEST
/* end of tests */

Suite *make_queue_suite(void)
{
     Suite *s;
     TCase *tc_queue;

     s = suite_create("queue");
     tc_queue = tcase_create("queue");
     tcase_add_checked_fixture(tc_queue, checked_setup, checked_teardown);

     tcase_add_test(tc_queue, test_queue_create);
     tcase_add_test(tc_queue, test_queue_enqueue_dequeue);
     /* end of adding tests */

     suite_add_tcase(s, tc_queue);

     return s;
}
