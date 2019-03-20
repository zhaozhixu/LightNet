# This file is based on https://github.com/onnx/onnx-tensorflow

import inspect
from itertools import chain
import warnings

import numpy as np
from onnx import NodeProto
from onnx import TensorProto
from onnx import ValueInfoProto
from onnx import numpy_helper
from onnx.helper import make_graph
from onnx.helper import make_tensor
from onnx.helper import make_tensor_value_info
from onnx.helper import mapping

import util
import attr_converter

class OnnxNode(object):
  """
  Reimplementation of NodeProto from ONNX, but in a form
  more convenient to work with from Python.
  """

  def __init__(self, node):
    self.name = str(node.name)
    self.op_type = str(node.op_type)
    self.domain = str(node.domain)
    self.attrs = dict([(attr.name,
                        attr_converter.translate_onnx(
                          attr.name, attr_converter.convert_onnx(attr)))
                       for attr in node.attribute])
    self.inputs = list(node.input)
    self.outputs = list(node.output)
    self.node_proto = node


class OnnxGraph(object):
  """ A helper class for making ONNX graph.
  This class holds all information ONNX graph needs.
  """

  def __init__(self, name=None, graph_proto=None):
    if graph_proto:
      self._name = graph_proto.name
      self._inputs_proto = list(graph_proto.input)
      self._outputs_proto = list(graph_proto.output)
      self._nodes_proto = list(graph_proto.node)
      self._consts_proto = list(graph_proto.initializer)
      self._value_info_proto = list(graph_proto.value_info)
      self._consts = dict([(init.name, numpy_helper.to_array(init))
                           for init in graph_proto.initializer])
    else:
      self._name = name or ""
      self._inputs_proto = []
      self._outputs_proto = []
      self._nodes_proto = []
      self._consts = {}
      self._consts_proto = []
      self._value_info_proto = []
    # Either way, data_type_cast_map is empty when initialized.
    self._data_type_cast_map = {}
