#include <assert.h>
#include "tl_util.h"
#include "tl_type.h"

/* TODO: maybe platform dependent */
static const size_t dtype_size[TL_DTYPE_SIZE] = {
     4, 4, 2, 1, 4, 2, 1, 4
};

static const char *dtype_fmt[TL_DTYPE_SIZE] = {
     "%.3f", "%d", "%d", "%d", "%u", "%u", "%u", "%d"
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

const char *tl_fmt(tl_dtype dtype)
{
     check_dtype(dtype);
     return dtype_fmt[dtype];
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
     return (fprintf_func[dtype])(fp, fmt, p);
}

tl_fprintf_func tl_fprintf_getfunc(tl_dtype dtype)
{
     check_dtype(dtype);
     return fprintf_func[dtype];
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
     return (cmp_func[dtype])(p1, p2);
}

tl_cmp_func tl_cmp_getfunc(tl_dtype dtype)
{
     check_dtype(dtype);
     return cmp_func[dtype];
}

static void mul_float(void *p1, void *p2, void *r)
{
     *(float *)r = *(float *)p1 * *(float *)p2;
}

static void mul_int32(void *p1, void *p2, void *r)
{
     *(int32_t *)r = *(int32_t *)p1 * *(int32_t *)p2;
}

static void mul_int16(void *p1, void *p2, void *r)
{
     *(int16_t *)r = *(int16_t *)p1 * *(int16_t *)p2;
}

static void mul_int8(void *p1, void *p2, void *r)
{
     *(int8_t *)r = *(int8_t *)p1 * *(int8_t *)p2;
}

static void mul_uint32(void *p1, void *p2, void *r)
{
     *(uint32_t *)r = *(uint32_t *)p1 * *(uint32_t *)p2;
}

static void mul_uint16(void *p1, void *p2, void *r)
{
     *(uint16_t *)r = *(uint16_t *)p1 * *(uint16_t *)p2;
}

static void mul_uint8(void *p1, void *p2, void *r)
{
     *(uint8_t *)r = *(uint8_t *)p1 * *(uint8_t *)p2;
}

static void mul_bool(void *p1, void *p2, void *r)
{
     *(tl_bool_t *)r = *(tl_bool_t *)p1 * *(tl_bool_t *)p2;
}

static tl_mul_func mul_func[TL_DTYPE_SIZE] = {
     mul_float,
     mul_int32,
     mul_int16,
     mul_int8,
     mul_uint32,
     mul_uint16,
     mul_uint8,
     mul_bool
};

void tl_mul(void *p1, void *p2, void *r, tl_dtype dtype)
{
     check_dtype(dtype);
     (mul_func[dtype])(p1, p2, r);
}

tl_mul_func tl_mul_getfunc(tl_dtype dtype)
{
     check_dtype(dtype);
     return mul_func[dtype];
}
