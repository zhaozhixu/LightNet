from ctypes import *
import lib

def init(option):
    lib.libln.ln_msg_init(option)

def cleanup():
    lib.libln.ln_msg_cleanup()
