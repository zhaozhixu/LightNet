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

#ifndef _LN_CUDNN_H_
#define _LN_CUDNN_H_
#ifdef LN_CUDNN

#include <cudnn.h>

struct ln_cudnn_context {
    cudnnHandle_t cudnn_handle;
};
typedef struct ln_cudnn_context ln_cudnn_context;

#define LN_CUDNN_CK(status)                             \
    do {                                                \
        if (status != CUDNN_STATUS_SUCCESS)             \
            ln_err_bt("CUDNN_ERROR(%d): %s", status,    \
                      cudnnGetErrorString(status));     \
    } while(0)

#ifdef __cplusplus
LN_CPPSTART
#endif

ln_cudnn_context *ln_cudnn_context_create(void);
void ln_cudnn_context_free(ln_cudnn_context *context);
cudnnDataType_t ln_cudnn_datatype(tl_dtype dtype);
cudnnTensorDescriptor_t ln_cudnn_tensor_nchw_init(tl_tensor *tensor);
void ln_cudnn_tensor_cleanup(cudnnTensorDescriptor_t desc);

#ifdef __cplusplus
LN_CPPEND
#endif

#endif  /* LN_CUDNN */
#endif  /* _LN_CUDNN_H_ */
