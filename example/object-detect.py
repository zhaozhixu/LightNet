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
    usage = '''Usage: object-detect.py [options] DIR
Do object detection of *.jpg files in directory DIR using Python API.

options:
    -h    print this message
    -d    display bounding box in a "dection" window;
          press 'space' to pause, 'q' to exit
'''
    print(usage)
    exit(exit_code)

def main():
    if len(sys.argv) < 2 or len(sys.argv) > 3:
        exit_usage(1)
    if sys.argv[1] == '-h':
        exit_usage(0)
    do_display = sys.argv[1] == '-d'
    img_dir = sys.argv[-1]

    detect.init()
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
        if do_display:
            draw_bbox(img, bbox)

    detect.cleanup()
    print("total image number: "+str(img_num))
    print("frames per second: "+str(img_num/run_time))

if __name__ == "__main__":
    main()
