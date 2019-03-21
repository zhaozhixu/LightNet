import traceback
import util
from pb_wrapper import OnnxNode

def new_opname(optype):
    if not hasattr(new_opname, 'opname_count'):
        new_opname.opname_count = {}
    if new_opname.opname_count.has_key(optype):
        new_opname.opname_count[optype] += 1
    else:
        new_opname.opname_count[optype] = 0
    return optype + str(new_opname.opname_count[optype])

# TODO: give it a class
def error(msg):
    try:
        raise Exception, msg
    except Exception, e:
        traceback.print_exc(e)

def handle_pads(node):
    if not node.attrs.has_key('pads') and node.attrs['auto_pad'] == 'NOTSET':
        error("'%s' for node '%s' must have a 'pads' attribute or a non-NOTSET 'auto_pad'"%(node.op_type, node.name))
    if node.attrs.has_key('pads') and node.attrs['auto_pad'] != 'NOTSET':
        error("'%s' for node '%s' cannot use 'pads' and non-NOTSET 'auto_pad' simultaneously"%(node.op_type, node.name))
    if node.attrs.has_key('pads'):
        pad_shape = node.attrs['pads']
    else:
        pad_shape = [0 for i in range(len(node.attrs['strides']) * 2)]
    return pad_shape

def Add(node):
    assert node.op_type == 'Add'
    op = {'name': new_opname('elew'),
          'optype': 'elew',
          'tensors_in': [{'arg_name': 'src1', 'name': node.inputs[0]},
                         {'arg_name': 'src2', 'name': node.inputs[1]}],
          'tensors_out': [{'arg_name': 'dst', 'name': node.outputs[0]}],
          'params': [{'arg_name': 'elew_op', 'value': 'TL_ADD'}]}
    return [op]

def ArgMax(node):
    assert node.op_type == 'ArgMax'
    opname = new_opname('maxreduce_arg') # use it to generate ignored 'dst' name
    op = {'name': opname,
          'optype': 'maxreduce_arg',
          'tensors_in': [{'arg_name': 'src', 'name': node.inputs[0]}],
          'tensors_out': [{'arg_name': 'dst', 'name': opname+'_dst'},
                          {'arg_name': 'arg', 'name': node.outputs[0]}],
          'params': [{'arg_name': 'axis', 'value': node.attrs['axis']}]}
    return [op]

def AveragePool(node):
    assert node.op_type == 'AveragePool'
    if len(node.attrs['kernel_shape']) != 2:
        error("'%s' for node '%s' only supports 2-d tensors now"%(node.op_type, node.name))
    pad_shape = handle_pads(node)

    op = {'name': new_opname('avgpool2d'),
          'optype': 'avgpool2d',
          'tensors_in': [{'arg_name': 'src', 'name': node.inputs[0]}],
          'tensors_out': [{'arg_name': 'dst', 'name': node.outputs[0]}],
          'params': [{'arg_name': 'size', 'value': node.attrs['kernel_shape']},
                     {'arg_name': 'stride', 'value': node.attrs['strides']},
                     {'arg_name': 'padding', 'value': pad_shape},
                     {'arg_name': 'autopad', 'value': node.attrs['auto_pad']}]}
    return [op]

def BatchNormalization(node):
    assert node.op_type == 'BatchNormalization'
    if not node.attrs.has_key('epsilon'):
        epsilon = 1e-5
    else:
        epsilon = node.attrs['epsilon']

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

def Concat(node):
    assert node.op_type == 'Concat'
    if len(node.inputs) != 2:
        error("'%s' for node '%s' only supports 2 input tensors now"%(node.op_type, node.name))

    op = {'name': new_opname('concat'),
          'optype': 'concat',
          'tensors_in': [{'arg_name': 'src1', 'name': node.inputs[0]},
                         {'arg_name': 'src2', 'name': node.inputs[1]}],
          'tensors_out': [{'arg_name': 'dst', 'name': node.outputs[0]}],
          'params': [{'arg_name': 'axis', 'value': node.attrs['axis']}]}
    return [op]

def Conv(node):
    assert node.op_type == 'Conv'
    if len(node.attrs['strides']) != 2:
        error("'%s' for node '%s' only supports 2-d tensors now"%(node.op_type, node.name))
    if not node.attrs.has_key('kernel_shape'):
        error("'%s' for node '%s' must have a 'kernel_shape' attribute now"%(node.op_type, node.name))

    pad_shape = handle_pads(node)

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
                     {'arg_name': 'autopad', 'value': node.attrs['autopad']},
                     {'arg_name': 'dilation', 'value': node.attrs['dilation']}]}
    return [op]

def Div(node):
    assert node.op_type == 'Div'
    op = {'name': new_opname('elew'),
          'optype': 'elew',
          'tensors_in': [{'arg_name': 'src1', 'name': node.inputs[0]},
                         {'arg_name': 'src2', 'name': node.inputs[1]}],
          'tensors_out': [{'arg_name': 'dst', 'name': node.outputs[0]}],
          'params': [{'arg_name': 'elew_op', 'value': 'TL_DIV'}]}
    return [op]

def LeakyRelu(node):
    assert node.op_type == 'LeakyRelu'
    if not node.attrs.has_key('alpha'):
        alpha = 0.01
    else:
        alpha = node.attrs['alpha']
    op = {'name': new_opname('lrelu'),
          'optype': 'lrelu',
          'tensors_in': [{'arg_name': 'src', 'name': node.inputs[0]}],
          'tensors_out': [{'arg_name': 'dst', 'name': node.outputs[0]}],
          'params': [{'arg_name': 'negslope', 'value': alpha}]}
    return [op]

def MaxPool(node):
    assert node.op_type == 'MaxPool'
    if len(node.attrs['kernel_shape']) != 2:
        error("'%s' for node '%s' only supports 2-d tensors now"%(node.op_type, node.name))
    pad_shape = handle_pads(node)

    op = {'name': new_opname('maxpool2d'),
          'optype': 'maxpool2d',
          'tensors_in': [{'arg_name': 'src', 'name': node.inputs[0]}],
          'tensors_out': [{'arg_name': 'dst', 'name': node.outputs[0]}],
          'params': [{'arg_name': 'size', 'value': node.attrs['kernel_shape']},
                     {'arg_name': 'stride', 'value': node.attrs['strides']},
                     {'arg_name': 'padding', 'value': pad_shape},
                     {'arg_name': 'autopad', 'value': node.attrs['auto_pad']}]}
    return [op]

def Pow(node):
    assert node.op_type == 'Pow'
    op = {'name': new_opname('elew'),
          'optype': 'elew',
          'tensors_in': [{'arg_name': 'src1', 'name': node.inputs[0]},
                         {'arg_name': 'src2', 'name': node.inputs[1]}],
          'tensors_out': [{'arg_name': 'dst', 'name': node.outputs[0]}],
          'params': [{'arg_name': 'elew_op', 'value': 'TL_POW'}]}
    return [op]

onnx_to_ln_op_converters = {
    'Add': Add,
    'ArgMax': ArgMax,
    'AveragePool': AveragePool,
    'BatchNormalization', BatchNormalization,
    'Concat', Concat,
    'Conv', Conv,
    'Div', Div,
    'LeakyRelu', LeakyRelu,
    'MaxPool', MaxPool,
    'Pow', Pow,
}

def unsupported_node(node):
    error("unimplemented ONNX operator type '%s' for node '%s'"%(node.op_type, node.name))

def onnx_node_to_ln_op(onnx_node):
    return onnx_to_ln_op_converters.get(onnx_node.op_type,
                                        unsupported_node)(onnx_node)
