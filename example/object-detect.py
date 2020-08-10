#! /usr/bin/env python3

import numpy as np
import time
import os
import sys
import cv2
import detect

def draw_bbox(img, bbox):
    # bbox: [xmin, ymin, xmax, ymax]
    cv2.rectangle(img,
                  (int(np.round(float(bbox[0]))),
                   int(np.round(float(bbox[1])))),
                  (int(np.round(float(bbox[2]))),
                   int(np.round(float(bbox[3])))), (0, 255, 0))
    cv2.imshow('detection', img)
    key = cv2.waitKey(5)
    if key == 32:           # space
        while cv2.waitKey(0) != 32:
            continue
    elif key == 113:        # q
        exit()

def exit_usage(exit_code):
    usage = '''Usage: object-detect.py [options] NET_FILE WEIGHT_FILE IMG_DIR
Do object detection of *.jpg images in directory IMG_DIR with network model
in NET_FILE and weight data in WEIGHT_FILE datain directory using Python API.

options:
    -h    print this message
    -r    run compiled NET_FILE
'''
    print(usage)
    exit(exit_code)

def main():
    need_compile = True
    if len(sys.argv) >= 2 and sys.argv[1] == '-h':
        exit_usage(0)
    if len(sys.argv) < 4:
        exit_usage(1)
    if sys.argv[1] == '-r':
        need_compile = False
        net = sys.argv[2]
        weight_file = sys.argv[3]
        img_dir = sys.argv[4]
    else:
        net = sys.argv[1]
        weight_file = sys.argv[2]
        img_dir = sys.argv[3]

    detect.init(net, weight_file, need_compile)
    files = sorted(os.listdir(img_dir))
    run_time = 0
    img_num = 0
    for f in files:
        if f.split('.')[-1] != 'jpg':
            continue
        file_path = img_dir+'/'+f
        img = cv2.imread(file_path, 1)
        time_start = time.time()
        bbox = detect.run(img, img.shape[0], img.shape[1])
        time_end = time.time()
        run_time = run_time + time_end - time_start
        img_num = img_num + 1
        print("[%.6f, %.6f, %.6f, %.6f]" % (bbox[0], bbox[1], bbox[2], bbox[3]))
        draw_bbox(img, bbox)

    detect.cleanup()
    print("total image number: %d" % img_num)
    print("frames per second of detection: %.6f" % (img_num/run_time))

if __name__ == "__main__":
    main()
