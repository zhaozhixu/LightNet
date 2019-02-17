#! /usr/bin/env python

import sys
import os
import re
import tempfile
import shutil
import traceback

def msg_exit(msg, num):
    sys.stderr.write(msg + '\n')
    exit(num)

def err_exit(msg):
    msg_exit(msg, 1)

def save_vars(ckpt_prefix, savedir):
    import numpy as np
    import tensorflow as tf
    ckpt_dir = os.path.split(os.path.realpath(ckpt_prefix))[0]
    np.set_printoptions(threshold=sys.maxsize)
    with tf.Session() as sess:
        saver = tf.train.import_meta_graph(ckpt_prefix + '.meta')
        saver.restore(sess, tf.train.latest_checkpoint(ckpt_dir))
        for x in tf.global_variables():
            filename = savedir + '/' + re.sub(r'/', '_', x.op.name) + '.txt'
            with open(filename, 'w') as f:
                f.write(str(x.eval(session=sess)))

def convert_vars(ckpt_prefix, savename):
    savedir = os.path.split(os.path.realpath(savename))[0]
    if not os.path.isdir(savedir):
        os.makedirs(savedir)
    tmpdir = tempfile.mkdtemp(prefix='genwts_tf_', dir=savedir)
    try:
        save_vars(ckpt_prefix, tmpdir)
    except Exception, e:
        shutil.rmtree(tmpdir)
        traceback.print_exc(e)
        err_exit('An exception has occured')
    current_dir = os.path.split(os.path.realpath(__file__))[0]
    genwts_script = current_dir + '/genwts.pl'
    os.system('chmod +x ' + genwts_script)
    cmd = genwts_script + ' -t 0 -o ' + savename + ' ' + tmpdir + '/*'
    os.system(cmd)
    shutil.rmtree(tmpdir)

usage = '''Usage: genwts_tf.py CKPT_PREFIX SAVENAME
Convert Tensorflow checkpoints to a weight file.
CKPT_PREFIX is the path prefix to the checkpoint files.
SAVENAME is the name of output weight file.

For example, if your checkpoint files are dir1/model.ckpt.meta,
dir1/model.ckpt.data and dir1/model.ckpt.index, execute:
    genwts_tf.py dir1/model.ckpt model.wts'''

if len(sys.argv) < 2 or len(sys.argv) > 3:
    err_exit(usage)
if sys.argv[1] == '-h' or sys.argv[1] == '--help':
    msg_exit(usage, 0)

ckpt_prefix = sys.argv[1]
savename = sys.argv[2]
convert_vars(ckpt_prefix, savename)
