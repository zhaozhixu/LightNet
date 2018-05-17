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

#include <math.h>
#include <limits.h>
#include <assert.h>
#include "tl_util.h"
#include "tl_type.h"

#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))

static const size_t dtype_size[TL_DTYPE_SIZE] = {
     sizeof(double),
     sizeof(float),
     sizeof(int32_t),
     sizeof(int16_t),
     sizeof(int8_t),
     sizeof(uint32_t),
     sizeof(uint16_t),
     sizeof(uint8_t),
     sizeof(tl_bool_t)
};

static const char *dtype_fmt[TL_DTYPE_SIZE] = {
     "%.3f", "%.3f", "%d", "%d", "%d", "%u", "%u", "%u", "%d"
};

static inline void check_dtype(tl_dtype dtype)
{
     assert(dtype >= 0 && dtype < TL_DTYPE_SIZE);
}

size_t tl_size_of(tl_dtype dtype)
{
     check_dtype(dtype);
     return dtype_size[dtype];
}

/* tl_fprintf_func */
const char *tl_fmt(tl_dtype dtype)
{
     check_dtype(dtype);
     return dtype_fmt[dtype];
}

static int fprintf_double(FILE *fp, const char *fmt, void *p)
{
     if (!fmt)
          return fprintf(fp, dtype_fmt[TL_DOUBLE], *(double *)p);
     else
          return fprintf(fp, fmt, *(double *)p);
}

static int fprintf_float(FILE *fp, const char *fmt, void *p)
{
     if (!fmt)
          return fprintf(fp, dtype_fmt[TL_FLOAT], *(float *)p);
     else
          return fprintf(fp, fmt, *(float *)p);
}

static int fprintf_int32(FILE *fp, const char *fmt, void *p)
{
     if (!fmt)
          return fprintf(fp, dtype_fmt[TL_INT32], *(int32_t *)p);
     else
          return fprintf(fp, fmt, *(int32_t *)p);
}

static int fprintf_int16(FILE *fp, const char *fmt, void *p)
{
     if (!fmt)
          return fprintf(fp, dtype_fmt[TL_INT16], *(int16_t *)p);
     else
          return fprintf(fp, fmt, *(int16_t *)p);
}

static int fprintf_int8(FILE *fp, const char *fmt, void *p)
{
     if (!fmt)
          return fprintf(fp, dtype_fmt[TL_INT8], *(int8_t *)p);
     else
          return fprintf(fp, fmt, *(int8_t *)p);
}

static int fprintf_uint32(FILE *fp, const char *fmt, void *p)
{
     if (!fmt)
          return fprintf(fp, dtype_fmt[TL_UINT32], *(uint32_t *)p);
     else
          return fprintf(fp, fmt, *(uint16_t *)p);
}

static int fprintf_uint16(FILE *fp, const char *fmt, void *p)
{
     if (!fmt)
          return fprintf(fp, dtype_fmt[TL_UINT16], *(uint16_t *)p);
     else
          return fprintf(fp, fmt, *(uint16_t *)p);
}

static int fprintf_uint8(FILE *fp, const char *fmt, void *p)
{
     if (!fmt)
          return fprintf(fp, dtype_fmt[TL_UINT8], *(uint8_t *)p);
     else
          return fprintf(fp, fmt, *(uint8_t *)p);
}

static int fprintf_bool(FILE *fp, const char *fmt, void *p)
{
     if (!fmt)
          return fprintf(fp, dtype_fmt[TL_BOOL], *(tl_bool_t *)p);
     else
          return fprintf(fp, fmt, *(tl_bool_t *)p);
}

static tl_fprintf_func fprintf_func[TL_DTYPE_SIZE] = {
     fprintf_double,
     fprintf_float,
     fprintf_int32,
     fprintf_int16,
     fprintf_int8,
     fprintf_uint32,
     fprintf_uint16,
     fprintf_uint8,
     fprintf_bool
};

int tl_fprintf(FILE* fp, const char* fmt, void* p, tl_dtype dtype)
{
     check_dtype(dtype);
     return fprintf_func[dtype](fp, fmt, p);
}

tl_fprintf_func tl_fprintf_getfunc(tl_dtype dtype)
{
     check_dtype(dtype);
     return fprintf_func[dtype];
}

/* tl_cmp_func */
static int cmp_double(void *p1, void *p2)
{
     return *(double *)p1 - *(double *)p2;
}

static int cmp_float(void *p1, void *p2)
{
     return *(float *)p1 - *(float *)p2;
}

static int cmp_int32(void *p1, void *p2)
{
     return *(int32_t *)p1 - *(int32_t *)p2;
}

static int cmp_int16(void *p1, void *p2)
{
     return *(int16_t *)p1 - *(int16_t *)p2;
}

static int cmp_int8(void *p1, void *p2)
{
     return *(int8_t *)p1 - *(int8_t *)p2;
}

static int cmp_uint32(void *p1, void *p2)
{
     return *(uint32_t *)p1 - *(uint32_t *)p2;
}

static int cmp_uint16(void *p1, void *p2)
{
     return *(uint16_t *)p1 - *(uint16_t *)p2;
}

static int cmp_uint8(void *p1, void *p2)
{
     return *(uint8_t *)p1 - *(uint8_t *)p2;
}

static int cmp_bool(void *p1, void *p2)
{
     return *(tl_bool_t *)p1 - *(tl_bool_t *)p2;
}

static tl_cmp_func cmp_func[TL_DTYPE_SIZE] = {
     cmp_double,
     cmp_float,
     cmp_int32,
     cmp_int16,
     cmp_int8,
     cmp_uint32,
     cmp_uint16,
     cmp_uint8,
     cmp_bool
};

int tl_cmp(void *p1, void *p2, tl_dtype dtype)
{
     check_dtype(dtype);
     return cmp_func[dtype](p1, p2);
}

tl_cmp_func tl_cmp_getfunc(tl_dtype dtype)
{
     check_dtype(dtype);
     return cmp_func[dtype];
}

/* tl_elew_func */
static inline void check_elew_op(tl_elew_op op)
{
     assert(op >= 0 && op < TL_ELEW_OP_SIZE);
}

typedef void (*elew_op_func) (void *p1, void *p2, void *r);

static void mul_double(void *p1, void *p2, void *r)
{
     *(double *)r = *(double *)p1 * *(double *)p2;
}

static void div_double(void *p1, void *p2, void *r)
{
     assert(*(double *)p2);
     *(double *)r = *(double *)p1 / *(double *)p2;
}

static void sum_double(void *p1, void *p2, void *r)
{
     *(double *)r = *(double *)p1 + *(double *)p2;
}

static void sub_double(void *p1, void *p2, void *r)
{
     *(double *)r = *(double *)p1 - *(double *)p2;
}

static void max_double(void *p1, void *p2, void *r)
{
     *(double *)r = max(*(double *)p1, *(double *)p2);
}

static void min_double(void *p1, void *p2, void *r)
{
     *(double *)r = min(*(double *)p1, *(double *)p2);
}

static void pow_double(void *p1, void *p2, void *r)
{
     *(double *)r = powf(*(double *)p1, *(double *)p2);
}

static elew_op_func elew_op_double[TL_ELEW_OP_SIZE] = {
     mul_double,
     div_double,
     sum_double,
     sub_double,
     max_double,
     min_double,
     pow_double
};

static void elew_double(void *p1, void *p2, void *r, tl_elew_op elew_op)
{
     check_elew_op(elew_op);
     elew_op_double[elew_op](p1, p2, r);
}

static void mul_float(void *p1, void *p2, void *r)
{
     *(float *)r = *(float *)p1 * *(float *)p2;
}

static void div_float(void *p1, void *p2, void *r)
{
     assert(*(float *)p2);
     *(float *)r = *(float *)p1 / *(float *)p2;
}

static void sum_float(void *p1, void *p2, void *r)
{
     *(float *)r = *(float *)p1 + *(float *)p2;
}

static void sub_float(void *p1, void *p2, void *r)
{
     *(float *)r = *(float *)p1 - *(float *)p2;
}

static void max_float(void *p1, void *p2, void *r)
{
     *(float *)r = max(*(float *)p1, *(float *)p2);
}

static void min_float(void *p1, void *p2, void *r)
{
     *(float *)r = min(*(float *)p1, *(float *)p2);
}

static void pow_float(void *p1, void *p2, void *r)
{
     *(float *)r = powf(*(float *)p1, *(float *)p2);
}

static elew_op_func elew_op_float[TL_ELEW_OP_SIZE] = {
     mul_float,
     div_float,
     sum_float,
     sub_float,
     max_float,
     min_float,
     pow_float
};

static void elew_float(void *p1, void *p2, void *r, tl_elew_op elew_op)
{
     check_elew_op(elew_op);
     elew_op_float[elew_op](p1, p2, r);
}

static void mul_int32(void *p1, void *p2, void *r)
{
     *(int32_t *)r = *(int32_t *)p1 * *(int32_t *)p2;
}

static void div_int32(void *p1, void *p2, void *r)
{
     assert(*(int32_t *)p2);
     *(int32_t *)r = *(int32_t *)p1 / *(int32_t *)p2;
}

static void sum_int32(void *p1, void *p2, void *r)
{
     *(int32_t *)r = *(int32_t *)p1 + *(int32_t *)p2;
}

static void sub_int32(void *p1, void *p2, void *r)
{
     *(int32_t *)r = *(int32_t *)p1 - *(int32_t *)p2;
}

static void max_int32(void *p1, void *p2, void *r)
{
     *(int32_t *)r = max(*(int32_t *)p1, *(int32_t *)p2);
}

static void min_int32(void *p1, void *p2, void *r)
{
     *(int32_t *)r = min(*(int32_t *)p1, *(int32_t *)p2);
}

static void pow_int32(void *p1, void *p2, void *r)
{
     double d1, d2, dr;

     d1 = (double)*(int32_t *)p1;
     d2 = (double)*(int32_t *)p2;
     dr = pow(d1, d2);
     if (dr >= INT32_MAX)
          *(int32_t *)r = INT32_MAX;
     else if (dr <= INT32_MIN)
          *(int32_t *)r = INT32_MIN;
     else
          *(int32_t *)r = (int32_t)dr;
}

static elew_op_func elew_op_int32[TL_ELEW_OP_SIZE] = {
     mul_int32,
     div_int32,
     sum_int32,
     sub_int32,
     max_int32,
     min_int32,
     pow_int32
};

static void elew_int32(void *p1, void *p2, void *r, tl_elew_op elew_op)
{
     check_elew_op(elew_op);
     elew_op_int32[elew_op](p1, p2, r);
}

static void mul_int16(void *p1, void *p2, void *r)
{
     *(int16_t *)r = *(int16_t *)p1 * *(int16_t *)p2;
}

static void div_int16(void *p1, void *p2, void *r)
{
     assert(*(int16_t *)p2);
     *(int16_t *)r = *(int16_t *)p1 / *(int16_t *)p2;
}

static void sum_int16(void *p1, void *p2, void *r)
{
     *(int16_t *)r = *(int16_t *)p1 + *(int16_t *)p2;
}

static void sub_int16(void *p1, void *p2, void *r)
{
     *(int16_t *)r = *(int16_t *)p1 - *(int16_t *)p2;
}

static void max_int16(void *p1, void *p2, void *r)
{
     *(int16_t *)r = max(*(int16_t *)p1, *(int16_t *)p2);
}

static void min_int16(void *p1, void *p2, void *r)
{
     *(int16_t *)r = min(*(int16_t *)p1, *(int16_t *)p2);
}

static void pow_int16(void *p1, void *p2, void *r)
{
     double d1, d2, dr;

     d1 = (double)*(int16_t *)p1;
     d2 = (double)*(int16_t *)p2;
     dr = pow(d1, d2);
     if (dr >= INT16_MAX)
          *(int16_t *)r = INT16_MAX;
     else if (dr <= INT16_MIN)
          *(int16_t *)r = INT16_MIN;
     else
          *(int16_t *)r = (int16_t)dr;
}

static elew_op_func elew_op_int16[TL_ELEW_OP_SIZE] = {
     mul_int16,
     div_int16,
     sum_int16,
     sub_int16,
     max_int16,
     min_int16,
     pow_int16
};

static void elew_int16(void *p1, void *p2, void *r, tl_elew_op elew_op)
{
     check_elew_op(elew_op);
     elew_op_int16[elew_op](p1, p2, r);
}

static void mul_int8(void *p1, void *p2, void *r)
{
     *(int8_t *)r = *(int8_t *)p1 * *(int8_t *)p2;
}

static void div_int8(void *p1, void *p2, void *r)
{
     assert(*(int8_t *)p2);
     *(int8_t *)r = *(int8_t *)p1 / *(int8_t *)p2;
}

static void sum_int8(void *p1, void *p2, void *r)
{
     *(int8_t *)r = *(int8_t *)p1 + *(int8_t *)p2;
}

static void sub_int8(void *p1, void *p2, void *r)
{
     *(int8_t *)r = *(int8_t *)p1 - *(int8_t *)p2;
}

static void max_int8(void *p1, void *p2, void *r)
{
     *(int8_t *)r = max(*(int8_t *)p1, *(int8_t *)p2);
}

static void min_int8(void *p1, void *p2, void *r)
{
     *(int8_t *)r = min(*(int8_t *)p1, *(int8_t *)p2);
}

static void pow_int8(void *p1, void *p2, void *r)
{
     double d1, d2, dr;

     d1 = (double)*(int8_t *)p1;
     d2 = (double)*(int8_t *)p2;
     dr = pow(d1, d2);
     if (dr >= INT8_MAX)
          *(int8_t *)r = INT8_MAX;
     else if (dr <= INT8_MIN)
          *(int8_t *)r = INT8_MIN;
     else
          *(int8_t *)r = (int8_t)dr;
}

static elew_op_func elew_op_int8[TL_ELEW_OP_SIZE] = {
     mul_int8,
     div_int8,
     sum_int8,
     sub_int8,
     max_int8,
     min_int8,
     pow_int8
};

static void elew_int8(void *p1, void *p2, void *r, tl_elew_op elew_op)
{
     check_elew_op(elew_op);
     elew_op_int8[elew_op](p1, p2, r);
}

static void mul_uint32(void *p1, void *p2, void *r)
{
     *(uint32_t *)r = *(uint32_t *)p1 * *(uint32_t *)p2;
}

static void div_uint32(void *p1, void *p2, void *r)
{
     assert(*(uint32_t *)p2);
     *(uint32_t *)r = *(uint32_t *)p1 / *(uint32_t *)p2;
}

static void sum_uint32(void *p1, void *p2, void *r)
{
     *(uint32_t *)r = *(uint32_t *)p1 + *(uint32_t *)p2;
}

static void sub_uint32(void *p1, void *p2, void *r)
{
     *(uint32_t *)r = *(uint32_t *)p1 - *(uint32_t *)p2;
}

static void max_uint32(void *p1, void *p2, void *r)
{
     *(uint32_t *)r = max(*(uint32_t *)p1, *(uint32_t *)p2);
}

static void min_uint32(void *p1, void *p2, void *r)
{
     *(uint32_t *)r = min(*(uint32_t *)p1, *(uint32_t *)p2);
}

static void pow_uint32(void *p1, void *p2, void *r)
{
     double d1, d2, dr;

     d1 = (double)*(uint32_t *)p1;
     d2 = (double)*(uint32_t *)p2;
     dr = pow(d1, d2);
     if (dr >= UINT32_MAX)
          *(uint32_t *)r = UINT32_MAX;
     else
          *(uint32_t *)r = (uint32_t)dr;
}

static elew_op_func elew_op_uint32[TL_ELEW_OP_SIZE] = {
     mul_uint32,
     div_uint32,
     sum_uint32,
     sub_uint32,
     max_uint32,
     min_uint32,
     pow_uint32
};

static void elew_uint32(void *p1, void *p2, void *r, tl_elew_op elew_op)
{
     check_elew_op(elew_op);
     elew_op_uint32[elew_op](p1, p2, r);
}

static void mul_uint16(void *p1, void *p2, void *r)
{
     *(uint16_t *)r = *(uint16_t *)p1 * *(uint16_t *)p2;
}

static void div_uint16(void *p1, void *p2, void *r)
{
     assert(*(uint16_t *)p2);
     *(uint16_t *)r = *(uint16_t *)p1 / *(uint16_t *)p2;
}

static void sum_uint16(void *p1, void *p2, void *r)
{
     *(uint16_t *)r = *(uint16_t *)p1 + *(uint16_t *)p2;
}

static void sub_uint16(void *p1, void *p2, void *r)
{
     *(uint16_t *)r = *(uint16_t *)p1 - *(uint16_t *)p2;
}

static void max_uint16(void *p1, void *p2, void *r)
{
     *(uint16_t *)r = max(*(uint16_t *)p1, *(uint16_t *)p2);
}

static void min_uint16(void *p1, void *p2, void *r)
{
     *(uint16_t *)r = min(*(uint16_t *)p1, *(uint16_t *)p2);
}

static void pow_uint16(void *p1, void *p2, void *r)
{
     double d1, d2, dr;

     d1 = (double)*(uint16_t *)p1;
     d2 = (double)*(uint16_t *)p2;
     dr = pow(d1, d2);
     if (dr >= UINT16_MAX)
          *(uint16_t *)r = UINT16_MAX;
     else
          *(uint16_t *)r = (uint16_t)dr;
}

static elew_op_func elew_op_uint16[TL_ELEW_OP_SIZE] = {
     mul_uint16,
     div_uint16,
     sum_uint16,
     sub_uint16,
     max_uint16,
     min_uint16,
     pow_uint16
};

static void elew_uint16(void *p1, void *p2, void *r, tl_elew_op elew_op)
{
     check_elew_op(elew_op);
     elew_op_uint16[elew_op](p1, p2, r);
}

static void mul_uint8(void *p1, void *p2, void *r)
{
     *(uint8_t *)r = *(uint8_t *)p1 * *(uint8_t *)p2;
}

static void div_uint8(void *p1, void *p2, void *r)
{
     assert(*(uint8_t *)p2);
     *(uint8_t *)r = *(uint8_t *)p1 / *(uint8_t *)p2;
}

static void sum_uint8(void *p1, void *p2, void *r)
{
     *(uint8_t *)r = *(uint8_t *)p1 + *(uint8_t *)p2;
}

static void sub_uint8(void *p1, void *p2, void *r)
{
     *(uint8_t *)r = *(uint8_t *)p1 - *(uint8_t *)p2;
}

static void max_uint8(void *p1, void *p2, void *r)
{
     *(uint8_t *)r = max(*(uint8_t *)p1, *(uint8_t *)p2);
}

static void min_uint8(void *p1, void *p2, void *r)
{
     *(uint8_t *)r = min(*(uint8_t *)p1, *(uint8_t *)p2);
}

static void pow_uint8(void *p1, void *p2, void *r)
{
     double d1, d2, dr;

     d1 = (double)*(uint8_t *)p1;
     d2 = (double)*(uint8_t *)p2;
     dr = pow(d1, d2);
     if (dr >= UINT8_MAX)
          *(uint8_t *)r = UINT8_MAX;
     else
          *(uint8_t *)r = (uint8_t)dr;
}

static elew_op_func elew_op_uint8[TL_ELEW_OP_SIZE] = {
     mul_uint8,
     div_uint8,
     sum_uint8,
     sub_uint8,
     max_uint8,
     min_uint8,
     pow_uint8
};

static void elew_uint8(void *p1, void *p2, void *r, tl_elew_op elew_op)
{
     check_elew_op(elew_op);
     elew_op_uint8[elew_op](p1, p2, r);
}


static void mul_bool(void *p1, void *p2, void *r)
{
     *(tl_bool_t *)r = *(tl_bool_t *)p1 * *(tl_bool_t *)p2;
}

static void div_bool(void *p1, void *p2, void *r)
{
     assert(*(tl_bool_t *)p2);
     *(tl_bool_t *)r = *(tl_bool_t *)p1 / *(tl_bool_t *)p2;
}

static void sum_bool(void *p1, void *p2, void *r)
{
     *(tl_bool_t *)r = *(tl_bool_t *)p1 + *(tl_bool_t *)p2;
}

static void sub_bool(void *p1, void *p2, void *r)
{
     *(tl_bool_t *)r = *(tl_bool_t *)p1 - *(tl_bool_t *)p2;
}

static void max_bool(void *p1, void *p2, void *r)
{
     *(tl_bool_t *)r = max(*(tl_bool_t *)p1, *(tl_bool_t *)p2);
}

static void min_bool(void *p1, void *p2, void *r)
{
     *(tl_bool_t *)r = min(*(tl_bool_t *)p1, *(tl_bool_t *)p2);
}

static void pow_bool(void *p1, void *p2, void *r)
{
     double d1, d2, dr;

     d1 = (double)*(tl_bool_t *)p1;
     d2 = (double)*(tl_bool_t *)p2;
     dr = pow(d1, d2);
     if (dr >= INT_MAX)
          *(tl_bool_t *)r = INT_MAX;
     else
          *(tl_bool_t *)r = (tl_bool_t)dr;
}

static elew_op_func elew_op_bool[TL_ELEW_OP_SIZE] = {
     mul_bool,
     div_bool,
     sum_bool,
     sub_bool,
     max_bool,
     min_bool,
     pow_bool
};

static void elew_bool(void *p1, void *p2, void *r, tl_elew_op elew_op)
{
     check_elew_op(elew_op);
     elew_op_bool[elew_op](p1, p2, r);
}

static tl_elew_func elew_func[TL_DTYPE_SIZE] = {
     elew_double,
     elew_float,
     elew_int32,
     elew_int16,
     elew_int8,
     elew_uint32,
     elew_uint16,
     elew_uint8,
     elew_bool
};

void tl_elew(void *p1, void *p2, void *r, tl_elew_op elew_op, tl_dtype dtype)
{
     check_dtype(dtype);
     elew_func[dtype](p1, p2, r, elew_op);
}

tl_elew_func tl_elew_getfunc(tl_dtype dtype)
{
     check_dtype(dtype);
     return elew_func[dtype];
}
