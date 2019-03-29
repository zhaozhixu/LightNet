import sys
from ctypes import *

libln = None

def is_py3():
    return sys.version_info > (3,)

def lib_init():
    global libln
    if not libln is None:
        return
    libln = cdll.LoadLibrary("liblightnet.so")
    # print("initialize liblightnet.so")

def lib_cleanup():
    libln = None

def str_array(str_list):
    array_type = c_char_p * len(str_list)
    array = array_type();
    for i in range(len(str_list)):
        if is_py3():
            array[i] = bytes(str_list[i], encoding = "utf8")
        else:
            array[i] = str_list[i]
    return array

def alloc(init, size=None):
    return create_string_buffer(init, size)

def version():
    buf = alloc(20)
    libln.ln_sprint_version(buf)
    if is_py3():
        return str(buf.value, encoding = "utf8")
    else:
        return str(buf.value)
