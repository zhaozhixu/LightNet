import sys
import os

sys.path.append(os.path.split(os.path.realpath(__file__))[0])

import backend

def onnx_get_model(onnx_graph):
    return backend.get_model(onnx_graph)
