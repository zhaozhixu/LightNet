#! /usr/bin/env python3

import sys
import os
import onnx
import onnx2ln
import json

onnx_path = sys.argv[1]
onnx_model = onnx.load(onnx_path)
# print(onnx_model)
ln_model = onnx2ln.onnx_get_model(onnx_model.graph)
# print(ln_model)
ln_model_json = json.dumps(ln_model, indent=4)
print(ln_model_json);
