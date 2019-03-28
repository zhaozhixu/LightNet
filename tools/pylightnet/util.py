from ctypes import *
import lib

def sprint_version(buf):
    lib.libln.ln_sprint_version.restype = c_char_p
    return lib.libln.ln_sprint_version(buf)

def streq(str1, str2):
    lib.libln.ln_streq.restype = c_int
    return lib.libln.ln_streq(str1, str2)
