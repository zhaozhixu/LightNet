import sys
from ctypes import *
import pylightnet as ln

context = None
option = None
input_name = None
IS_PYTHON3 = sys.version_info > (3,)

class infer_handle(object):
    def __init__(self, source, weight, options=None):
        if not options is None:
            argv = ['', '-r', '-f', weight, source] + options
        else:
            argv = ['', '-r', '-f', weight, source]
        option = ln.option.create(ln.lib.str_array(argv))
        self.option = option
        ln.msg.init(option)
        ln.arch.init()
        ln.name.init()
        ctx = ln.context.create()
        self.ctx = ctx
        ln.context.init(ctx, ln.option.get_source(option))
        ln.context.load(ctx, ln.option.get_datafile(option))

    def __del__(self):
        ln.context.unload(self.ctx)
        ln.context.cleanup(self.ctx)
        ln.context.free(self.ctx)
        ln.name.cleanup()
        ln.arch.cleanup()
        ln.msg.cleanup()
        ln.option.free(self.option)

    def create_data_dict(self, names):
        out_dict = {}
        for name in names:
            if IS_PYTHON3:
                name = bytes(name, encoding='utf8')
            data_size = ln.context.data_size(self.ctx, name)
            buf = ln.lib.alloc(data_size)
            out_dict[name] = buf
        return out_dict

    def run(self, in_dict, out_dict):
        print "hi1"
        for name, data in in_dict.items():
            if IS_PYTHON3:
                name = bytes(name, encoding='utf8')
            ln.context.set_data(self.ctx, name, data)
            ln.context.run(self.ctx)
        print "hi2"
        for name in out_dict.keys():
            if IS_PYTHON3:
                name = bytes(name, encoding='utf8')
            data = ln.context.get_data(self.ctx, name)
            data_size = ln.context.data_size(self.ctx, name)
            print (data_size)
            print (data)
            data1 = ln.lib.cast_buf(data, c_float)
            print (data1[0])
            memmove(out_dict[name], data, data_size)
