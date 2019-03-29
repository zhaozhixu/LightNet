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

def set_data(ctx, name, data):
    lib.libln.ln_context_set_data(ctx, name, data)

def get_data(ctx, name):
    lib.libln.ln_context_get_data.restype = c_void_p
    return lib.libln.ln_context_get_data(ctx, name)

def data_size(ctx, name):
    lib.libln.ln_context_data_size.restype = c_size_t
    return lib.libln.ln_context_data_size(ctx, name)

def run(ctx):
    lib.libln.ln_context_run(ctx)

def unload(ctx):
    lib.libln.ln_context_unload(ctx)
