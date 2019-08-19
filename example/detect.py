import cv2
import numpy as np
import os

import pylightnet as ln
from ctypes import *

INPUT_H = 368
INPUT_W = 640
INPUT_C = 3

# anchor shape for ShuffleDet
CONVOUT_H = 12
CONVOUT_W = 20
CONVOUT_C = 153
ANCHORS_PER_GRID = 9
ANCHOR_SHAPE = [ 229., 137., 48., 71., 289., 245.,
                 185., 134., 85., 142., 31., 41.,
                 197., 191., 237., 206., 63., 108.]

def set_anchors(convout_w, convout_h, anchors_per_grid):
    H, W, B = 12, 20, 9
    anchor_shapes = np.reshape(
        [np.array(
            [[  229.,  137.], [ 48., 71.], [ 289.,  245.],
             [ 185.,  134.], [  85.,  142.], [ 31., 41.],
             [ 197., 191.], [  237., 206.], [  63.,  108.]])] * H * W,
        (H, W, B, 2)
    )
    center_x = np.reshape(
        np.transpose(
            np.reshape(
                np.array([np.arange(1, W+1)*float(INPUT_W)/(W+1)]*H*B),
                (B, H, W)
            ),
            (1, 2, 0)
        ),
        (H, W, B, 1)
    )
    center_y = np.reshape(
        np.transpose(
            np.reshape(
                np.array([np.arange(1, H+1)*float(INPUT_H)/(H+1)]*W*B),
                (B, W, H)
            ),
            (2, 1, 0)
        ),
        (H, W, B, 1)
    )
    anchors = np.reshape(
        np.concatenate((center_x, center_y, anchor_shapes), axis=3),
        (convout_h*convout_w*anchors_per_grid*4)
    )

    res = np.zeros(convout_h*convout_w*anchors_per_grid*4, dtype=np.float32)
    for i in range(len(res)):
        res[i] = anchors[i]

    return res

mean = np.array([103.939, 116.779, 123.68], dtype=np.float32)
ready_data = ln.lib.alloc(INPUT_W*INPUT_H*INPUT_C*4)

def preprocess(data):
    global mean, ready_data
    data = cv2.resize(data, (INPUT_W, INPUT_H)).flatten()
    ln.util.img_submean(data.ctypes.data_as(POINTER(c_ubyte)),
                        mean.ctypes.data_as(POINTER(c_float)),
                        ready_data, INPUT_H, INPUT_W, INPUT_C)

infer = None
out_dict = None
current_dir = os.path.split(os.path.realpath(__file__))[0]

def init():
    global infer, out_dict, current_dir
    # compile model, *.net is the model file
    cmd = 'ir2json.pl ' + current_dir + '/data/shuffledet_dac.net > ' \
        + current_dir + '/data/net.json'
    status = os.system(cmd)
    if status != 0:
        print("ir2json.pl failed")
        exit(1)
    compiler = ln.compiler.compiler(current_dir + '/data/net.json',
                                    'tensorrt', current_dir + '/data/out.json')
    compiler.compile()
    # out.json is the compiled model, *.wts is the weight file
    infer = ln.infer.infer(current_dir + '/data/out.json',
                           current_dir + '/data/shuffledet_dac.wts')

    out_dict = infer.create_data_dict(['final_bbox'])
    anchors = set_anchors(CONVOUT_W, CONVOUT_H, ANCHORS_PER_GRID)
    infer.set_data({'anchors': anchors.ctypes.data_as(POINTER(c_float))})

def cleanup():
    global current_dir
    status = os.system('rm -f ' + current_dir + '/data/net.json ' \
                       + current_dir + '/data/out.json')
    if status != 0:
        print("rm failed")
        exit(1)

def run(data, img_height, img_width):
    global ready_data
    preprocess(data)
    in_dict = {'input': ready_data}
    param_dict = {'transform_bboxSQD0': {'img_width': c_double(img_width),
                                         'img_height': c_double(img_height)}}
    infer.set_data(in_dict)
    infer.set_param(param_dict)
    infer.run()
    infer.get_data(out_dict)
    return ln.lib.cast_buf(out_dict['final_bbox'], c_float)
