from ctypes import *
import lib

def init():
    lib.libln.ln_arch_init()

def cleanup():
    lib.libln.ln_arch_cleanup()
