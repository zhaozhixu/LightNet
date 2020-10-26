from onnx import TensorProto
import warnings

TENSOR_TYPE_TO_TL_TYPE = {
    int(TensorProto.FLOAT): 'TL_FLOAT',
    int(TensorProto.UINT8): 'TL_UINT8',
    int(TensorProto.INT8): 'TL_INT8',
    int(TensorProto.UINT16): 'TL_UINT16',
    int(TensorProto.INT16): 'TL_INT16',
    int(TensorProto.INT32): 'TL_INT32',
    int(TensorProto.INT64): 'TL_INT64',
    int(TensorProto.BOOL): 'TL_BOOL',
    int(TensorProto.FLOAT16): 'TL_DTYPE_INVALID',
    int(TensorProto.DOUBLE): 'TL_DOUBLE',
    int(TensorProto.COMPLEX64): 'TL_DTYPE_INVALID',
    int(TensorProto.COMPLEX128): 'TL_DTYPE_INVALID',
    int(TensorProto.UINT32): 'TL_UINT32',
    int(TensorProto.UINT64): 'TL_UINT64',
    int(TensorProto.STRING): 'TL_DTYPE_INVALID'
}

TENSOR_TYPE_NAME = {
    int(TensorProto.FLOAT): 'FLOAT',
    int(TensorProto.UINT8): 'UINT8',
    int(TensorProto.INT8): 'INT8',
    int(TensorProto.UINT16): 'UINT16',
    int(TensorProto.INT16): 'INT16',
    int(TensorProto.INT32): 'INT32',
    int(TensorProto.INT64): 'INT64',
    int(TensorProto.BOOL): 'TL_BOOL',
    int(TensorProto.FLOAT16): 'FLOAT16',
    int(TensorProto.DOUBLE): 'DOUBLE',
    int(TensorProto.COMPLEX64): 'COMPLEX64',
    int(TensorProto.COMPLEX128): 'COMPLEX128',
    int(TensorProto.UINT32): 'UINT32',
    int(TensorProto.UINT64): 'UINT64',
    int(TensorProto.STRING): 'STRING'
}

def dtype_onnx2tl(name, onnx_dtype):
    tl_dtype = TENSOR_TYPE_TO_TL_TYPE[onnx_dtype]
    if tl_dtype == 'TL_DTYPE_INVALID':
        warnings.warn("Can't convert onnx dtype {} of tensor {} to TL_DTYPE. Return TL_DTYPE_INVALID.".format(TENSOR_TYPE_NAME[onnx_dtype], name))
    return tl_dtype
