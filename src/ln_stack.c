/*
 * Copyright (c) 2018-2020 Zhixu Zhao
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

#include "ln_stack.h"

ln_stack *ln_stack_create(void)
{
    ln_stack *s;

    s = (ln_stack *)ln_alloc(sizeof(ln_stack));
    s->top = NULL;
    s->size = 0;

    return s;
}

void ln_stack_free(ln_stack *stack)
{
    ln_list_free(stack->top);
    ln_free(stack);
}

/* return the stack with a new element (a new stack if stack == NULL) */
ln_stack *ln_stack_push(ln_stack *stack, void *data)
{
    ln_stack *s;

    if (!stack) {
        s = ln_stack_create();
        s->top = ln_list_append(NULL, data);
        s->size = 1;
        return s;
    }

    s = stack;
    s->top = ln_list_prepend(s->top, data);
    s->size++;

    return s;
}

void *ln_stack_pop(ln_stack *stack)
{
    void *data;

    data = ln_list_nth_data(stack->top, 0);
    stack->top = ln_list_remove_nth(stack->top, 0);
    if (stack->size > 0)
        stack->size--;

    return data;
}
