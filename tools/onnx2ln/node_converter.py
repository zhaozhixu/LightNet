import traceback
import util
from pb_wrapper import OnnxNode

def new_opname(optype):
    if not hasattr(new_opname, 'opname_count'):
        new_opname.opname_count = {}
    if optype in new_opname.opname_count:
        new_opname.opname_count[optype] += 1
    else:
        new_opname.opname_count[optype] = 0
    return optype + str(new_opname.opname_count[optype])

# TODO: give it a class
def error(msg):
    try:
        raise Exception(msg)
    except Exception as e:
        traceback.print_exc()

def handle_pads(node, tensor_dict):
    if not 'pads' in node.attrs and node.attrs['auto_pad'] == 'NOTSET':
        error("'%s' for node '%s' must have a 'pads' attribute or a non-NOTSET 'auto_pad'"%(node.op_type, node.name))
    if 'pads' in node.attrs and 'auto_pad' in node.attrs and node.attrs['auto_pad'] != 'NOTSET':
        error("'%s' for node '%s' cannot use 'pads' and non-NOTSET 'auto_pad' simultaneously"%(node.op_type, node.name))
    if 'pads' in node.attrs:
        pad_shape = node.attrs['pads']
        node.attrs['auto_pad'] = 'NOTSET'
    else:
        pad_shape = [0 for i in range(len(node.attrs['strides']) * 2)]
    return pad_shape

def Add(node, tensor_dict):
    assert node.op_type == 'Add'
    tensor_dict[node.outputs[0]] = {'name': node.outputs[0],
                                    'dtype': tensor_dict[node.inputs[0]]['dtype'],
                                    'dims': None, # TODO: do shape inference
                                    'data': None}
    op = {'name': new_opname('elew'),
          'optype': 'elew',
          'tensors_in': [{'arg_name': 'src1', 'name': node.inputs[0]},
                         {'arg_name': 'src2', 'name': node.inputs[1]}],
          'tensors_out': [{'arg_name': 'dst', 'name': node.outputs[0]}],
          'params': [{'arg_name': 'elew_op', 'value': 'TL_ADD'}]}
    return [op]

def ArgMax(node, tensor_dict):
    assert node.op_type == 'ArgMax'
    if 'keepdims' in node.attrs and node.attrs['keepdims'] == 0:
        error("'%s' for node '%s' only support 'keepdims' == 1 now"%(node.op_type, node.name))

    opname = new_opname('maxreduce_arg') # use it to generate ignored 'dst' name
    tensor_dict[node.outputs[0]] = {'name': opname+'_dst_ln_',
                                    'dtype': tensor_dict[node.inputs[0]]['dtype'],
                                    'dims': None, # TODO: do shape inference
                                    'data': None}
    tensor_dict[node.outputs[0]] = {'name': node.outputs[0],
                                    'dtype': "TL_INT32",
                                    'dims': None, # TODO: do shape inference
                                    'data': None}
    op = {'name': opname,
          'optype': 'maxreduce_arg',
          'tensors_in': [{'arg_name': 'src', 'name': node.inputs[0]}],
          'tensors_out': [{'arg_name': 'dst', 'name': opname+'_dst_ln_'},
                          {'arg_name': 'arg', 'name': node.outputs[0]}],
          'params': [{'arg_name': 'axis', 'value': node.attrs['axis']}]}

    return [op]

def AveragePool(node, tensor_dict):
    assert node.op_type == 'AveragePool'
    if len(node.attrs['kernel_shape']) != 2:
        error("'%s' for node '%s' only supports 2-d tensors now"%(node.op_type, node.name))
    pad_shape = handle_pads(node, tensor_dict)

    tensor_dict[node.outputs[0]] = {'name': node.outputs[0],
                                    'dtype': tensor_dict[node.inputs[0]]['dtype'],
                                    'dims': None, # TODO: do shape inference
                                    'data': None}
    op = {'name': new_opname('avgpool2d'),
          'optype': 'avgpool2d',
          'tensors_in': [{'arg_name': 'src', 'name': node.inputs[0]}],
          'tensors_out': [{'arg_name': 'dst', 'name': node.outputs[0]}],
          'params': [{'arg_name': 'size', 'value': node.attrs['kernel_shape']},
                     {'arg_name': 'stride', 'value': node.attrs['strides']},
                     {'arg_name': 'padding', 'value': pad_shape},
                     {'arg_name': 'autopad', 'value': node.attrs['auto_pad']}]}
    return [op]

def BatchNormalization(node, tensor_dict):
    assert node.op_type == 'BatchNormalization'
    if not 'epsilon' in node.attrs:
        epsilon = 1e-5
    else:
        epsilon = node.attrs['epsilon']

    tensor_dict[node.outputs[0]] = {'name': node.outputs[0],
                                    'dtype': tensor_dict[node.inputs[0]]['dtype'],
                                    'dims': None, # TODO: do shape inference
                                    'data': None}
    op = {'name': new_opname('batchnorm'),
          'optype': 'batchnorm',
          'tensors_in': [{'arg_name': 'src', 'name': node.inputs[0]},
                         {'arg_name': 'scale', 'name': node.inputs[1]},
                         {'arg_name': 'offset', 'name': node.inputs[2]},
                         {'arg_name': 'mean', 'name': node.inputs[3]},
                         {'arg_name': 'var', 'name': node.inputs[4]}],
          'tensors_out': [{'arg_name': 'dst', 'name': node.outputs[0]}],
          'params': [{'arg_name': 'epsilon', 'value': epsilon}]}
    return [op]

def Concat(node, tensor_dict):
    assert node.op_type == 'Concat'
    if len(node.inputs) != 2:
        error("'%s' for node '%s' only supports 2 input tensors now"%(node.op_type, node.name))

    tensor_dict[node.outputs[0]] = {'name': node.outputs[0],
                                    'dtype': tensor_dict[node.inputs[0]]['dtype'],
                                    'dims': None, # TODO: do shape inference
                                    'data': None}
    op = {'name': new_opname('concat'),
          'optype': 'concat',
          'tensors_in': [{'arg_name': 'src1', 'name': node.inputs[0]},
                         {'arg_name': 'src2', 'name': node.inputs[1]}],
          'tensors_out': [{'arg_name': 'dst', 'name': node.outputs[0]}],
          'params': [{'arg_name': 'axis', 'value': node.attrs['axis']}]}
    return [op]

def Conv(node, tensor_dict):
    assert node.op_type == 'Conv'
    if len(node.attrs['strides']) != 2:
        error("'%s' for node '%s' only supports 2-d tensors now"%(node.op_type, node.name))
    if not 'kernel_shape' in node.attrs:
        error("'%s' for node '%s' must have a 'kernel_shape' attribute now"%(node.op_type, node.name))

    pad_shape = handle_pads(node, tensor_dict)

    tensor_dict[node.outputs[0]] = {'name': node.outputs[0],
                                    'dtype': tensor_dict[node.inputs[0]]['dtype'],
                                    'dims': None, # TODO: do shape inference
                                    'data': None}
    op = {'name': new_opname('conv2d'),
          'optype': 'conv2d',
          'tensors_in': [{'arg_name': 'src', 'name': node.inputs[0]},
                         {'arg_name': 'weight', 'name': node.inputs[1]},
                         {'arg_name': 'bias', 'name': node.inputs[2]}],
          'tensors_out': [{'arg_name': 'dst', 'name': node.outputs[0]}],
          'params': [{'arg_name': 'group', 'value': node.attrs['group']},
                     {'arg_name': 'size', 'value': node.attrs['kernel_shape']},
                     {'arg_name': 'stride', 'value': node.attrs['strides']},
                     {'arg_name': 'padding', 'value': pad_shape},
                     {'arg_name': 'autopad', 'value': node.attrs['auto_pad']},
                     {'arg_name': 'dilation', 'value': node.attrs['dilations']}]}
    return [op]

def Div(node, tensor_dict):
    assert node.op_type == 'Div'
    tensor_dict[node.outputs[0]] = {'name': node.outputs[0],
                                    'dtype': tensor_dict[node.inputs[0]]['dtype'],
                                    'dims': None, # TODO: do shape inference
                                    'data': None}
    op = {'name': new_opname('elew'),
          'optype': 'elew',
          'tensors_in': [{'arg_name': 'src1', 'name': node.inputs[0]},
                         {'arg_name': 'src2', 'name': node.inputs[1]}],
          'tensors_out': [{'arg_name': 'dst', 'name': node.outputs[0]}],
          'params': [{'arg_name': 'elew_op', 'value': 'TL_DIV'}]}
    return [op]

def LeakyRelu(node, tensor_dict):
    assert node.op_type == 'LeakyRelu'
    if not 'alpha' in node.attrs:
        alpha = 0.01
    else:
        alpha = node.attrs['alpha']

    tensor_dict[node.outputs[0]] = {'name': node.outputs[0],
                                    'dtype': tensor_dict[node.inputs[0]]['dtype'],
                                    'dims': None, # TODO: do shape inference
                                    'data': None}
    op = {'name': new_opname('lrelu'),
          'optype': 'lrelu',
          'tensors_in': [{'arg_name': 'src', 'name': node.inputs[0]}],
          'tensors_out': [{'arg_name': 'dst', 'name': node.outputs[0]}],
          'params': [{'arg_name': 'negslope', 'value': alpha}]}
    return [op]

def MaxPool(node, tensor_dict):
    assert node.op_type == 'MaxPool'
    if len(node.attrs['kernel_shape']) != 2:
        error("'%s' for node '%s' only supports 2-d tensors now"%(node.op_type, node.name))
    pad_shape = handle_pads(node, tensor_dict)

    tensor_dict[node.outputs[0]] = {'name': node.outputs[0],
                                    'dtype': tensor_dict[node.inputs[0]]['dtype'],
                                    'dims': None, # TODO: do shape inference
                                    'data': None}
    op = {'name': new_opname('maxpool2d'),
          'optype': 'maxpool2d',
          'tensors_in': [{'arg_name': 'src', 'name': node.inputs[0]}],
          'tensors_out': [{'arg_name': 'dst', 'name': node.outputs[0]}],
          'params': [{'arg_name': 'size', 'value': node.attrs['kernel_shape']},
                     {'arg_name': 'stride', 'value': node.attrs['strides']},
                     {'arg_name': 'padding', 'value': pad_shape},
                     {'arg_name': 'autopad', 'value': node.attrs['auto_pad']}]}
    return [op]

def Pow(node, tensor_dict):
    assert node.op_type == 'Pow'
    tensor_dict[node.outputs[0]] = {'name': node.outputs[0],
                                    'dtype': tensor_dict[node.inputs[0]]['dtype'],
                                    'dims': None, # TODO: do shape inference
                                    'data': None}
    op = {'name': new_opname('elew'),
          'optype': 'elew',
          'tensors_in': [{'arg_name': 'src1', 'name': node.inputs[0]},
                         {'arg_name': 'src2', 'name': node.inputs[1]}],
          'tensors_out': [{'arg_name': 'dst', 'name': node.outputs[0]}],
          'params': [{'arg_name': 'elew_op', 'value': 'TL_POW'}]}
    return [op]

def ReduceMax(node, tensor_dict):
    assert node.op_type == 'ReduceMax'
    if not 'axes' in node.attrs or len(node.attrs['axes']) > 1:
        error("'%s' for node '%s' only suppport 'axes' have one element now"%(node.op_type, node.name))
    if 'keepdims' in node.attrs and node.attrs['keepdims'] == 0:
        error("'%s' for node '%s' only suppport 'keepdims' == 1 now"%(node.op_type, node.name))

    tensor_dict[node.outputs[0]] = {'name': node.outputs[0],
                                    'dtype': tensor_dict[node.inputs[0]]['dtype'],
                                    'dims': None, # TODO: do shape inference
                                    'data': None}
    op = {'name': new_opname('maxreduce'),
          'optype': 'maxreduce',
          'tensors_in': [{'arg_name': 'src', 'name': node.inputs[0]}],
          'tensors_out': [{'arg_name': 'dst', 'name': node.outputs[0]}],
          'params': [{'arg_name': 'axes', 'value': node.attrs['axes'][0]}]}

    return [op]

def Relu(node, tensor_dict):
    assert node.op_type == 'Relu'
    tensor_dict[node.outputs[0]] = {'name': node.outputs[0],
                                    'dtype': tensor_dict[node.inputs[0]]['dtype'],
                                    'dims': None, # TODO: do shape inference
                                    'data': None}
    op = {'name': new_opname('relu'),
          'optype': 'relu',
          'tensors_in': [{'arg_name': 'src', 'name': node.inputs[0]}],
          'tensors_out': [{'arg_name': 'dst', 'name': node.outputs[0]}],
          'params': []}

    return [op]

def Reshape(node, tensor_dict):
    assert node.op_type == 'Reshape'
    assert node.inputs[1] in tensor_dict
    if tensor_dict[node.inputs[1]]['data'] is None:
        error("'%s' for node '%s' doesn't support dynamic supplied 'shape' tensor now"%(node.op_type, node.name))

    tensor_dict[node.outputs[0]] = {'name': node.outputs[0],
                                    'dtype': tensor_dict[node.inputs[0]]['dtype'],
                                    'dims': None, # TODO: do shape inference
                                    'data': None}
    op = {'name': new_opname('reshape'),
          'optype': 'reshape',
          'tensors_in': [{'arg_name': 'src', 'name': node.inputs[0]}],
          'tensors_out': [{'arg_name': 'dst', 'name': node.outputs[0]}],
          'params': [{'arg_name': 'dims', 'value': tensor_dict[node.inputs[1]]['data']}]}

    return [op]

def Resize(node, tensor_dict):
    assert node.op_type == 'Resize'
    if not 'mode' in node.attrs:
        mode = 'TL_NEAREST'
    elif node.attrs['mode'] == 'nearest':
        mode = 'TL_NEAREST'
    elif node.attrs['mode'] == 'linear':
        mode = 'TL_LINEAR'
    else:
        error("'%s' for node '%s' doesn't support 'mode' == '%s'"%(node.op_type, node.name, node.attrs['mode']))

    assert node.inputs[1] in tensor_dict
    if tensor_dict[node.inputs[1]]['data'] is None:
        error("'%s' for node '%s' doesn't support dynamic supplied 'scales' tensor now"%(node.op_type, node.name))

    tensor_dict[node.outputs[0]] = {'name': node.outputs[0],
                                    'dtype': tensor_dict[node.inputs[0]]['dtype'],
                                    'dims': None, # TODO: do shape inference
                                    'data': None}
    op = {'name': new_opname('resize'),
          'optype': 'resize',
          'tensors_in': [{'arg_name': 'src', 'name': node.inputs[0]}],
          'tensors_out': [{'arg_name': 'dst', 'name': node.outputs[0]}],
          'params': [{'arg_name': 'mode', 'value': mode},
                     {'arg_name': 'scales', 'value': tensor_dict[node.inputs[1]]['data']}]}

    return [op]

def Sigmoid(node, tensor_dict):
    assert node.op_type == 'Sigmoid'
    tensor_dict[node.outputs[0]] = {'name': node.outputs[0],
                                    'dtype': tensor_dict[node.inputs[0]]['dtype'],
                                    'dims': None, # TODO: do shape inference
                                    'data': None}
    op = {'name': new_opname('sigmoid'),
          'optype': 'sigmoid',
          'tensors_in': [{'arg_name': 'src', 'name': node.inputs[0]}],
          'tensors_out': [{'arg_name': 'dst', 'name': node.outputs[0]}],
          'params': []}

    return [op]

def Slice(node, tensor_dict):
    assert node.op_type == 'Slice'
    assert node.inputs[1] in tensor_dict
    assert node.inputs[2] in tensor_dict
    assert node.inputs[3] in tensor_dict
    assert node.inputs[4] in tensor_dict
    if tensor_dict[node.inputs[1]]['data'] is None:
        error("'%s' for node '%s' doesn't support dynamic supplied 'starts' tensor now"%(node.op_type, node.name))
    if tensor_dict[node.inputs[2]]['data'] is None:
        error("'%s' for node '%s' doesn't support dynamic supplied 'ends' tensor now"%(node.op_type, node.name))
    if tensor_dict[node.inputs[3]]['data'] is None:
        error("'%s' for node '%s' doesn't support dynamic supplied 'axes' tensor now"%(node.op_type, node.name))
    if tensor_dict[node.inputs[4]]['data'] is None:
        error("'%s' for node '%s' doesn't support dynamic supplied 'steps' tensor now"%(node.op_type, node.name))
    if len(tensor_dict[node.inputs[1]]['data']) != 1 or len(tensor_dict[node.inputs[2]]['data']) != 1 or len(tensor_dict[node.inputs[3]]['data']) != 1 or len(tensor_dict[node.inputs[4]]['data']) != 1:
        error("'%s' for node '%s' only support slice on one axis now"%(node.op_type, node.name))
    if tensor_dict[node.inputs[4]]['data'][0] != 1:
        error("'%s' for node '%s' only support 'steps' == 1 now"%(node.op_type, node.name))

    tensor_dict[node.outputs[0]] = {'name': node.outputs[0],
                                    'dtype': tensor_dict[node.inputs[0]]['dtype'],
                                    'dims': None, # TODO: do shape inference
                                    'data': None}
    op = {'name': new_opname('slice'),
          'optype': 'slice',
          'tensors_in': [{'arg_name': 'src', 'name': node.inputs[0]}],
          'tensors_out': [{'arg_name': 'dst', 'name': node.outputs[0]}],
          'params': [{'arg_name': 'start', 'value': tensor_dict[node.inputs[1]]['data'][0]},
                     {'arg_name': 'axis', 'value': tensor_dict[node.inputs[3]]['data'][0]},
                     {'arg_name': 'len', 'value': tensor_dict[node.inputs[2]]['data'][0]-tensor_dict[node.inputs[1]]['data'][0]}]}

    return [op]

def Softmax(node, tensor_dict):
    assert node.op_type == 'Softmax'
    if not 'axis' in node.attrs:
        axis = 1
    else:
        axis = node.attrs['axis']

    tensor_dict[node.outputs[0]] = {'name': node.outputs[0],
                                    'dtype': tensor_dict[node.inputs[0]]['dtype'],
                                    'dims': None, # TODO: do shape inference
                                    'data': None}
    op = {'name': new_opname('softmax'),
          'optype': 'softmax',
          'tensors_in': [{'arg_name': 'src', 'name': node.inputs[0]}],
          'tensors_out': [{'arg_name': 'dst', 'name': node.outputs[0]}],
          'params': [{'arg_name': 'axis', 'value': axis}]}

    return [op]

def Transpose(node, tensor_dict):
    assert node.op_type == 'Transpose'

    tensor_dict[node.outputs[0]] = {'name': node.outputs[0],
                                    'dtype': tensor_dict[node.inputs[0]]['dtype'],
                                    'dims': None, # TODO: do shape inference
                                    'data': None}
    op = {'name': new_opname('transpose'),
          'optype': 'transpose',
          'tensors_in': [{'arg_name': 'src', 'name': node.inputs[0]}],
          'tensors_out': [{'arg_name': 'dst', 'name': node.outputs[0]}],
          'params': [{'arg_name': 'axes', 'value': node.attrs['perm']}]}

    return [op]

def Upsample(node, tensor_dict):
    assert node.op_type == 'Upsample'
    if not 'mode' in node.attrs:
        mode = 'TL_NEAREST'
    elif node.attrs['mode'] == 'nearest':
        mode = 'TL_NEAREST'
    elif node.attrs['mode'] == 'linear':
        mode = 'TL_LINEAR'
    else:
        error("'%s' for node '%s' doesn't support 'mode' == '%s'"%(node.op_type, node.name, node.attrs['mode']))

    assert node.inputs[1] in tensor_dict
    if tensor_dict[node.inputs[1]]['data'] is None:
        error("'%s' for node '%s' doesn't support dynamic supplied 'scales' tensor now"%(node.op_type, node.name))

    tensor_dict[node.outputs[0]] = {'name': node.outputs[0],
                                    'dtype': tensor_dict[node.inputs[0]]['dtype'],
                                    'dims': None, # TODO: do shape inference
                                    'data': None}
    op = {'name': new_opname('resize'),
          'optype': 'resize',
          'tensors_in': [{'arg_name': 'src', 'name': node.inputs[0]}],
          'tensors_out': [{'arg_name': 'dst', 'name': node.outputs[0]}],
          'params': [{'arg_name': 'mode', 'value': mode},
                     {'arg_name': 'scales', 'value': tensor_dict[node.inputs[1]]['data']}]}

    return [op]

# Dropout treated as a Forward op
def Dropout(node, tensor_dict):
    assert node.op_type == 'Dropout'
    tensor_dict[node.outputs[0]] = {'name': node.outputs[0],
                                    'dtype': tensor_dict[node.inputs[0]]['dtype'],
                                    'dims': None, # TODO: do shape inference
                                    'data': None}
    op = {'name': new_opname('forward'),
          'optype': 'forward',
          'tensors_in': [{'arg_name': 'src', 'name': node.inputs[0]}],
          'tensors_out': [{'arg_name': 'dst', 'name': node.outputs[0]}],
          'params': []}

    return [op]

onnx_to_ln_op_converters = {
    'Add': Add,
    'ArgMax': ArgMax,
    'AveragePool': AveragePool,
    'BatchNormalization': BatchNormalization,
    'Concat': Concat,
    'Conv': Conv,
    'Div': Div,
    'LeakyRelu': LeakyRelu,
    'MaxPool': MaxPool,
    'Pow': Pow,
    'ReduceMax': ReduceMax,
    'Relu': Relu,
    'Reshape': Reshape,
    'Resize': Resize,
    'Sigmoid': Sigmoid,
    'Slice': Slice,
    'Softmax': Softmax,
    'Upsample': Upsample,
    'Dropout': Dropout,
}

def unsupported_node(node, tensor_dict):
    error("unimplemented ONNX operator type '%s' for node '%s'"%(node.op_type, node.name))

def onnx_node_to_ln_op(onnx_node, tensor_dict):
    return onnx_to_ln_op_converters.get(onnx_node.op_type,
                                        unsupported_node)(onnx_node, tensor_dict)
