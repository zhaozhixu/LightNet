from ctypes import *
import lib

def create(argv):
    lib.libln.ln_option_create.restype = c_void_p
    return lib.libln.ln_option_create(len(argv), argv)

def free(option):
    lib.libln.ln_option_free(option)

def get_source(option):
    lib.libln.ln_option_get_source.restype = c_char_p
    return lib.libln.ln_option_get_source(option)

def get_target(option):
    lib.libln.ln_option_get_target.restype = c_char_p
    return lib.libln.ln_option_get_target(option)

def get_outfile(option):
    lib.libln.ln_option_get_outfile.restype = c_char_p
    return lib.libln.ln_option_get_outfile(option)

def get_datafile(option):
    lib.libln.ln_option_get_datafile.restype = c_char_p
    return lib.libln.ln_option_get_datafile(option)

def get_compile(option):
    lib.libln.ln_option_get_datafile.restype = c_int
    return lib.libln.ln_option_get_compile(option)

def get_run(option):
    lib.libln.ln_option_get_run.restype = c_int
    return lib.libln.ln_option_get_run(option)
