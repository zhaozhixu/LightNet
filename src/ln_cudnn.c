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

#include "ln_util.h"
#include "ln_tensor.h"
#include "ln_msg.h"
#include "ln_cudnn.h"

ln_cudnn_context *ln_cudnn_context_create(void)
{
    ln_cudnn_context *context;

    context = ln_alloc(sizeof(ln_cudnn_context));
    LN_CUDNN_CK(cudnnCreate(&context->cudnn_handle));

    return context;
}

void ln_cudnn_context_free(ln_cudnn_context *context)
{
    LN_CUDNN_CK(cudnnDestroy(context->cudnn_handle));
    ln_free(context);
}

cudnnDataType_t ln_cudnn_datatype(tl_dtype dtype)
{
    cudnnDataType_t ret;

    switch (dtype) {
    case TL_DOUBLE:
        ret = CUDNN_DATA_DOUBLE;
        break;
    case TL_FLOAT:
        ret = CUDNN_DATA_FLOAT;
        break;
    case TL_INT32:
        ret = CUDNN_DATA_INT32;
        break;
    case TL_INT16:
        ln_msg_error("unsupported tl_dtype for cudnn %s", tl_dtype_name(dtype));
        break;
    case TL_INT8:
        ret = CUDNN_DATA_INT8;
        break;
    case TL_UINT32:
        ln_msg_error("unsupported tl_dtype for cudnn %s", tl_dtype_name(dtype));
        break;
    case TL_UINT16:
        ln_msg_error("unsupported tl_dtype for cudnn %s", tl_dtype_name(dtype));
        break;
    case TL_UINT8:
        ln_msg_error("unsupported tl_dtype for cudnn %s", tl_dtype_name(dtype));
        break;
    case TL_BOOL:
        ln_msg_error("unsupported tl_dtype for cudnn %s", tl_dtype_name(dtype));
        break;
    default:
        ln_msg_error("unsupported tl_dtype %d", dtype);
        break;
    }

    return ret;
}

cudnnTensorDescriptor_t ln_cudnn_tensor_nchw_init(tl_tensor *tensor)
{
    cudnnTensorDescriptor_t desc;

    LN_CUDNN_CK(cudnnCreateTensorDescriptor(&desc));
    LN_CUDNN_CK(cudnnSetTensor4dDescriptor(desc, CUDNN_TENSOR_NCHW,
                                           ln_cudnn_datatype(tensor->dtype),
                                           tensor->dims[0], tensor->dims[1],
                                           tensor->dims[2], tensor->dims[3]));
    return desc;
}

void ln_cudnn_tensor_cleanup(cudnnTensorDescriptor_t desc)
{
    LN_CUDNN_CK(cudnnDestroyTensorDescriptor(desc));
}
