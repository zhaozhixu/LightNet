#! /usr/bin/python

import sys

from ctypes import *

libln = None

def lib_init():
    global libln
    if not libln is None:
        return
    libln = cdll.LoadLibrary("liblightnet.so")

def lib_cleanup():
    libln = None

def str_array(str_list):
    array_type = c_char_p * len(str_list)
    array = array_type();
    for i in range(len(str_list)):
        array[i] = str_list[i]
    return array

def ln_option_create(argv):
    argv_array = str_array(argv)
    libln.ln_option_create.restype = c_void_p
    return libln.ln_option_create(len(argv), argv_array)

def ln_option_free(option):
    libln.ln_option_free(option)

def ln_option_get_source(option):
    libln.ln_option_get_source.restype = c_char_p
    return libln.ln_option_get_source(option)

def ln_option_get_target(option):
    libln.ln_option_get_target.restype = c_char_p
    return libln.ln_option_get_target(option)

def ln_option_get_outfile(option):
    libln.ln_option_get_outfile.restype = c_char_p
    return libln.ln_option_get_outfile(option)

def ln_option_get_datafile(option):
    libln.ln_option_get_datafile.restype = c_char_p
    return libln.ln_option_get_datafile(option)

def ln_option_get_compile(option):
    libln.ln_option_get_datafile.restype = c_int
    return libln.ln_option_get_compile(option)

def ln_option_get_run(option):
    libln.ln_option_get_run.restype = c_int
    return libln.ln_option_get_run(option)

def ln_msg_init(option):
    libln.ln_msg_init(option)

def ln_msg_cleanup():
    libln.ln_msg_cleanup()

def ln_arch_init():
    libln.ln_arch_init()

def ln_arch_cleanup():
    libln.ln_arch_cleanup()

def ln_name_init():
    libln.ln_name_init()

def ln_name_cleanup():
    libln.ln_name_cleanup()

def ln_context_create():
    libln.ln_context_create.restype = c_void_p
    return libln.ln_context_create()

def ln_context_init(ctx, source):
    libln.ln_context_init(ctx, source)

def ln_context_cleanup(ctx):
    libln.ln_context_cleanup(ctx)

def ln_context_free(ctx):
    libln.ln_context_free(ctx)

def ln_context_compile(ctx, target):
    libln.ln_context_compile(ctx, target)

def ln_context_print(ctx, outfile):
    libln.ln_context_print(ctx, outfile)

def ln_context_load(ctx, datafile):
    libln.ln_context_load(ctx, datafile)

def ln_context_run(ctx):
    libln.ln_context_run(ctx)

def ln_context_unload(ctx):
    libln.ln_context_unload(ctx)

def ln_streq(str1, str2):
    libln.ln_streq.restype = c_int
    return libln.ln_streq(str1, str2)

def main(argv):
    lib_init()
    option = ln_option_create(argv)
    ln_msg_init(option)
    ln_arch_init()
    ln_name_init()
    ctx = ln_context_create()
    ln_context_init(ctx, ln_option_get_source(option))

    if (ln_option_get_compile(option)):
        ln_context_compile(ctx, ln_option_get_target(option))

    if not ln_streq(ln_option_get_outfile(option), b"!"):
        ln_context_print(ctx, ln_option_get_outfile(option))

    if ln_option_get_run(option):
        ln_context_load(ctx, ln_option_get_datafile(option))
        ln_context_run(ctx)
        ln_context_unload(ctx)

    ln_context_cleanup(ctx)
    ln_context_free(ctx)
    ln_name_cleanup()
    ln_arch_cleanup()
    ln_msg_cleanup()
    ln_option_free(option)

if __name__ == "__main__":
    main(sys.argv)
