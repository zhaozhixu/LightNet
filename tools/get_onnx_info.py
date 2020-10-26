#! /usr/bin/env python3

import argparse
import onnx

def infer_shape(model):
    onnx_model = model
    if isinstance(model, onnx.GraphProto):
        onnx_model = onnx.helper.make_model(model)
    return onnx.shape_inference.infer_shapes(onnx_model)

def print_onnx_initializers(graph):
    for tensor in graph.initializer:
        print('initializer:\n{}'.format(tensor))

def print_onnx_inputs(graph):
    for value_info in graph.input:
        print('input:\n{}'.format(value_info))

def print_onnx_feed_inputs(graph):
    inputs = {value_info.name: value_info for value_info in graph.input}
    initializers = {tensor.name: tensor for tensor in graph.initializer}
    fead_input_names = set(inputs.keys()) - set(initializers.keys())
    fead_inputs = {name: inputs[name] for name in fead_input_names}
    for value_info in fead_inputs.values():
        print('fead_input:\n{}'.format(value_info))

def print_onnx_outputs(graph):
    for value_info in graph.output:
        print('output:\n{}'.format((value_info)))

def print_onnx_value_infos(graph):
    for value_info in graph.value_info:
        print('value_info:\n{}'.format((value_info)))

def print_onnx_nodes(graph):
    for node in graph.node:
        print('node:\n{}'.format(node))

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Get information from an onnx model')
    parser.add_argument('model', nargs=1,
                        help='onnx model path')
    parser.add_argument('--inputs', action='store_true',
                        help='print the input tensors')
    parser.add_argument('--feed_inputs', action='store_true',
                        help='print the input tensors that should be feed to the model')
    parser.add_argument('--initializers', action='store_true',
                        help='print the initializer tensors')
    parser.add_argument('--outputs', action='store_true',
                        help='print the output tensors')
    parser.add_argument('--value_infos', action='store_true',
                        help='print the GraphProto.value_info in the model')
    parser.add_argument('--nodes', action='store_true',
                        help='print the GraphProto.node in the model')
    args = parser.parse_args()

    input_model = onnx.load(args.model[0])
    onnx_model = infer_shape(input_model)
    if (args.inputs):
        print_onnx_inputs(onnx_model.graph)
    if (args.feed_inputs):
        print_onnx_feed_inputs(onnx_model.graph)
    if (args.initializers):
        print_onnx_initializers(onnx_model.graph)
    if (args.outputs):
        print_onnx_outputs(onnx_model.graph)
    if (args.value_infos):
        print_onnx_value_infos(onnx_model.graph)
    if (args.nodes):
        print_onnx_nodes(onnx_model.graph)
