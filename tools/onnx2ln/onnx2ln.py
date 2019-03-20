# This file is based on https://github.com/onnx/onnx-tensorflow

import json
import warnings

from onnx import TensorProto
from onnx import numpy_helper
from onnx_tf.common import data_type
from pb_wrapper import OnnxNode
from pb_wrapper import OnnxGraph
from onnx_node_converter import new_opname

TENSOR_TYPE_TO_TL_TYPE = {
    int(TensorProto.FLOAT): 'TL_FLOAT',
    int(TensorProto.UINT8): 'TL_UINT8',
    int(TensorProto.INT8): 'TL_INT8',
    int(TensorProto.UINT16): 'TL_UINT16',
    int(TensorProto.INT16): 'TL_INT16',
    int(TensorProto.INT32): 'TL_INT32',
    int(TensorProto.INT64): 'TL_DTYPE_INVALID',
    int(TensorProto.BOOL): 'TL_BOOL',
    int(TensorProto.FLOAT16): 'TL_DTYPE_INVALID',
    int(TensorProto.DOUBLE): 'TL_DOUBLE',
    int(TensorProto.COMPLEX64): 'TL_DTYPE_INVALID',
    int(TensorProto.COMPLEX128): 'TL_DTYPE_INVALID',
    int(TensorProto.UINT32): 'TL_UINT32',
    int(TensorProto.UINT64): 'TL_DTYPE_INVALID',
    int(TensorProto.STRING): 'TL_DTYPE_INVALID'
}

def dtype_onnx2tl(onnx_dtype):
    tl_dtype = TENSOR_TYPE_TO_TL_TYPE[onnx_dtype]
    if tl_dtype == 'TL_DTYPE_INVALID':
        warnings.warn("Can't convert onnx dtype {} to TensorLight dtype. Return TL_DTYPE_INVALID".format(onnx_dtype))
        return tl_dtype

def onnx_get_model(onnx_graph, opset):
    if onnx_graph.initializer:
        input_tensors = onnx_initializer_to_data_tensors(
            onnx_graph.initializer);
        initialized = {init.name for init in onnx_graph.initializer}
    else:
        input_tensors = []
        initialized = set()

    # creating empty tensors for currently unknown inputs
    for value_info in onnx_graph.input:
        if value_info.name in initialized:
            continue
        shape = list(
            d.dim_value if (d.dim_value > 0 and d.dim_param == "") else None
            for d in value_info.type.tensor_type.shape.dim)
        tensor = {'name': value_info.name,
              'dtype': dtype_onnx2tl(value_info.type.tensor_type.elem_type),
              'dims': shape,
              'data': [0]}
        input_tensors.append(tensor)

    model = {'ops': []}

    for tensor in input_tensors:
        model['ops'].append(new_create_op(tensor))

    for node in onnx_graph.node:
        onnx_node = OnnxNode(node)
        model['ops'].append(onnx_node_to_ln_op(onnx_node))

    return model

def onnx_initializer_to_data_tensors(initializer):
    """ Convert ONNX graph initializer to input tensor items.

    :param initializer: ONNX graph initializer, list of TensorProto.
    :return: List of input tensor items.
    """

    def tensor2list(onnx_tensor):
        # Use the onnx.numpy_helper because the data may be raw
        return numpy_helper.to_array(onnx_tensor).flatten().tolist()
    tensors = [{'name': init.name,
                'dtype': dtype_onnx2tl(init.data_type),
                'dims': init.dims,
                'data': tensor2list(init)}
               for init in initializer]
    return tensors

def onnx_node_to_ln_op(onnx_node):
    pass

def new_create_op(tensor):
    op = {'name': new_opname("create"),
          'optype': 'create',
          'tensors_in': [],
          'tensors_out': [{'arg_name': 'dst', 'name': tensor['name']}],
          'params': [{'arg_name': 'dtype', 'value': tensor['dtype']},
                     {'arg_name': 'dims', 'value': tensor['dims']},
                     {'arg_name': 'data', 'value': tensor['data']},
                     {'arg_name': 'ran', 'value': [0, 0]},
                     {'arg_name': 'from_file', 'value': false}]}
    return op
