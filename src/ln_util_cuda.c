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

#include <errno.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <cuda_runtime.h>

#include "ln_util.h"

struct ln_cuda_stream {
    cudaStream_t stream;
};
typedef struct ln_cuda_stream ln_cuda_stream;

#define LN_CUDA_CK(status)                                              \
    do {                                                                \
        if (status != cudaSuccess)                                      \
            ln_err_bt("CUDA_ERROR(%d) %s: %s", status,                  \
                      cudaGetErrorName(status), cudaGetErrorString(status)); \
    } while(0)

void ln_cuda_set_device(int n)
{
    assert(n <= LN_MAX_CUDA_DEVICE);
    LN_CUDA_CK(cudaSetDevice(n));
}

int ln_cuda_get_device()
{
    int n = 0;
    LN_CUDA_CK(cudaGetDevice(&n));
    return n;
}

int ln_is_device_mem(const void *ptr)
{
    assert(ptr);
    struct cudaPointerAttributes attributes;
    cudaError_t status;

    status = cudaPointerGetAttributes(&attributes, ptr);
    if (status == cudaErrorInvalidValue)
        return 0;
    LN_CUDA_CK(status);
    return attributes.memoryType == cudaMemoryTypeDevice;
}


void *ln_alloc_cuda(size_t size)
{
    void *p;

    assert(size > 0);
    LN_CUDA_CK(cudaMalloc(&p, size));
    assert(p);

    return p;
}

void ln_memset_cuda(void *dst, int c, size_t n)
{
    assert(ln_is_device_mem(dst));
    LN_CUDA_CK(cudaMemset(dst, c, n));
}

void ln_memcpy_h2d(void *dst, const void *src, size_t size)
{
    assert(!ln_is_device_mem(src));
    assert(ln_is_device_mem(dst));
    LN_CUDA_CK(cudaMemcpy(dst, src, size, cudaMemcpyHostToDevice));
}

void ln_memcpy_d2h(void *dst, const void *src, size_t size)
{
    assert(ln_is_device_mem(src));
    assert(!ln_is_device_mem(dst));
    LN_CUDA_CK(cudaMemcpy(dst, src, size, cudaMemcpyDeviceToHost));
}

void ln_memcpy_d2d(void *dst, const void *src, size_t size)
{
    assert(ln_is_device_mem(src));
    assert(ln_is_device_mem(dst));
    LN_CUDA_CK(cudaMemcpy(dst, src, size, cudaMemcpyDeviceToDevice));
}

void ln_free_cuda(void *p)
{
    assert(ln_is_device_mem(p));
    LN_CUDA_CK(cudaFree(p));
}

void *ln_clone_h2d(const void *src, size_t size)
{
    void *p;

    assert(!ln_is_device_mem(src));
    p = ln_alloc_cuda(size);
    LN_CUDA_CK(cudaMemcpy(p, src, size, cudaMemcpyHostToDevice));
    return p;
}

void *ln_clone_d2h(const void *src, size_t size)
{
    void *p;

    assert(ln_is_device_mem(src));
    p = ln_alloc(size);
    LN_CUDA_CK(cudaMemcpy(p, src, size, cudaMemcpyDeviceToHost));
    return p;
}

void *ln_clone_d2d(const void *src, size_t size)
{
    void *p;

    assert(ln_is_device_mem(src));
    p = ln_alloc_cuda(size);
    LN_CUDA_CK(cudaMemcpy(p, src, size, cudaMemcpyDeviceToDevice));
    return p;
}

void *ln_repeat_h2d(void *data, size_t size, int times)
{
    void *p, *dst;
    int i;

    assert(!ln_is_device_mem(data) && times > 0);
    dst = p = ln_alloc_cuda(size * times);
    for (i = 0; i < times; i++, p = (char *)p + size)
        LN_CUDA_CK(cudaMemcpy(p, data, size, cudaMemcpyHostToDevice));
    return dst;
}

void *ln_repeat_d2h(void *data, size_t size, int times)
{
    void *p, *dst;
    int i;

    assert(ln_is_device_mem(data) && times > 0);
    dst = p = ln_alloc(size * times);
    for (i = 0; i < times; i++, p = (char *)p + size)
        LN_CUDA_CK(cudaMemcpy(p, data, size, cudaMemcpyDeviceToHost));
    return dst;
}

void *ln_repeat_d2d(void *data, size_t size, int times)
{
    void *p, *dst;
    int i;

    assert(ln_is_device_mem(data) && times > 0);
    dst = p = ln_alloc_cuda(size * times);
    for (i = 0; i < times; i++, p = (char *)p + size)
        LN_CUDA_CK(cudaMemcpy(p, data, size, cudaMemcpyDeviceToDevice));
    return dst;
}

void ln_cuda_stream_create(ln_cuda_stream *pstream)
{
    LN_CUDA_CK(cudaStreamCreate(&pstream->stream));
}

void ln_cuda_stream_sync(ln_cuda_stream stream)
{
    LN_CUDA_CK(cudaStreamSynchronize(stream.stream));
}
