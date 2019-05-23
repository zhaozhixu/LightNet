from ctypes import *
import pylightnet as ln

class infer(object):
    def __init__(self, source, weight=None, options=[]):
        if weight is None:
            argv = ['', '-r', source] + options
        else:
            argv = ['', '-r', '-f', weight, source] + options
        self.option = ln.option.create(ln.lib.str_array(argv))
        ln.msg.init(self.option)
        ln.arch.init()
        ln.name.init()
        self.ctx = ln.context.create()
        ln.context.init(self.ctx, ln.option.get_source(self.option))
        ln.context.load(self.ctx, ln.option.get_datafile(self.option))

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
            name = ln.lib.str2bytes(name)
            data_size = ln.context.data_size(self.ctx, name)
            buf = ln.lib.alloc(data_size)
            out_dict[name] = buf
        return out_dict

    def set_data(self, data_dict):
        for name, data in data_dict.items():
            name = ln.lib.str2bytes(name)
            ln.context.set_data(self.ctx, name, data)

    def get_data(self, data_dict):
        for name, data in data_dict.items():
            name = ln.lib.str2bytes(name)
            ln.context.get_data(self.ctx, name, data)

    def set_param(self, param_dict):
        for opname, param in param_dict.items():
            opname = ln.lib.str2bytes(opname)
            for pname, value in param.items():
                pname = ln.lib.str2bytes(pname)
                if isinstance(value, Array):
                    ln.context.set_param(self.ctx, opname, pname,
                                         value, len(value))
                else:
                    ln.context.set_param(self.ctx, opname, pname, value)

    def run(self):
        ln.context.run(self.ctx)
