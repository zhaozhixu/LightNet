import sys
from ctypes import *
import pylightnet as ln

class handler(object):
    def __init__(self, net, target="cpu", outfile="out.json", datafile=None,
                 compile_only=False, run_only=False, options=[]):
        if datafile is not None:
            datafile_opt = ['-f', datafile]
        else:
            datafile_opt = []
        if compile_only:
            compile_opt = ['-c']
        else:
            compile_opt = []
        if run_only:
            run_opt = ['-r']
        else:
            run_opt = []

        argv = [sys.argv[0], '-t', target, '-o', outfile, net] + \
            datafile_opt + compile_opt + run_opt + options
        self.option = ln.option.create(ln.lib.str_array(argv))
        self.ctx = ln.context.create()
        ln.context.init(self.ctx, ln.option.get_source(self.option))

    def __del__(self):
        ln.context.unload(self.ctx)
        ln.context.cleanup(self.ctx)
        ln.context.free(self.ctx)
        ln.option.free(self.option)

    def compile(self):
        if not ln.option.get_compile(self.option):
            sys.stderr.write("LightNet handler " + str(self) +
                             " is not initialized to be compilable\n")
            return
        ln.context.compile(self.ctx, ln.option.get_target(self.option),
                           ln.option.get_datafile(self.option))
        if not ln.util.streq(ln.option.get_outfile(self.option), b'!'):
            ln.context.Print(self.ctx, ln.option.get_outfile(self.option));

    def load(self):
        ln.context.load(self.ctx, ln.option.get_datafile(self.option))

    def run(self):
        ln.context.run(self.ctx)

    def create_data_dict(self, names):
        out_dict = {}
        for name in names:
            name_bytes = ln.lib.str2bytes(name)
            data_size = ln.context.data_size(self.ctx, name_bytes)
            buf = ln.lib.alloc(data_size)
            out_dict[name] = buf
        return out_dict

    def set_data(self, data_dict):
        for name, data in data_dict.items():
            name_bytes = ln.lib.str2bytes(name)
            ln.context.set_data(self.ctx, name_bytes, data)

    def get_data(self, data_dict):
        for name, data in data_dict.items():
            name_bytes = ln.lib.str2bytes(name)
            ln.context.get_data(self.ctx, name_bytes, data)

    def set_param(self, param_dict):
        for opname, param in param_dict.items():
            opname_bytes = ln.lib.str2bytes(opname)
            for pname, value in param.items():
                pname_bytes = ln.lib.str2bytes(pname)
                if isinstance(value, Array):
                    ln.context.set_param(self.ctx, opname_bytes, pname_bytes,
                                         value, len(value))
                else:
                    ln.context.set_param(self.ctx, opname_bytes, pname_bytes,
                                         value)

    def run(self):
        ln.context.run(self.ctx)
