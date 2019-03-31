from ctypes import *
import lib

def create():
    lib.libln.ln_context_create.restype = c_void_p
    return lib.libln.ln_context_create()

def init(ctx, source):
    lib.libln.ln_context_init(ctx, source)

def cleanup(ctx):
    lib.libln.ln_context_cleanup(ctx)

def free(ctx):
    lib.libln.ln_context_free(ctx)

def compile(ctx, target):
    lib.libln.ln_context_compile(ctx, target)

def Print(ctx, outfile):
    lib.libln.ln_context_print(ctx, outfile)

def load(ctx, datafile):
    lib.libln.ln_context_load(ctx, datafile)

def set_data(ctx, tname, data):
    lib.libln.ln_context_set_data(ctx, tname, data)

def get_data(ctx, tname, data):
    lib.libln.ln_context_get_data.restype = c_void_p
    return lib.libln.ln_context_get_data(ctx, tname, data)

def data_size(ctx, tname):
    lib.libln.ln_context_data_size.restype = c_size_t
    return lib.libln.ln_context_data_size(ctx, tname)

def set_param(ctx, opname, pname, *args):
    if len(args) == 1:
        lib.libln.ln_context_set_param(ctx, opname, pname, args[0])
    elif len(args == 2):
        lib.libln.ln_context_set_param(ctx, opname, pname, args[0], args[1])
    else:
        assert False;

def run(ctx):
    lib.libln.ln_context_run(ctx)

def unload(ctx):
    lib.libln.ln_context_unload(ctx)
