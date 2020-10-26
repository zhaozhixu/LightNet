# This file is based on https://github.com/onnx/onnx-tensorflow

import json
import warnings
import sys

import onnx
from onnx import TensorProto
from onnx import numpy_helper
from onnx import shape_inference
from onnx import ModelProto
from onnx import GraphProto
from onnx import helper
# from onnx_tf.common import data_type
from pb_wrapper import OnnxNode
from pb_wrapper import OnnxGraph
from node_converter import new_opname
from node_converter import new_create_op
from node_converter import tensor_proto_to_tensor
from node_converter import onnx_node_to_ln_op
from type_converter import dtype_onnx2tl

def add_value_info_for_constants(model : onnx.ModelProto):
    """
    Currently onnx.shape_inference doesn't use the shape of initializers, so add
    that info explicitly as ValueInfoProtos.
    Mutates the model.
    Args:
        model: The ModelProto to update.
    """
    # All (top-level) constants will have ValueInfos before IRv4 as they are all inputs
    if model.ir_version < 4:
        return

    def add_const_value_infos_to_graph(graph : onnx.GraphProto):
        inputs = {i.name for i in graph.input}
        existing_info = {vi.name: vi for vi in graph.value_info}
        for init in graph.initializer:
            # Check it really is a constant, not an input
            if init.name in inputs:
                continue

            # The details we want to add
            elem_type = init.data_type
            shape = init.dims

            # Get existing or create new value info for this constant
            vi = existing_info.get(init.name)
            if vi is None:
                vi = graph.value_info.add()
                vi.name = init.name

            # Even though it would be weird, we will not overwrite info even if it doesn't match
            tt = vi.type.tensor_type
            if tt.elem_type == onnx.TensorProto.UNDEFINED:
                tt.elem_type = elem_type
            if not tt.HasField("shape"):
                # Ensure we set an empty list if the const is scalar (zero dims)
                tt.shape.dim.extend([])
                for dim in shape:
                    tt.shape.dim.add().dim_value = dim

        # Handle subgraphs
        for node in graph.node:
            for attr in node.attribute:
                # Ref attrs refer to other attrs, so we don't need to do anything
                if attr.ref_attr_name != "":
                    continue

                if attr.type == onnx.AttributeProto.GRAPH:
                    add_const_value_infos_to_graph(attr.g)
                if attr.type == onnx.AttributeProto.GRAPHS:
                    for g in attr.graphs:
                        add_const_value_infos_to_graph(g)


    return add_const_value_infos_to_graph(model.graph)

def get_model(onnx_model):
    if not isinstance(onnx_model, ModelProto) and not isinstance(onnx_model, GraphProto):
        raise TypeError('get_model() only accepts ModelProto or GraphProto '
                        'incorrect type: {}'.format(type(onnx_model)))
    if isinstance(onnx_model, GraphProto):
        onnx_model = helper.make_model(onnx_model)
    add_value_info_for_constants(onnx_model)
    onnx_model = shape_inference.infer_shapes(onnx_model)
    # onnx.checker.check_model(onnx_model)
    onnx_graph = onnx_model.graph
    # print(onnx_graph)
    # exit()
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
                  'dtype': dtype_onnx2tl(value_info.name, value_info.type.tensor_type.elem_type),
                  'dims': shape,
                  'data': None}
        input_tensors.append((value_info.name, tensor))

    value_infos = {vi.name: vi for vi in onnx_graph.value_info}
    for vi in onnx_graph.output:
        value_infos[vi.name] = vi
    tensor_dict = dict(input_tensors)
    tensor_dict['__value_infos'] = value_infos
    model = {'ops': []}

    for tensor in input_tensors:
        model['ops'].append(new_create_op(tensor[1]))

    for node in onnx_graph.node:
        onnx_node = OnnxNode(node)
        ops = onnx_node_to_ln_op(onnx_node, tensor_dict)
        for op in ops:
            model['ops'].append(op)

    return model

def onnx_initializer_to_data_tensors(initializer):
    return [tensor_proto_to_tensor(init) for init in initializer]
