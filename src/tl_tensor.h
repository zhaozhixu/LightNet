#ifndef _TL_TENSOR_H_
#define _TL_TENSOR_H_

#include "tl_type.h"

typedef struct tl_tensor tl_tensor;
struct tl_tensor {
     tl_dtype  dtype;
     int       len;
     int       ndim;
     int      *dims;
     void     *data;
};

#ifdef __cplusplus
extern "C" {
#endif

int tl_tensor_issameshape(const tl_tensor *t1, const tl_tensor *t2);
tl_tensor *tl_tensor_create(void *data, int ndim, const int *dims,
                                 tl_dtype dtype);
void tl_tensor_free(tl_tensor *t, int do_free_data);
tl_tensor *tl_tensor_zeros(tl_dtype dtype, int ndim, ...);
tl_tensor *tl_tensor_vcreate(tl_dtype dtype, int ndim, ...);
tl_tensor *tl_tensor_clone(const tl_tensor *src);
void tl_tensor_fprint(FILE *stream, const tl_tensor *t, const char *fmt);
void tl_tensor_print(const tl_tensor *t, const char *fmt);
int tl_tensor_save(const char *file_name, const tl_tensor *t, const char *fmt);
tl_tensor *tl_tensor_create_slice(const tl_tensor *src, int axis, int len,
                                       tl_dtype dtype);
tl_tensor *tl_tensor_slice(const tl_tensor *src, tl_tensor *dst, int axis,
                                int start, int len);
tl_tensor *tl_tensor_reshape(const tl_tensor *src, int ndim, const int *dims);
tl_tensor *tl_tensor_vreshape(const tl_tensor *src, int ndim, ...);
tl_tensor *tl_tensor_maxreduce(const tl_tensor *src, tl_tensor *dst,
                                    tl_tensor *arg, int axis);
tl_tensor *tl_tensor_mul(const tl_tensor *src1, const tl_tensor *src2,
                              tl_tensor *dst);
tl_tensor *tl_tensor_transpose(const tl_tensor *src, tl_tensor *dst,
                                    const int *axes, int **workspace);

#ifdef __cplusplus
}
#endif

#endif  /* _TL_TENSOR_H_ */
