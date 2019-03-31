from ctypes import *
import pylightnet as ln

class compiler(object):
    def __init__(self, source, target, outfile='out.json', options=[]):
        argv = ['', '-c', '-t', target, '-o', outfile, source] + options
        self.option = ln.option.create(ln.lib.str_array(argv))
        ln.msg.init(self.option)
        ln.arch.init()
        ln.name.init()
        self.ctx = ln.context.create()
        ln.context.init(self.ctx, ln.option.get_source(self.option))

    def __del__(self):
        ln.context.cleanup(self.ctx)
        ln.context.free(self.ctx)
        ln.name.cleanup()
        ln.arch.cleanup()
        ln.msg.cleanup()
        ln.option.free(self.option)

    def compile(self):
        ln.context.compile(self.ctx, ln.option.get_target(self.option))
        if not ln.util.streq(ln.option.get_outfile(self.option), b'!'):
            ln.context.Print(self.ctx, ln.option.get_outfile(self.option));
