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

#ifndef _TEST_LIGHTNET_H_
#define _TEST_LIGHTNET_H_

#include <stdio.h>
#include <check.h>

#ifndef ck_assert_float_eq_tol
#define ck_assert_float_eq_tol(X, Y, T)					\
     do {								\
	  float _ck_x = (X);						\
	  float _ck_y = (Y);						\
	  float _ck_t = (T);						\
	  ck_assert_msg(_ck_x - _ck_y < _ck_t,				\
			"Assertion '%s' failed: %s == %f, %s == %f, %s == %f", \
			#X" == "#Y, #X, _ck_x, #Y, _ck_y, #T, _ck_t);	\
     } while(0)
#endif

#ifndef ck_assert_array_int_eq
#define _ck_assert_array_int_eq(XA, YA, N)				\
     do {								\
	  int _ck_n = (N);						\
	  for (int i = 0; i < _ck_n; i++) {				\
	       intmax_t _ck_x = (XA)[i];                                \
	       intmax_t _ck_y = (YA)[i];                                \
	       if (_ck_x != _ck_x)					\
		    ck_assert_msg(0,					\
				  "Assertion 'array "#XA" == "#YA"' failed: "#XA"[%d] == %d, "#YA"[%d] == %d", \
				  i, _ck_x, i, _ck_y);			\
	  }								\
     } while(0)

static inline void ck_assert_array_int_eq(int *XA, int *YA, int N)
{
     _ck_assert_array_int_eq(XA, YA, N);
}
#endif

#ifndef ck_assert_array_uint_eq
#define _ck_assert_array_uint_eq(XA, YA, N)				\
     do {								\
	  int _ck_n = (N);						\
	  for (int i = 0; i < _ck_n; i++) {				\
	       uintmax_t _ck_x = (XA)[i];				\
	       uintmax_t _ck_y = (XA)[i];				\
	       if (_ck_x != _ck_y)					\
		    ck_assert_msg(0,					\
				  "Assertion 'array "#XA" == "#YA"' failed: "#XA"[%d] == %ud, "#YA"[%d] == %ud", \
				  i, _ck_x, i, _ck_y);			\
	  }								\
     } while(0)

static inline void ck_assert_array_uint_eq(unsigned int *XA, unsigned int *YA, int N)
{
     _ck_assert_array_uint_eq(XA, YA, N);
}
#endif

#ifndef ck_assert_array_float_eq_tol
#define _ck_assert_array_float_eq_tol(XA, YA, N, T)			\
     do {								\
	  int _ck_n = (N);						\
	  float _ck_t = (T);						\
	  for (int i = 0; i < _ck_n; i++) {				\
	       float _ck_x = (XA)[i];					\
	       float _ck_y = (XA)[i];					\
	       if (_ck_x - _ck_y >= _ck_t)				\
		    ck_assert_msg(0,					\
				  "Assertion 'array "#XA" == "#YA"' failed: "#XA"[%d] == %f, "#YA"[%d] == %f, "#T" == %f", \
				  i, _ck_x, i, _ck_y, _ck_t);		\
	  }								\
     } while(0)

static inline void ck_assert_array_float_eq_tol(float *XA, float *YA, int N, float T)
{
     _ck_assert_array_float_eq_tol(XA, YA, N, T);
}
#endif


#ifdef __cplusplus
#define CPPSTART extern "C" {
#define CPPEND }
#endif

Suite *make_master_suite(void);
Suite *make_list_suite(void);
Suite *make_error_suite(void);
Suite *make_param_suite(void);
Suite *make_tensor_suite(void);
Suite *make_op_suite(void);
Suite *make_parse_suite(void);
/* end of declarations */

#ifdef __cplusplus
CPPEND
#endif

#endif /* _TEST_LIGHTNET_H_ */
