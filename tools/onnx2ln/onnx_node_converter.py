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

def Add(node):
    assert node.op_type == 'Add'
    op = {'name': new_opname('elew'),
          'optype': 'elew',
          'tensors_in': [{'arg_name': 'src1', 'name': node.inputs[0]},
                         {'arg_name': 'src2', 'name': node.inputs[1]}],
          'tensors_out': [{'arg_name': 'dst', 'name': node.outputs[0]}],
          'params': [{'arg_name': 'elew_op', 'value': 'TL_ADD'}]}
    return op

def ArgMax(node):
    assert node.op_type == 'ArgMax'
    opname = new_opname('maxreduce_arg') # use it to generate ignored 'dst' name
    op = {'name': opname,
          'optype': 'maxreduce_arg',
          'tensors_in': [{'arg_name': 'src', 'name': node.inputs[0]}],
          'tensors_out': [{'arg_name': 'dst', 'name': opname+'_dst'},
                          {'arg_name': 'arg', 'name': node.outputs[0]}],
          'params': [{'arg_name': 'axis', 'value': node.attrs['axis']}]}
    return op

def AveragePool(node):
    assert node.op_type == 'AveragePool'
    if len(node.attrs['kernel_shape']) != 2:
        error("'AveragePool' for node '%s' only supports 2-d tensors now"%node.name)
    if not node.attrs.has_key('pads') and node.attrs['auto_pad'] == 'NOTSET':
        error("'AveragePool' for node '%s' must have a 'pads' attribute or a non-NOTSET 'auto_pad'"%node.name)
    if node.attrs.has_key('pads') and node.attrs['auto_pad'] != 'NOTSET':
        error("'AveragePool' for node '%s' cannot use 'pads' and non-NOTSET 'auto_pad' simultaneously"%node.name)

    pad_shape = []
    if node.attrs.has_key('pads'):
        pad_shape = node.attrs['pads']
    elif node.attrs['auto_pad'] != 'NOTSET':
        pad_shape = util.autopad_shape()
    else:
        assert False

    op = {'name': new_opname('avgpool2d'),
          'optype': 'avgpool2d',
          'tensors_in': [{'arg_name': 'src', 'name': node.inputs[0]}],
          'tensors_out': [{'arg_name': 'dst', 'name': node.outputs[0]}],
          'params': [{'arg_name': 'size', 'value': node.attrs['kernel_shape']},
                     {'arg_name': 'stride', 'value': node.attrs['strides']},
                     {'arg_name': 'padding', 'value': pad_shape}]}
    return op

def BatchNormalization(node):
    assert node.op_type == 'BatchNormalization'
    if len(node.attrs['kernel_shape']) != 2:
        error("'AveragePool' for node '%s' only supports 2-d tensors now"%node.name)
    op = {'name': new_opname('avgpool2d'),
          'optype': 'avgpool2d',
          'tensors_in': [{'arg_name': 'src', 'name': node.inputs[0]}],
          'tensors_out': [{'arg_name': 'dst', 'name': node.outputs[0]}],
          'params': [{'arg_name': 'size', 'value': node.attrs['kernel_shape']},
                     {'arg_name': 'stride', 'value': node.attrs['strides']},
                     {'arg_name': 'padding', 'value': node.attrs['pads']}]}
    return op

onnx_to_ln_op_converters = {
    'Add': Add,
    'ArgMax': ArgMax,
    'AveragePool': AveragePool,
}

def unsupported_node(node):
    error("unimplemented ONNX operator type '%s' for node '%s'"%(node.op_type, node.name))

def onnx_node_to_ln_op(onnx_node):
    return onnx_to_ln_op_converters.get(onnx_node.op_type,
                                        unsupported_node)(onnx_node)
