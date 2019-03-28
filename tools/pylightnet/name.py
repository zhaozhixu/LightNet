from ctypes import *
import lib

def init():
    lib.libln.ln_name_init()

def cleanup():
    lib.libln.ln_name_cleanup()
