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

# the original algorithm is from SqueezeDet's official source code
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

handler = None
out_dict = None

def init(net, datafile, need_compile):
    global handler, out_dict
    # create a new LightNet handler
    handler = ln.handler(net, datafile=datafile, target="tensorrt")
    if need_compile:
        # compile model
        handler.compile()
    # init and load data
    handler.load()
    # create a dict to hold output data,
    # 'final_bbox' is a tensor defined in the net
    out_dict = handler.create_data_dict(['final_bbox'])
    # create and set the anchors for object detection,
    # 'anchors' is a tensor defined in the net
    anchors = set_anchors(CONVOUT_W, CONVOUT_H, ANCHORS_PER_GRID)
    handler.set_data({'anchors': anchors.ctypes.data_as(POINTER(c_float))})

def cleanup():
    return

def run(data, img_height, img_width):
    global handler, out_dict
    # set input data
    in_dict = {'input': data.ctypes.data_as(c_void_p)}
    handler.set_data(in_dict)
    # Set the original width and height of the image as parameters of
    # operator 'transform_bboxSQD0' in the net.
    # This is not necessary though, since the images' shape is fixed.
    # Just to show the way to set operator parameters while running
    param_dict = {'transform_bboxSQD0': {'img_width': c_double(img_width),
                                         'img_height': c_double(img_height)}}
    handler.set_param(param_dict)
    # run the net and copy output data to out_dict
    handler.run()
    handler.get_data(out_dict)
    res = ln.lib.cast_buf(out_dict['final_bbox'], c_float)

    return [res[0], res[1], res[2], res[3]]
